// ====================================================================
// LightVoice: Event Loop Thread
// src/net/EventLoopThread.h
//
// A helper class that starts an EventLoop in a dedicated thread.
// This is the building block for an EventLoopThreadPool.
//
// Author: Gemini
// ====================================================================

#pragma once

#include "common/noncopyable.h"
#include <thread>
#include <mutex>
#include <condition_variable>

namespace lightvoice {
namespace net {

class EventLoop; // Forward declaration

class EventLoopThread : noncopyable {
public:
    EventLoopThread();
    ~EventLoopThread();

    // Starts the thread and returns the EventLoop pointer.
    EventLoop* startLoop();

private:
    void threadFunc();

    EventLoop* loop_ = nullptr;
    bool exiting_ = false;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

} // namespace net
} // namespace lightvoice
