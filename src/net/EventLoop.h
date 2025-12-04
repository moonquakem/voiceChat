// ====================================================================
// LightVoice: Event Loop
// src/net/EventLoop.h
//
// The heart of the Reactor pattern. Each EventLoop runs in a single
// thread and manages all I/O events for a set of file descriptors.
// It's responsible for calling the Poller and dispatching events
// to the appropriate Channels.
//
// Author: Gemini
// ====================================================================

#pragma once

#include "common/noncopyable.h"
#include <thread>
#include <vector>
#include <functional>
#include <mutex>

// Forward declarations
namespace lightvoice {
namespace net {
class Channel;
class Poller;
}
}

namespace lightvoice {
namespace net {

class EventLoop : noncopyable {
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    // The main event loop. Must not be called directly by users.
    void loop();
    void quit();

    // Runs a function in this event loop's thread. Thread-safe.
    void runInLoop(Functor cb);

    // Queues a function to be run in this loop. Thread-safe.
    void queueInLoop(Functor cb);

    // Asserts that the current thread is the one this loop belongs to.
    void assertInLoopThread() {
        if (!isInLoopThread()) {
            abortNotInLoopThread();
        }
    }

    bool isInLoopThread() const { return threadId_ == std::this_thread::get_id(); }

    // Channel management
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

private:
    void abortNotInLoopThread();
    void doPendingFunctors();

    using ChannelList = std::vector<Channel*>;

    bool looping_;
    std::atomic<bool> quit_;
    const std::thread::id threadId_;
    
    std::unique_ptr<Poller> poller_;
    ChannelList activeChannels_;

    std::mutex mutex_;
    std::vector<Functor> pendingFunctors_;
};

} // namespace net
} // namespace lightvoice
