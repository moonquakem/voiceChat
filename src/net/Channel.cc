// ====================================================================
// LightVoice: Channel
// src/net/Channel.cc
//
// Implementation for the Channel class.
//
// Author: Gemini
// ====================================================================

#include "net/Channel.h"
#include "net/EventLoop.h"
#include "common/Logger.h"

#ifdef __linux__
#include <poll.h>
#endif

namespace lightvoice {
namespace net {

const int Channel::kNoneEvent = 0;
#ifdef __linux__
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;
#else
// Placeholder for other systems
const int Channel::kReadEvent = 1;
const int Channel::kWriteEvent = 2;
#endif


Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop),
      fd_(fd),
      events_(0),
      revents_(0),
      index_(-1) {
    LOGGER_DEBUG("Channel created for fd {}", fd);
}

Channel::~Channel() {
    // This assertion ensures that the channel is properly managed,
    // i.e., it's not being destroyed while still being watched by the poller.
    assert(!loop_->hasChannel(this));
    LOGGER_DEBUG("Channel destroyed for fd {}", fd_);
}

void Channel::update() {
    loop_->updateChannel(this);
}

void Channel::remove() {
    loop_->removeChannel(this);
}

void Channel::handleEvent() {
#ifdef __linux__
    // Handle peer connection closed
    if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
        if (closeCallback_) closeCallback_();
    }

    // Handle errors
    if (revents_ & (POLLNVAL | POLLERR)) {
        if (errorCallback_) errorCallback_();
    }

    // Handle readable event
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
        if (readCallback_) readCallback_();
    }

    // Handle writable event
    if (revents_ & POLLOUT) {
        if (writeCallback_) writeCallback_();
    }
#endif
}

} // namespace net
} // namespace lightvoice
