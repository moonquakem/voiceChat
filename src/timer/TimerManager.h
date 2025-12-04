// ====================================================================
// LightVoice: Timer Manager
// src/timer/TimerManager.h
//
// Manages all timed events in an EventLoop. It uses a min-heap
// to efficiently find the next timer to expire and a timerfd
// for high-precision, low-overhead notifications from the kernel.
// This is a Linux-specific implementation.
//
// Author: Gemini
// ====================================================================

#pragma once

#include "common/noncopyable.h"
#include "common/Timestamp.h"
#include "timer/Timer.h"

#include <vector>
#include <set>
#include <memory>

// Forward declarations
namespace lightvoice {
namespace net {
class Channel;
class EventLoop;
}
}

namespace lightvoice {

class TimerManager : noncopyable {
public:
    explicit TimerManager(net::EventLoop* loop);
    ~TimerManager();

    // Adds a new timer. Thread-safe.
    void addTimer(TimerCallback cb, Timestamp when, double interval);

    // Cancels a timer. Thread-safe. (Not implemented for simplicity)
    // void cancel(TimerId timerId);

private:
    using TimerList = std::set<std::pair<Timestamp, Timer*>>;

    // Functions to be called within the loop thread
    void addTimerInLoop(Timer* timer);
    void handleRead();

    // Get expired timers
    std::vector<Timer*> getExpired(Timestamp now);

    // Reset expired timers
    void reset(const std::vector<Timer*>& expired, Timestamp now);

    net::EventLoop* loop_;
    const int timerfd_;
    std::unique_ptr<net::Channel> timerfdChannel_;
    TimerList timers_; // Timers sorted by expiration
};

} // namespace lightvoice
