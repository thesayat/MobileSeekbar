#include "SeekbarView.h"

#include <cmath>
#include <algorithm>

#include "../Common/Math.h"
#include "include/core/SkRRect.h"

static void drawSegmentRounded(SkCanvas* c, const SkRect& r, float radius, const SkPaint& paint) {
    if (!c || r.isEmpty()) return;
    const float rr = std::min(radius, r.width() * 0.5f);
    if (r.width() < 2.f) {
        c->drawRect(r, paint);
    } else {
        c->drawRRect(SkRRect::MakeRectXY(r, rr, rr), paint);
    }
}

static int findChapterIndexAtMs(const std::vector<Chapter>& ch, int64_t ms) {
    for (int i = 0; i < (int)ch.size(); ++i) {
        if (ms >= ch[i].startMs && ms < ch[i].endMs) return i;
    }
    return -1;
}

void SeekbarView::drawLoadingSpinner(SkCanvas* c, float cx, float cy, float sizePx, float phase) const {
    if (!c) return;

    constexpr int kDots = 8;
    const float twoPi = 6.28318530718f;

    const float ringR = sizePx * 1.35f;
    const float dotR  = sizePx * 0.40f;

    const float head = phase * (float)kDots;

    SkPaint p;
    p.setAntiAlias(true);

    const float angleOffset = -twoPi * 0.25f;

    for (int i = 0; i < kDots; ++i) {
        const float a = angleOffset + twoPi * ((float)i / (float)kDots);

        const float x = cx + std::cos(a) * ringR;
        const float y = cy + std::sin(a) * ringR;

        float delta = (float)i - head;
        while (delta < 0.f) delta += (float)kDots;
        while (delta >= (float)kDots) delta -= (float)kDots;

        float intensity = 1.0f - delta * 0.12f;
        intensity = Common::Math::clampf(intensity, 0.15f, 1.0f);

        const uint8_t a8 = (uint8_t)(intensity * 255.0f);
        p.setColor(SkColorSetARGB(a8, 255, 255, 255));
        c->drawCircle(x, y, dotR, p);
    }
}

void SeekbarView::draw(SkCanvas* c,
                       const AppPlaybackModel& model,
                       int screenW,
                       int screenH,
                       float loadingPhase) const {
    if (!c || screenW <= 0 || screenH <= 0) return;

    const SeekbarLayout layout = makeSeekbarLayout(screenW, screenH, style_);
    const SkRect baseBar = layout.barRect;
    if (baseBar.isEmpty()) return;

    // effective position (drag preview)
    int64_t effectivePosMs = model.player.positionMs;
    if (model.seek.isDragging) effectivePosMs = model.seek.dragPositionMs;

    const int hoveredChapter = (model.seek.isDragging && !model.chapters.empty())
                               ? findChapterIndexAtMs(model.chapters, effectivePosMs)
                               : -1;

    float played01 = 0.f;
    if (model.player.durationMs > 0) {
        played01 = (float)((double)effectivePosMs / (double)model.player.durationMs);
    }
    played01 = Common::Math::clamp01(played01);

    const float playedEndX = baseBar.left() + baseBar.width() * played01;

    // Paint
    SkPaint trackPaint;
    trackPaint.setAntiAlias(true);
    trackPaint.setColor(SkColorSetARGB(160, 30, 30, 30));

    SkPaint playedPaint;
    playedPaint.setAntiAlias(true);
    playedPaint.setColor(SkColorSetARGB(255, 255, 255, 255));

    SkPaint thumbPaint;
    thumbPaint.setAntiAlias(true);
    thumbPaint.setColor(style_.thumbColor);

    const float centerY = baseBar.centerY();
    const float baseH = baseBar.height();
    const float hoverH = baseH + style_.hoverExtraPx;

    auto drawAllSegments = [&](auto&& fnDrawPerSegment) {
        if (model.chapters.empty() || model.player.durationMs <= 0) {
            SkRect seg = SkRect::MakeLTRB(baseBar.left(), centerY - baseH*0.5f,
                                          baseBar.right(), centerY + baseH*0.5f);
            fnDrawPerSegment(seg, 0);
            return;
        }

        for (int idx = 0; idx < (int)model.chapters.size(); ++idx) {
            const auto& ch = model.chapters[idx];
            const double s01 = (double)ch.startMs / (double)model.player.durationMs;
            const double e01 = (double)ch.endMs   / (double)model.player.durationMs;

            float segL = baseBar.left() + (float)(s01 * (double)baseBar.width());
            float segR = baseBar.left() + (float)(e01 * (double)baseBar.width());

            segL += style_.gapPx * 0.5f;
            segR -= style_.gapPx * 0.5f;

            segL = std::max(segL, baseBar.left());
            segR = std::min(segR, baseBar.right());
            if (segR <= segL) continue;

            const float segH = (idx == hoveredChapter) ? hoverH : baseH;

            SkRect segRect = SkRect::MakeLTRB(segL, centerY - segH*0.5f,
                                              segR, centerY + segH*0.5f);
            fnDrawPerSegment(segRect, idx);
        }
    };

    // track
    drawAllSegments([&](const SkRect& segRect, int /*idx*/) {
        drawSegmentRounded(c, segRect, segRect.height() * 0.5f, trackPaint);
    });

    // played
    drawAllSegments([&](const SkRect& segRect, int /*idx*/) {
        const float R = std::min(segRect.right(), playedEndX);
        if (R <= segRect.left()) return;
        SkRect part = SkRect::MakeLTRB(segRect.left(), segRect.top(), R, segRect.bottom());
        drawSegmentRounded(c, part, segRect.height() * 0.5f, playedPaint);
    });

    // spinner (buffering)
    if (model.player.isBuffering) {
        drawLoadingSpinner(c, screenW * 0.5f, screenH * 0.5f, 24.f, loadingPhase);
    }

    // thumb
    float thumbX = std::max(baseBar.left(), std::min(playedEndX, baseBar.right()));
    const float thumbR = baseBar.height() * style_.thumbRadiusMul;
    c->drawCircle(thumbX, centerY, thumbR, thumbPaint);
}
