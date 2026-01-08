#include "Renderer.h"

#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <GLES3/gl3.h>
#include <memory>
#include <vector>
#include <android/imagedecoder.h>
#include "Common/Time.h"
#include "Common/Math.h"
#include <cmath>

#include "AndroidDebug/AndroidOut.h"
#include "Common/Utility.h"
#include "Texture/TextureAsset.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "UI/SeekbarLayout.h"
#include "Render/ProjectionConfig.h"

#include "Shader/vertex.inl"
#include "Shader/fragment.inl"

Renderer::Renderer(android_app *pApp) :
app_(pApp),
display_(EGL_NO_DISPLAY),
surface_(EGL_NO_SURFACE),
context_(EGL_NO_CONTEXT),
width_(0),
height_(0),
shaderNeedsNewProjectionMatrix_(true)
{
    initRenderer();
}

Renderer::~Renderer() {
    if (display_ != EGL_NO_DISPLAY) {
        eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (context_ != EGL_NO_CONTEXT) {
            eglDestroyContext(display_, context_);
            context_ = EGL_NO_CONTEXT;
        }
        if (surface_ != EGL_NO_SURFACE) {
            eglDestroySurface(display_, surface_);
            surface_ = EGL_NO_SURFACE;
        }
        eglTerminate(display_);
        display_ = EGL_NO_DISPLAY;
    }
}

void Renderer::initRenderer() {

    seekbarController_.style() = seekbarStyle_;
    seekbarView_.style() = seekbarController_.style();

    // Choose your render attributes
    constexpr EGLint attribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_DEPTH_SIZE, 24,
            EGL_NONE
    };

    // The default display is probably what you want on Android
    auto display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, nullptr, nullptr);

    // figure out how many configs there are
    EGLint numConfigs;
    eglChooseConfig(display, attribs, nullptr, 0, &numConfigs);

    // get the list of configurations
    std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
    eglChooseConfig(display, attribs, supportedConfigs.get(), numConfigs, &numConfigs);

    // Find a config we like.
    // Could likely just grab the first if we don't care about anything else in the config.
    // Otherwise hook in your own heuristic
    auto config = *std::find_if(
            supportedConfigs.get(),
            supportedConfigs.get() + numConfigs,
            [&display](const EGLConfig &config) {
                EGLint red, green, blue, depth;
                if (eglGetConfigAttrib(display, config, EGL_RED_SIZE, &red)
                    && eglGetConfigAttrib(display, config, EGL_GREEN_SIZE, &green)
                    && eglGetConfigAttrib(display, config, EGL_BLUE_SIZE, &blue)
                    && eglGetConfigAttrib(display, config, EGL_DEPTH_SIZE, &depth)) {

                    aout << "Found config with " << red << ", " << green << ", " << blue << ", "
                         << depth << std::endl;
                    return red == 8 && green == 8 && blue == 8 && depth == 24;
                }
                return false;
            });

    aout << "Found " << numConfigs << " configs" << std::endl;
    aout << "Chose " << config << std::endl;

    // create the proper window surface
    EGLint format;
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    EGLSurface surface = eglCreateWindowSurface(display, config, app_->window, nullptr);

    // Create a GLES 3 context
    EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    eglBindAPI(EGL_OPENGL_ES_API);

    EGLContext context = eglCreateContext(display, config, nullptr, contextAttribs);

    // get some window metrics
    auto madeCurrent = eglMakeCurrent(display, surface, surface, context);
    assert(madeCurrent);

    display_ = display;
    surface_ = surface;
    context_ = context;

    // make width and height invalid so it gets updated the first frame in @a updateRenderArea()
    width_ = -1;
    height_ = -1;

    PRINT_GL_STRING(GL_VENDOR);
    PRINT_GL_STRING(GL_RENDERER);
    PRINT_GL_STRING(GL_VERSION);
    PRINT_GL_STRING_AS_LIST(GL_EXTENSIONS);

    shader_ = std::unique_ptr<Shader>(
            Shader::loadShader(vertex, fragment, "inPosition", "inUV", "uProjection"));
    assert(shader_);

    shader_->activate();

    // setup any other gl related global states
    glClearColor(CORNFLOWER_BLUE);

    // enable alpha globally for now, you probably don't want to do this in a game
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //init player
    playbackModel = {
            .player = {
                    .durationMs = 300000,
                    .positionMs = 20000,
                    .bufferedMs = 60000,
                    .isBuffering = false
            },
            .seek = {},
            .chapters = {
                    {0, 45000},
                    {45000, 140000},
                    {140000, 300000}
            }
    };

    normalizeChapters(playbackModel.chapters, playbackModel.player.durationMs);
}

