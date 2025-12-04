// ====================================================================
// LightVoice: Channel
// src/net/Channel.h
//
// A Channel represents an open file descriptor (like a socket) and
// the events it's interested in (e.g., readable, writable). It does
// not own the file descriptor. It's the bridge between the I/O
// multiplexing (Poller) and the event handling logic.
//
// Author: Gemini
// ====================================================================

#pragma once

#include "common/noncopyable.h"
#include <functional>

namespace lightvoice {
namespace net {

class EventLoop; // Forward declaration

class Channel : noncopyable {
public:
    using EventCallback = std::function<void()>;

    Channel(EventLoop* loop, int fd);
    ~Channel();

    // Handles the event dispatched by the EventLoop.
    void handleEvent();

    // Setters for the different event callbacks.
    void setReadCallback(EventCallback cb) { readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

    int fd() const { return fd_; }
    int events() const { return events_; }
    void set_revents(int revt) { revents_ = revt; }
    bool isNoneEvent() const { return events_ == kNoneEvent; }

    // Enable/disable interest in specific events.
    void enableReading() { events_ |= kReadEvent; update(); }
    void disableReading() { events_ &= ~kReadEvent; update(); }
    void enableWriting() { events_ |= kWriteEvent; update(); }
    void disableWriting() { events_ &= ~kWriteEvent; update(); }
    void disableAll() { events_ = kNoneEvent; update(); }

    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent; }

    // For Poller
    int index() { return index_; }
    void set_index(int idx) { index_ = idx; }

    EventLoop* ownerLoop() { return loop_; }
    void remove();

private:
    void update();

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_;
    const int fd_;
    int events_;  // Events this channel is interested in
    int revents_; // Events that have occurred
    int index_;   // Used by Poller

    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};

} // namespace net
} // namespace lightvoice
