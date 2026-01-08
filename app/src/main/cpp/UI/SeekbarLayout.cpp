#include "SeekbarLayout.h"

SeekbarLayout makeSeekbarLayout(int screenW, int screenH, const SeekbarStyle& style) {
    SeekbarLayout out{};

    if (screenW <= 0 || screenH <= 0) {
        out.barRect = SkRect::MakeEmpty();
        out.hitboxRect = SkRect::MakeEmpty();
        return out;
    }

    const float left  = style.marginXPx;
    const float right = (float)screenW - style.marginXPx;
    const float y     = (float)screenH - style.bottomOffsetPx;

    if (right <= left) {
        out.barRect = SkRect::MakeEmpty();
        out.hitboxRect = SkRect::MakeEmpty();
        return out;
    }

    out.barRect = SkRect::MakeXYWH(left, y, right - left, style.heightPx);

    out.hitboxRect = out.barRect;
    out.hitboxRect.inset(-style.hitSlopX, -style.hitSlopY);

    return out;
}
