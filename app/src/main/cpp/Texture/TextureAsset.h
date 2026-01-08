#ifndef ANDROIDGLINVESTIGATIONS_TEXTUREASSET_H
#define ANDROIDGLINVESTIGATIONS_TEXTUREASSET_H

#include <memory>
#include <android/asset_manager.h>
#include <GLES3/gl3.h>
#include <string>
#include <vector>

class TextureAsset {
public:
    static std::shared_ptr<TextureAsset>
    loadAsset(AAssetManager *assetManager, const std::string &assetPath);

    static std::shared_ptr<TextureAsset>
    wrapExisting(GLuint textureId, bool takeOwnership);

    ~TextureAsset();

    constexpr GLuint getTextureID() const { return textureID_; }

private:
    inline TextureAsset(GLuint textureId, bool takeOwnership)
            : textureID_(textureId), takeOwnership_(takeOwnership) {}

    GLuint textureID_ = 0;
    bool takeOwnership_ = true;
};

#endif //ANDROIDGLINVESTIGATIONS_TEXTUREASSET_H
