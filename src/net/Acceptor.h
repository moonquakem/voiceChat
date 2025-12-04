// ====================================================================
// LightVoice: Acceptor
// src/net/Acceptor.h
//
// Responsible for accepting new TCP connections. It owns the
// listening socket and a Channel for handling incoming connection
// events. When a new connection arrives, it calls a user-defined
// callback.
//
// Author: Gemini
// ====================================================================

#pragma once

#include "common/noncopyable.h"
#include "net/Channel.h"
#include <functional>

namespace lightvoice {
namespace net {

class EventLoop;
class InetAddress;

class Acceptor : noncopyable {
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;

    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback& cb) {
        newConnectionCallback_ = cb;
    }

    bool listenning() const { return listenning_; }
    void listen();

private:
    void handleRead();

    EventLoop* loop_;
    int acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listenning_;
    int idleFd_;
};

} // namespace net
} // namespace lightvoice
