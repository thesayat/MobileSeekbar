#include "FullscreenTexturedQuad.h"
#include "../Shader/Shader.h"
#include "ProjectionConfig.h"

#include <vector>

void FullscreenTexturedQuad::ensure(int screenW, int screenH, std::shared_ptr<TextureAsset> texAsset) {
    if (screenW <= 0 || screenH <= 0) return;
    if (!texAsset) return;

    const bool sizeChanged = (screenW != w_ || screenH != h_);
    if (!ready_ || sizeChanged) {
        w_ = screenW;
        h_ = screenH;

        const float halfH = ProjectionConfig::HalfHeight;
        const float halfW = halfH * (float(w_) / float(h_));

        std::vector<Vertex> v = {
                { Vector3{-halfW, -halfH, 0.0f}, Vector2{0.0f, 1.0f} },
                { Vector3{ halfW, -halfH, 0.0f}, Vector2{1.0f, 1.0f} },
                { Vector3{ halfW,  halfH, 0.0f}, Vector2{1.0f, 0.0f} },
                { Vector3{-halfW,  halfH, 0.0f}, Vector2{0.0f, 0.0f} },
        };

        std::vector<Index> idx = { 0, 1, 2, 2, 3, 0 };

        quad_ = Model(std::move(v), std::move(idx), texAsset);
        ready_ = true;
    }
}

void FullscreenTexturedQuad::draw(Shader& shader) {
    if (!ready_) return;
    shader.drawModel(quad_);
}
