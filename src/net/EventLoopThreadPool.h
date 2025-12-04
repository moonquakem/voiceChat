// ====================================================================
// LightVoice: Event Loop Thread Pool
// src/net/EventLoopThreadPool.h
//
// Manages a pool of EventLoop threads. It's used by TcpServer to
// dispatch new connections to different threads for I/O handling,
// allowing the server to scale across multiple CPU cores.
//
// Author: Gemini
// ====================================================================

#pragma once

#include "common/noncopyable.h"
#include <vector>
#include <memory>

namespace lightvoice {
namespace net {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable {
public:
    EventLoopThreadPool(EventLoop* baseLoop, int numThreads);
    ~EventLoopThreadPool();

    // Starts the thread pool.
    void start();

    // Gets the next EventLoop in the pool (round-robin).
    EventLoop* getNextLoop();

private:
    EventLoop* baseLoop_;
    bool started_;
    int numThreads_;
    int next_; // For round-robin
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};

} // namespace net
} // namespace lightvoice