void Renderer::render() {
    updateRenderArea();

    seekbarController_.update(playbackModel, width_, height_);

    circleProgress_.tick(playbackModel.player.isBuffering);

    if (shaderNeedsNewProjectionMatrix_) {
        float projectionMatrix[16] = {0};
        Utility::buildOrthographicMatrix(
                projectionMatrix,
                ProjectionConfig::HalfHeight,
                float(width_) / height_,
                ProjectionConfig::NearPlane,
                ProjectionConfig::FarPlane);
        shader_->setProjectionMatrix(projectionMatrix);
        shaderNeedsNewProjectionMatrix_ = false;
    }

    skiaRasterTex_.ensure(width_, height_);
    fullscreenQuad_.ensure(width_, height_, skiaRasterTex_.textureAsset());

    if (auto* c = skiaRasterTex_.canvas()) {
        c->clear(SK_ColorDKGRAY);
        seekbarView_.draw(c, playbackModel, width_, height_, circleProgress_.phase());
    }

    skiaRasterTex_.upload();

    glClear(GL_COLOR_BUFFER_BIT);

    if (fullscreenQuad_.ready()) {
        fullscreenQuad_.draw(*shader_);
    }

    EGLBoolean ok = eglSwapBuffers(display_, surface_);
    assert(ok == EGL_TRUE);
}

void Renderer::updateRenderArea() {
    if (display_ == EGL_NO_DISPLAY || surface_ == EGL_NO_SURFACE) return;

    EGLint width = 0;
    EGLint height = 0;

    if (!eglQuerySurface(display_, surface_, EGL_WIDTH, &width) ||
        !eglQuerySurface(display_, surface_, EGL_HEIGHT, &height)) {
        aout << "updateRenderArea: eglQuerySurface FAILED err=0x"
             << std::hex << eglGetError() << std::dec << "\n";
        return;
    }

    if (width <= 0 || height <= 0) {
        aout << "updateRenderArea: invalid size " << width << "x" << height << "\n";
        return;
    }

    if (width != width_ || height != height_) {
        width_ = width;
        height_ = height;

        glViewport(0, 0, width_, height_);
        shaderNeedsNewProjectionMatrix_ = true;

        aout << "updateRenderArea: " << width_ << "x" << height_ << "\n";
    }
}

void Renderer::handleInput() {
    // handle all queued inputs
    auto *inputBuffer = android_app_swap_input_buffers(app_);
    if (!inputBuffer) {
        // no inputs yet.
        return;
    }

    const SeekbarLayout layout = makeSeekbarLayout(width_, height_, seekbarStyle_);

    // handle motion events (motionEventsCounts can be 0).
    for (auto i = 0; i < inputBuffer->motionEventsCount; i++) {
        auto &motionEvent = inputBuffer->motionEvents[i];
        auto action = motionEvent.action;

        // Find the pointer index, mask and bitshift to turn it into a readable value.
        auto pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
                >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
        aout << "Pointer(s): ";

        // get the x and y position of this event if it is not ACTION_MOVE.
        auto &pointer = motionEvent.pointers[pointerIndex];
        auto x = GameActivityPointerAxes_getX(&pointer);
        auto y = GameActivityPointerAxes_getY(&pointer);

        bool endedDrag = false;


        // determine the action type and process the event accordingly.
        switch (action & AMOTION_EVENT_ACTION_MASK) {
            case AMOTION_EVENT_ACTION_DOWN:
            case AMOTION_EVENT_ACTION_POINTER_DOWN:
                aout << "(" << pointer.id << ", " << x << ", " << y << ") "
                     << "Pointer Down";

                seekbarController_.onDown(playbackModel, layout, width_, x, y, pointer.id);
                break;

            case AMOTION_EVENT_ACTION_CANCEL:
                // treat the CANCEL as an UP event: doing nothing in the app, except
                // removing the pointer from the cache if pointers are locally saved.
                // code pass through on purpose.
            case AMOTION_EVENT_ACTION_UP:
            case AMOTION_EVENT_ACTION_POINTER_UP:
                aout << "(" << pointer.id << ", " << x << ", " << y << ") "
                     << "Pointer Up";
                seekbarController_.onUp(playbackModel, layout, width_, x, y, pointer.id);
                break;

            case AMOTION_EVENT_ACTION_MOVE:
                seekbarController_.onMove(playbackModel, layout, motionEvent);

                aout << "Pointer Move";
                break;
            default:
                aout << "Unknown MotionEvent Action: " << action;
        }
        aout << std::endl;
    }
    // clear the motion input count in this buffer for main thread to re-use.
    android_app_clear_motion_events(inputBuffer);

    // handle input key events.
    for (auto i = 0; i < inputBuffer->keyEventsCount; i++) {
        auto &keyEvent = inputBuffer->keyEvents[i];
        aout << "Key: " << keyEvent.keyCode <<" ";
        switch (keyEvent.action) {
            case AKEY_EVENT_ACTION_DOWN:
                aout << "Key Down";
                break;
            case AKEY_EVENT_ACTION_UP:
                aout << "Key Up";
                break;
            case AKEY_EVENT_ACTION_MULTIPLE:
                // Deprecated since Android API level 29.
                aout << "Multiple Key Actions";
                break;
            default:
                aout << "Unknown KeyEvent Action: " << keyEvent.action;
        }
        aout << std::endl;
    }
    // clear the key input count too.
    android_app_clear_key_events(inputBuffer);
}