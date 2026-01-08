#include "SkiaRasterTexture.h"

#include "../AndroidDebug/AndroidOut.h"

SkiaRasterTexture::~SkiaRasterTexture() {
    if (tex_ != 0) {
        glDeleteTextures(1, &tex_);
        tex_ = 0;
    }
}

void SkiaRasterTexture::createTextureIfNeeded() {
    if (tex_ != 0) return;

    glGenTextures(1, &tex_);
    glBindTexture(GL_TEXTURE_2D, tex_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    texAsset_ = TextureAsset::wrapExisting(tex_, true);
}

void SkiaRasterTexture::allocateTextureStorage() {
    if (tex_ == 0 || w_ <= 0 || h_ <= 0) return;

    glBindTexture(GL_TEXTURE_2D, tex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w_, h_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
}

void SkiaRasterTexture::ensure(int w, int h) {
    if (w <= 0 || h <= 0) return;

    const bool sizeChanged = (w != w_ || h != h_);
    if (!surface_ || sizeChanged) {
        w_ = w;
        h_ = h;

        SkImageInfo info = SkImageInfo::Make(w_, h_, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        surface_ = SkSurfaces::Raster(info);

        if (!surface_) {
            aout << "SkiaRasterTexture: SkSurfaces::Raster FAILED\n";
            return;
        }

        createTextureIfNeeded();
        allocateTextureStorage();

        aout << "SkiaRasterTexture: rebuilt " << w_ << "x" << h_ << "\n";
    } else {
        createTextureIfNeeded();
    }
}

SkCanvas* SkiaRasterTexture::canvas() {
    return surface_ ? surface_->getCanvas() : nullptr;
}

void SkiaRasterTexture::upload() {
    if (!surface_ || tex_ == 0 || w_ <= 0 || h_ <= 0) return;

    SkPixmap pm;
    if (!surface_->peekPixels(&pm)) {
        aout << "SkiaRasterTexture: peekPixels FAILED\n";
        return;
    }

    const void* pixels = pm.addr();
    if (!pixels) {
        aout << "SkiaRasterTexture: pixels NULL\n";
        return;
    }

    glBindTexture(GL_TEXTURE_2D, tex_);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    const int rowBytes = (int)pm.rowBytes();
    const int rowLengthPixels = rowBytes / 4;

    if (rowBytes != w_ * 4) {
        glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLengthPixels);
    } else {
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    }

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w_, h_, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}
