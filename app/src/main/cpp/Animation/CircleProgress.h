#pragma once
#include <cstdint>

class CircleProgress {
public:
    float phase() const { return phase_; }

    void reset() { phase_ = 0.f; lastFrameMs_ = 0; }

    void setSpeed(float cyclesPerSecond) { speed_ = cyclesPerSecond; }

    void tick(bool enabled);

private:
    float phase_ = 0.f;
    float speed_ = 0.85f;
    int64_t lastFrameMs_ = 0;
};
