#pragma once
#include <cstdint>
#include <algorithm>

struct SeekbarState {
    bool isDragging = false;
    int64_t dragPositionMs = 0;

    void startDrag(int64_t startMs, int64_t durationMs) {
        isDragging = true;
        dragPositionMs = std::clamp(startMs, int64_t{0}, durationMs);
    }

    void updateDrag(int64_t ms, int64_t durationMs) {
        if (!isDragging) return;
        dragPositionMs = std::clamp(ms, int64_t{0}, durationMs);
    }

    void endDrag() { isDragging = false; }
};
