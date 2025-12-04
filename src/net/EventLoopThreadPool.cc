// ====================================================================
// LightVoice: Event Loop Thread Pool
// src/net/EventLoopThreadPool.cc
//
// Implementation for the EventLoopThreadPool class.
//
// Author: Gemini
// ====================================================================

#include "net/EventLoopThreadPool.h"
#include "net/EventLoop.h"
#include "net/EventLoopThread.h"
#include "common/Logger.h"

namespace lightvoice {
namespace net {

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, int numThreads)
    : baseLoop_(baseLoop),
      started_(false),
      numThreads_(numThreads),
      next_(0) {
    if (numThreads_ < 0) {
        LOGGER_CRITICAL("numThreads must be >= 0");
    }
}

EventLoopThreadPool::~EventLoopThreadPool() {
    // EventLoopThreads will be destroyed, which quits their loops and joins threads.
}

void EventLoopThreadPool::start() {
    baseLoop_->assertInLoopThread();
    started_ = true;

    for (int i = 0; i < numThreads_; ++i) {
        threads_.emplace_back(std::make_unique<EventLoopThread>());
        loops_.push_back(threads_[i]->startLoop());
    }
}

EventLoop* EventLoopThreadPool::getNextLoop() {
    baseLoop_->assertInLoopThread();
    assert(started_);

    EventLoop* loop = baseLoop_;

    if (!loops_.empty()) {
        // Round-robin
        loop = loops_[next_];
        next_ = (next_ + 1) % loops_.size();
    }

    return loop;
}

} // namespace net
} // namespace lightvoice
