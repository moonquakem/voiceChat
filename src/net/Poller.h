// ====================================================================
// LightVoice: Poller
// src/net/Poller.h
//
// The abstract base class for I/O multiplexing. It's the core of
// the Reactor pattern. Concrete implementations will use specific
// mechanisms like epoll or select.
//
// Author: Gemini
// ====================================================================

#pragma once

#include "common/noncopyable.h"
#include <vector>
#include <map>

// Forward declarations
namespace lightvoice {
namespace net {
class Channel;
class EventLoop;
}
}

namespace lightvoice {
namespace net {

class Poller : noncopyable {
public:
    using ChannelList = std::vector<Channel*>;

    explicit Poller(EventLoop* loop);
    virtual ~Poller() = default;

    // Polls the I/O events. Must be called in the loop thread.
    virtual void poll(int timeoutMs, ChannelList* activeChannels) = 0;

    // Changes the interested I/O events. Must be called in the loop thread.
    virtual void updateChannel(Channel* channel) = 0;

    // Removes a channel. Must be called in the loop thread.
    virtual void removeChannel(Channel* channel) = 0;

    // Checks if a channel is in the poller.
    virtual bool hasChannel(Channel* channel) const;

    // Factory method to create a default poller for the OS.
    static Poller* newDefaultPoller(EventLoop* loop);

protected:
    using ChannelMap = std::map<int, Channel*>;
    ChannelMap channels_;

private:
    EventLoop* ownerLoop_;
};

} // namespace net
} // namespace lightvoice
