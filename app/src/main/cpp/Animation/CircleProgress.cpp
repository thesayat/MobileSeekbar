#include "CircleProgress.h"
#include <cmath>
#include "../Common/Time.h"

void CircleProgress::tick(bool enabled) {
    const int64_t t = Common::Time::nowMs();
    if (lastFrameMs_ == 0) lastFrameMs_ = t;
    const float dt = (float)(t - lastFrameMs_) / 1000.0f;
    lastFrameMs_ = t;

    if (!enabled) {
        phase_ = 0.f;
        return;
    }

    phase_ += dt * speed_;
    if (phase_ >= 1.0f) phase_ -= std::floor(phase_);
}
