// ====================================================================
// LightVoice: TCP Server
// src/net/TcpServer.h
//
// The main class for creating a TCP server. It manages the listening
// socket, accepts new connections, and uses an EventLoopThreadPool
// to distribute the I/O load of these connections across multiple
// threads.
//
// Author: Gemini
// ====================================================================

#pragma once

#include "common/noncopyable.h"
#include "net/TcpConnection.h"
#include <map>
#include <string>
#include <memory>

// Forward declarations
namespace lightvoice {
namespace net {
class EventLoop;
class EventLoopThreadPool;
class InetAddress;
class Acceptor;
}
}

namespace lightvoice {
namespace net {

class TcpServer : noncopyable {
public:
    TcpServer(EventLoop* loop, const InetAddress& listenAddr, std::string name);
    ~TcpServer();

    // Sets the number of I/O threads.
    void setThreadNum(int numThreads);

    // Starts the server.
    void start();

    // Setters for user callbacks.
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }

private:
    // Called by Acceptor when a new connection arrives.
    void newConnection(int sockfd, const InetAddress& peerAddr);
    // Called when a connection is closed.
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;

    EventLoop* loop_; // The main loop for accepting connections
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;
    std::unique_ptr<EventLoopThreadPool> threadPool_;
    
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;

    bool started_;
    int nextConnId_;
    ConnectionMap connections_;
};

} // namespace net
} // namespace lightvoice
