// ====================================================================
// LightVoice: Timer
// src/timer/Timer.cc
//
// Implementation for the Timer class.
//
// Author: Gemini
// ====================================================================

#include "timer/Timer.h"

namespace lightvoice {

// Initialize the static atomic counter for created timers.
std::atomic<int64_t> Timer::s_numCreated_(0);

void Timer::restart(Timestamp now) {
    if (repeat_) {
        // If it's a repeating timer, calculate the next expiration time.
        expiration_ = Timestamp(now.microSecondsSinceEpoch() + interval_ * Timestamp::kMicroSecondsPerSecond);
    } else {
        // For non-repeating timers, set a distant future time to mark it as invalid.
        expiration_ = Timestamp(0);
    }
}

} // namespace lightvoice
