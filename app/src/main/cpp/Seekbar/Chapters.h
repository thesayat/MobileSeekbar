#pragma once
#include <cstdint>
#include <vector>
#include <algorithm>

struct Chapter {
    int64_t startMs = 0;
    int64_t endMs = 0;
};

inline void normalizeChapters(std::vector<Chapter>& ch, int64_t durationMs) {
    for (auto& c : ch) {
        c.startMs = std::clamp(c.startMs, int64_t{0}, durationMs);
        c.endMs   = std::clamp(c.endMs,   int64_t{0}, durationMs);
        if (c.endMs < c.startMs) std::swap(c.startMs, c.endMs);
    }
    std::sort(ch.begin(), ch.end(), [](const Chapter& a, const Chapter& b) {
        return a.startMs < b.startMs;
    });

    ch.erase(std::remove_if(ch.begin(), ch.end(), [](const Chapter& c) {
        return c.endMs <= c.startMs;
    }), ch.end());
}
