#pragma once
#include "include/core/SkCanvas.h"
#include "../Seekbar/AppPlaybackModel.h"
#include "SeekbarStyle.h"
#include "SeekbarLayout.h"

class SeekbarView {
public:
    explicit SeekbarView(SeekbarStyle style = {}) : style_(style) {}

    SeekbarStyle& style() { return style_; }
    const SeekbarStyle& style() const { return style_; }

    void draw(SkCanvas* c,
              const AppPlaybackModel& model,
              int screenW,
              int screenH,
              float loadingPhase) const;

private:
    SeekbarStyle style_;

    void drawLoadingSpinner(SkCanvas* c, float cx, float cy, float sizePx, float phase) const;
};
