// ====================================================================
// LightVoice: Timer Manager
// src/timer/TimerManager.cc
//
// Implementation for the TimerManager class. This file contains
// Linux-specific code using timerfd.
//
// Author: Gemini
// ====================================================================

#include "timer/TimerManager.h"
#include "net/EventLoop.h"
#include "net/Channel.h"
#include "common/Logger.h"

#ifdef __linux__
#include <sys/timerfd.h>
#include <unistd.h>
#endif

namespace lightvoice {

#ifdef __linux__
// Helper function to create a timerfd
int createTimerfd() {
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0) {
        LOGGER_CRITICAL("Failed to create timerfd");
    }
    return timerfd;
}

// Calculates time from now to 'when'
struct timespec howMuchTimeFromNow(Timestamp when) {
    int64_t microseconds = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
    if (microseconds < 100) {
        microseconds = 100; // Minimum 100us
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(microseconds / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>((microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
    return ts;
}

// Resets the timerfd to a new expiration
void resetTimerfd(int timerfd, Timestamp expiration) {
    struct itimerspec newValue;
    struct itimerspec oldValue;
    bzero(&newValue, sizeof newValue);
    bzero(&oldValue, sizeof oldValue);
    newValue.it_value = howMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if (ret) {
        LOGGER_ERROR("timerfd_settime() failed");
    }
}

// Reads from the timerfd to clear the event
void readTimerfd(int timerfd) {
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    if (n != sizeof howmany) {
        LOGGER_ERROR("TimerManager::handleRead() reads {} bytes instead of 8", n);
    }
}
#endif

TimerManager::TimerManager(net::EventLoop* loop)
    : loop_(loop),
#ifdef __linux__
      timerfd_(createTimerfd()),
      timerfdChannel_(std::make_unique<net::Channel>(loop, timerfd_))
#else
      timerfd_(0),
      timerfdChannel_(nullptr)
#endif
{
#ifdef __linux__
    LOGGER_DEBUG("TimerManager created, timerfd = {}", timerfd_);
    timerfdChannel_->setReadCallback(std::bind(&TimerManager::handleRead, this));
    timerfdChannel_->enableReading();
#else
    LOGGER_WARN("TimerManager is using a dummy implementation on non-Linux systems.");
#endif
}

TimerManager::~TimerManager() {
#ifdef __linux__
    timerfdChannel_->disableAll();
    timerfdChannel_->remove();
    ::close(timerfd_);
#endif
    for (const auto& pair : timers_) {
        delete pair.second;
    }
}

void TimerManager::addTimer(TimerCallback cb, Timestamp when, double interval) {
    Timer* timer = new Timer(std::move(cb), when, interval);
    loop_->runInLoop(std::bind(&TimerManager::addTimerInLoop, this, timer));
}

void TimerManager::addTimerInLoop(Timer* timer) {
    loop_->assertInLoopThread();
    bool earliestChanged = false;
    Timestamp when = timer->expiration();
    if (timers_.empty() || when < timers_.begin()->first) {
        earliestChanged = true;
    }

    timers_.insert({when, timer});

#ifdef __linux__
    if (earliestChanged) {
        resetTimerfd(timerfd_, when);
    }
#endif
}

void TimerManager::handleRead() {
#ifdef __linux__
    loop_->assertInLoopThread();
    Timestamp now(Timestamp::now());
    readTimerfd(timerfd_);

    std::vector<Timer*> expired = getExpired(now);

    for (Timer* timer : expired) {
        timer->run();
    }

    reset(expired, now);
#endif
}

std::vector<Timer*> TimerManager::getExpired(Timestamp now) {
    std::vector<Timer*> expired;
    // The first element in the set whose key is considered to go after `now`
    auto end = timers_.lower_bound({now, nullptr});
    for (auto it = timers_.begin(); it != end; ++it) {
        expired.push_back(it->second);
    }
    timers_.erase(timers_.begin(), end);
    return expired;
}

void TimerManager::reset(const std::vector<Timer*>& expired, Timestamp now) {
    for (Timer* timer : expired) {
        if (timer->repeat()) {
            timer->restart(now);
            addTimerInLoop(timer);
        } else {
            delete timer;
        }
    }

#ifdef __linux__
    Timestamp nextExpire;
    if (!timers_.empty()) {
        nextExpire = timers_.begin()->first;
    }
    if (nextExpire.microSecondsSinceEpoch() > 0) {
        resetTimerfd(timerfd_, nextExpire);
    }
#endif
}

} // namespace lightvoice
