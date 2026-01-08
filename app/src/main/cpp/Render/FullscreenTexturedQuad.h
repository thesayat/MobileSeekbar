#pragma once
#include "../Model/Model.h"
#include "../Texture/TextureAsset.h"
#include <memory>

class Shader;

class FullscreenTexturedQuad {
public:
    void ensure(int screenW, int screenH, std::shared_ptr<TextureAsset> texAsset);
    void draw(Shader& shader);

    bool ready() const { return ready_; }

private:
    bool ready_ = false;
    int w_ = 0;
    int h_ = 0;
    Model quad_{{}, {}, nullptr};
};
