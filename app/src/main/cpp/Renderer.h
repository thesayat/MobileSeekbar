#ifndef ANDROIDGLINVESTIGATIONS_RENDERER_H
#define ANDROIDGLINVESTIGATIONS_RENDERER_H

#include <EGL/egl.h>
#include <memory>

#include "Seekbar/AppPlaybackModel.h"
#include "UI/SeekbarStyle.h"
#include "UI/SeekbarView.h"
#include "Input/SeekbarController.h"
#include "Render/SkiaRasterTexture.h"
#include "Render/FullscreenTexturedQuad.h"
#include "Animation/CircleProgress.h"
#include "Shader/Shader.h"

struct android_app;

class Renderer {
public:

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    Renderer(android_app *pApp);

    virtual ~Renderer();

    /*!
     * Handles input from the android_app.
     *
     * Note: this will clear the input queue
     */
    void handleInput();

    /*!
     * Renders all the models in the renderer
     */
    void render();

private:
    /*!
     * Performs necessary OpenGL initialization. Customize this if you want to change your EGL
     * context or application-wide settings.
     */
    void initRenderer();

    /*!
     * @brief we have to check every frame to see if the framebuffer has changed in size. If it has,
     * update the viewport accordingly
     */
    void updateRenderArea();

    android_app *app_;
    EGLDisplay display_;
    EGLSurface surface_;
    EGLContext context_;
    EGLint width_;
    EGLint height_;

    bool shaderNeedsNewProjectionMatrix_;

    std::unique_ptr<Shader> shader_;

    AppPlaybackModel playbackModel;

    SeekbarStyle seekbarStyle_;
    SeekbarController seekbarController_;
    SeekbarView seekbarView_;
    SkiaRasterTexture skiaRasterTex_;
    FullscreenTexturedQuad fullscreenQuad_;
    CircleProgress circleProgress_;
};

#endif //ANDROIDGLINVESTIGATIONS_RENDERER_H