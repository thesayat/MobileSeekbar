#pragma once
#include <cstdint>
#include <chrono>

namespace Common
{

namespace Time
{
    inline int64_t nowMs() {
        using namespace std::chrono;
        return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
    }
} // Time

} // Common

