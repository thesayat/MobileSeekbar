#include "SeekbarController.h"

#include <cmath>
#include <game-activity/native_app_glue/android_native_app_glue.h>

#include "../Common/Time.h"
#include "../Common/Math.h"

void SeekbarController::update(AppPlaybackModel& model, int /*screenW*/, int /*screenH*/) {
    updateBufferingTimer(model);
}

int64_t SeekbarController::xToSeekMs(const SeekbarLayout& layout, int64_t durationMs, float x) const {
    if (layout.barRect.isEmpty() || durationMs <= 0) return 0;
    float p = (x - layout.barRect.left()) / layout.barRect.width();
    p = Common::Math::clamp01(p);
    const double ms = (double)p * (double)durationMs;
    return (int64_t)(ms + 0.5);
}

void SeekbarController::startBufferingForMs(AppPlaybackModel& model, int64_t ms) {
    const int64_t t = Common::Time::nowMs();
    bufferingUntilMs_ = t + ms;
    model.player.isBuffering = true;
}

void SeekbarController::updateBufferingTimer(AppPlaybackModel& model) {
    if (!model.player.isBuffering) return;
    const int64_t t = Common::Time::nowMs();
    if (bufferingUntilMs_ != 0 && t >= bufferingUntilMs_) {
        model.player.isBuffering = false;
        bufferingUntilMs_ = 0;
    }
}

void SeekbarController::onDown(AppPlaybackModel& model, const SeekbarLayout& layout, int /*screenW*/,
                               float x, float y, int32_t pointerId) {
    if (!model.seek.isDragging && layout.hitboxRect.contains(x, y) && model.player.durationMs > 0) {
        dragPointerId_ = pointerId;
        const int64_t ms = xToSeekMs(layout, model.player.durationMs, x);
        model.seek.startDrag(ms, model.player.durationMs);
    }
}

void SeekbarController::onMove(AppPlaybackModel& model, const SeekbarLayout& layout,
                               const GameActivityMotionEvent& motionEvent) {
    if (!model.seek.isDragging || dragPointerId_ == -1 || model.player.durationMs <= 0) return;

    for (int i = 0; i < motionEvent.pointerCount; ++i) {
        auto &p = motionEvent.pointers[i];
        if (p.id == dragPointerId_) {
            const float mx = GameActivityPointerAxes_getX(&p);
            const int64_t ms = xToSeekMs(layout, model.player.durationMs, mx);
            model.seek.updateDrag(ms, model.player.durationMs);
            break;
        }
    }
}

void SeekbarController::onUp(AppPlaybackModel& model, const SeekbarLayout& layout, int screenW,
                             float x, float y, int32_t pointerId) {
    bool endedDrag = false;

    // Commit drag
    if (model.seek.isDragging && dragPointerId_ == pointerId) {
        const int64_t commitMs = model.seek.dragPositionMs;
        model.player.setPosition(commitMs);
        model.seek.endDrag();
        dragPointerId_ = -1;
        endedDrag = true;

        startBufferingForMs(model, 2000);
    }

    // Double-tap
    if (!endedDrag && !model.seek.isDragging) {
        const int64_t t = Common::Time::nowMs();

        const float slop2 = kDoubleTapSlopPx * kDoubleTapSlopPx;
        const bool withinTime = (t - lastTapTimeMs_) > 0 && (t - lastTapTimeMs_) <= kDoubleTapWindowMs;
        const bool withinDist = Common::Math::dist2(x, y, lastTapX_, lastTapY_) <= slop2;

        if (withinTime && withinDist) {
            const bool leftHalf = (x < (float)screenW * 0.5f);
            const int64_t delta = leftHalf ? -10000 : 10000;
            model.player.seekBy(delta);

            startBufferingForMs(model, 2000);

            // reset
            lastTapTimeMs_ = 0;
        } else {
            lastTapTimeMs_ = t;
            lastTapX_ = x;
            lastTapY_ = y;
        }
    }
}
