// ====================================================================
// LightVoice: Event Loop Thread
// src/net/EventLoopThread.cc
//
// Implementation for the EventLoopThread class.
//
// Author: Gemini
// ====================================================================

#include "net/EventLoopThread.h"
#include "net/EventLoop.h"

namespace lightvoice {
namespace net {

EventLoopThread::EventLoopThread() {
}

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    if (loop_ != nullptr) {
        loop_->quit();
        if (thread_.joinable()) {
            thread_.join();
        }
    }
}

EventLoop* EventLoopThread::startLoop() {
    thread_ = std::thread(&EventLoopThread::threadFunc, this);

    EventLoop* loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        // Wait until the thread function has created the EventLoop
        cond_.wait(lock, [this] { return loop_ != nullptr; });
        loop = loop_;
    }

    return loop;
}

void EventLoopThread::threadFunc() {
    EventLoop loop; // Create loop on the stack of the new thread

    {
        std::lock_guard<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.loop(); // Start the event loop

    std::lock_guard<std::mutex> lock(mutex_);
    loop_ = nullptr;
}

} // namespace net
} // namespace lightvoice
