#pragma once
#include <memory>
#include <GLES3/gl3.h>

#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImageInfo.h"

#include "../Texture/TextureAsset.h"

class SkiaRasterTexture {
public:
    SkiaRasterTexture() = default;
    ~SkiaRasterTexture();

    SkiaRasterTexture(const SkiaRasterTexture&) = delete;
    SkiaRasterTexture& operator=(const SkiaRasterTexture&) = delete;

    void ensure(int w, int h);

    SkCanvas* canvas();
    void upload();

    int width() const { return w_; }
    int height() const { return h_; }

    GLuint textureId() const { return tex_; }
    std::shared_ptr<TextureAsset> textureAsset() const { return texAsset_; }

private:
    int w_ = 0;
    int h_ = 0;

    sk_sp<SkSurface> surface_;

    GLuint tex_ = 0;
    std::shared_ptr<TextureAsset> texAsset_;

    void createTextureIfNeeded();
    void allocateTextureStorage();
};
