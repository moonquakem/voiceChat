// ====================================================================
// LightVoice: Timer
// src/timer/Timer.h
//
// Represents a single timed event. It holds the callback to be
// executed, its expiration time, and an optional interval for
// repeated execution.
//
// Author: Gemini
// ====================================================================

#pragma once

#include "common/noncopyable.h"
#include "common/Timestamp.h"
#include <functional>
#include <atomic>

namespace lightvoice {

using TimerCallback = std::function<void()>;

class Timer : noncopyable {
public:
    Timer(TimerCallback cb, Timestamp when, double interval)
        : callback_(std::move(cb)),
          expiration_(when),
          interval_(interval),
          repeat_(interval > 0.0),
          sequence_(s_numCreated_++) {}

    void run() const {
        callback_();
    }

    Timestamp expiration() const { return expiration_; }
    bool repeat() const { return repeat_; }
    int64_t sequence() const { return sequence_; }

    void restart(Timestamp now);

    static int64_t numCreated() { return s_numCreated_; }

private:
    const TimerCallback callback_;
    Timestamp expiration_;
    const double interval_;
    const bool repeat_;
    const int64_t sequence_;

    static std::atomic<int64_t> s_numCreated_;
};

} // namespace lightvoice
