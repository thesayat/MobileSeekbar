#pragma once
#include "include/core/SkColor.h"

struct SeekbarStyle {
    float heightPx = 10.f;
    float hoverExtraPx = 4.f;
    float bottomOffsetPx = 70.f;
    float marginXPx = 52.f;
    float gapPx = 6.f;

    float thumbRadiusMul = 1.8f;
    SkColor thumbColor = SkColorSetARGB(255, 255, 0, 0);

    // hitbox
    float hitSlopY = 40.f;
    float hitSlopX = 10.f;
};
