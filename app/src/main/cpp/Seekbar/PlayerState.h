#pragma once
#include <cstdint>
#include <algorithm>

struct PlayerState {
    int64_t durationMs = 0;
    int64_t positionMs = 0;
    int64_t bufferedMs = 0;
    bool isBuffering = false;

    void setDuration(int64_t ms) {
        durationMs = std::max<int64_t>(0, ms);
        positionMs = std::clamp(positionMs, int64_t{0}, durationMs);
        bufferedMs = std::clamp(bufferedMs, int64_t{0}, durationMs);
    }

    void setPosition(int64_t ms) {
        positionMs = std::clamp(ms, int64_t{0}, durationMs);
    }

    void setBuffered(int64_t ms) {
        bufferedMs = std::clamp(ms, int64_t{0}, durationMs);
    }

    void seekBy(int64_t deltaMs) {
        setPosition(positionMs + deltaMs);
    }

    float played01() const {
        if (durationMs <= 0) return 0.f;
        return static_cast<float>(positionMs) / static_cast<float>(durationMs);
    }

    float buffered01() const {
        if (durationMs <= 0) return 0.f;
        return static_cast<float>(bufferedMs) / static_cast<float>(durationMs);
    }
};