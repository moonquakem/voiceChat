// ====================================================================
// LightVoice: TCP Connection
// src/net/TcpConnection.h
//
// Represents a single TCP connection. It manages the connection's
// lifecycle, handles reading and writing data, and invokes user-
// defined callbacks for events like message arrival and disconnection.
// It is managed by a std::shared_ptr and uses
// std::enable_shared_from_this to safely pass itself to callbacks.
//
// Author: Gemini
// ====================================================================

#pragma once

#include "common/noncopyable.h"
#include "net/Buffer.h"
#include <memory>
#include <functional>
#include <any>

// Forward declarations
struct tcp_info;
namespace google {
namespace protobuf {
class Message;
}
}

namespace lightvoice {
namespace net {

class EventLoop;
class Channel;
class InetAddress; // Assuming this exists for peer address

// Define a smart pointer for TcpConnection
class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

// Callback types
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using MessageCallback = std::function<void(const TcpConnectionPtr&, Buffer*, Timestamp)>;


class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(EventLoop* loop,
                  std::string name,
                  int sockfd,
                  const InetAddress& localAddr,
                  const InetAddress& peerAddr);
    ~TcpConnection();

    // Getters
    EventLoop* getLoop() const { return loop_; }
    const std::string& name() const { return name_; }
    const InetAddress& localAddress() const { return localAddr_; }
    const InetAddress& peerAddress() const { return peerAddr_; }
    bool connected() const { return state_ == kConnected; }

    // Send data (thread-safe)
    void send(const std::string& message);
    void send(Buffer* message); // Takes ownership

    // Shutdown connection
    void shutdown();

    // Setters for callbacks
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }

    // Called when the connection is established.
    void connectEstablished();
    // Called when the connection is being destroyed.
    void connectDestroyed();

    // Context for higher-level application data
    void setContext(const std::any& context) { context_ = context; }
    const std::any& getContext() const { return context_; }
    std::any* getMutableContext() { return &context_; }

private:
    enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };

    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void* data, size_t len);
    void shutdownInLoop();

    void setState(StateE s) { state_ = s; }

    EventLoop* loop_;
    const std::string name_;
    StateE state_;
    std::unique_ptr<Channel> channel_;
    const int sockfd_;
    const InetAddress& localAddr_;
    const InetAddress& peerAddr_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    CloseCallback closeCallback_;

    Buffer inputBuffer_;
    Buffer outputBuffer_;

    std::any context_;
};

} // namespace net
} // namespace lightvoice
