#pragma once
#include "include/core/SkRect.h"
#include "SeekbarStyle.h"

struct SeekbarLayout {
    SkRect barRect;
    SkRect hitboxRect;
};

SeekbarLayout makeSeekbarLayout(int screenW, int screenH, const SeekbarStyle& style);
