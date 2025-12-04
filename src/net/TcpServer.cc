// ====================================================================
// LightVoice: TCP Server
// src/net/TcpServer.cc
//
// Implementation for the TcpServer class.
//
// Author: Gemini
// ====================================================================

#include "net/TcpServer.h"
#include "net/EventLoop.h"
#include "net/EventLoopThreadPool.h"
#include "net/Acceptor.h"
#include "net/InetAddress.h"
#include "common/Logger.h"

namespace lightvoice {
namespace net {

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, std::string name)
    : loop_(loop),
      name_(std::move(name)),
      acceptor_(std::make_unique<Acceptor>(loop, listenAddr, true)),
      threadPool_(std::make_unique<EventLoopThreadPool>(loop, 0)),
      started_(false),
      nextConnId_(1) {
    acceptor_->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {
    loop_->assertInLoopThread();
    LOGGER_DEBUG("TcpServer::~TcpServer [{}] destructing", name_);
    for (auto& item : connections_) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(
            std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::setThreadNum(int numThreads) {
    assert(numThreads >= 0);
    threadPool_ = std::make_unique<EventLoopThreadPool>(loop_, numThreads);
}

void TcpServer::start() {
    if (!started_) {
        started_ = true;
        threadPool_->start();
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
    loop_->assertInLoopThread();
    EventLoop* ioLoop = threadPool_->getNextLoop();
    
    char buf[64];
    snprintf(buf, sizeof(buf), "-%s#%d", peerAddr.toIpPort().c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    LOGGER_INFO("TcpServer::newConnection [{}] - new connection [{}] from {}", name_, connName, peerAddr.toIpPort());
    
    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    
    TcpConnectionPtr conn = std::make_shared<TcpConnection>(ioLoop, connName, sockfd, localAddr, peerAddr);
    connections_[connName] = conn;
    
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));

    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
    loop_->assertInLoopThread();
    LOGGER_INFO("TcpServer::removeConnectionInLoop [{}] - connection {}", name_, conn->name());
    
    size_t n = connections_.erase(conn->name());
    assert(n == 1);
    
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

} // namespace net
} // namespace lightvoice
