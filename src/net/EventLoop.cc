// ====================================================================
// LightVoice: Event Loop
// src/net/EventLoop.cc
//
// Implementation for the EventLoop class.
//
// Author: Gemini
// ====================================================================

#include "net/EventLoop.h"
#include "net/Poller.h"
#include "net/Channel.h"
#include "common/Logger.h"

namespace lightvoice {
namespace net {

// Per-thread EventLoop instance
thread_local EventLoop* t_loopInThisThread = nullptr;

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      threadId_(std::this_thread::get_id()),
      poller_(Poller::newDefaultPoller(this)) {
    
    LOGGER_DEBUG("EventLoop created {} in thread {}", fmt::ptr(this), std::this_thread::get_id());
    if (t_loopInThisThread) {
        LOGGER_CRITICAL("Another EventLoop {} exists in this thread {}", fmt::ptr(t_loopInThisThread), std::this_thread::get_id());
    } else {
        t_loopInThisThread = this;
    }
}

EventLoop::~EventLoop() {
    LOGGER_DEBUG("EventLoop {} of thread {} destructs", fmt::ptr(this), threadId_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;
    LOGGER_DEBUG("EventLoop {} start looping", fmt::ptr(this));

    while (!quit_) {
        activeChannels_.clear();
        poller_->poll(10000, &activeChannels_); // Poll with a 10s timeout
        for (Channel* channel : activeChannels_) {
            channel->handleEvent();
        }
        doPendingFunctors();
    }

    LOGGER_DEBUG("EventLoop {} stop looping", fmt::ptr(this));
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
    // Note: If the loop is blocked in poll(), it won't quit immediately.
    // A wakeup mechanism (like writing to an eventfd) can be added here
    // for faster shutdown if needed.
}

void EventLoop::runInLoop(Functor cb) {
    if (isInLoopThread()) {
        cb();
    } else {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(Functor cb) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctors_.push_back(std::move(cb));
    }
    // Note: Add a wakeup mechanism here if needed.
}

void EventLoop::updateChannel(Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    return poller_->hasChannel(channel);
}

void EventLoop::abortNotInLoopThread() {
    LOGGER_CRITICAL("EventLoop::abortNotInLoopThread - EventLoop {} was created in threadId_ = {}, current thread id = {}",
                   fmt::ptr(this), threadId_, std::this_thread::get_id());
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (const Functor& functor : functors) {
        functor();
    }
}

} // namespace net
} // namespace lightvoice
