#pragma once
#include <cstdint>
#include <game-activity/GameActivityEvents.h>
#include "../Seekbar/AppPlaybackModel.h"
#include "../UI/SeekbarStyle.h"
#include "../UI/SeekbarLayout.h"

class SeekbarController {
public:
    explicit SeekbarController(SeekbarStyle style = {}) : style_(style) {}

    SeekbarStyle& style() { return style_; }
    const SeekbarStyle& style() const { return style_; }

    void update(AppPlaybackModel& model, int screenW, int screenH);

    void onDown(AppPlaybackModel& model, const SeekbarLayout& layout, int screenW,
                float x, float y, int32_t pointerId);

    void onMove(AppPlaybackModel& model, const SeekbarLayout& layout,
                const GameActivityMotionEvent& motionEvent);

    void onUp(AppPlaybackModel& model, const SeekbarLayout& layout, int screenW,
              float x, float y, int32_t pointerId);

private:
    SeekbarStyle style_;

    // drag
    int32_t dragPointerId_ = -1;

    // double-tap
    int64_t lastTapTimeMs_ = 0;
    float lastTapX_ = 0.f;
    float lastTapY_ = 0.f;

    static constexpr int64_t kDoubleTapWindowMs = 280;
    static constexpr float kDoubleTapSlopPx = 40.f;

    // buffering
    int64_t bufferingUntilMs_ = 0;

    int64_t xToSeekMs(const SeekbarLayout& layout, int64_t durationMs, float x) const;
    void startBufferingForMs(AppPlaybackModel& model, int64_t ms);
    void updateBufferingTimer(AppPlaybackModel& model);
};
