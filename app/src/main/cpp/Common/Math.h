#pragma once
#include <algorithm>

namespace Common
{

namespace Math
{
    inline float clamp01(float v) {
        if (v < 0.f) return 0.f;
        if (v > 1.f) return 1.f;
        return v;
    }

    inline float clampf(float v, float lo, float hi) {
        return (v < lo) ? lo : (v > hi) ? hi : v;
    }

    inline float dist2(float ax, float ay, float bx, float by) {
        const float dx = ax - bx;
        const float dy = ay - by;
        return dx*dx + dy*dy;
    }
} // Math

} // Common


