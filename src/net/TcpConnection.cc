// ====================================================================
// LightVoice: TCP Connection
// src/net/TcpConnection.cc
//
// Implementation for the TcpConnection class.
//
// Author: Gemini
// ====================================================================

#include "net/TcpConnection.h"
#include "net/EventLoop.h"
#include "net/Channel.h"
#include "net/InetAddress.h" // Assuming this exists
#include "common/Logger.h"
#include <cerrno>
#include <unistd.h>

namespace lightvoice {
namespace net {

// Helper to create a socket
namespace sockets {
    int createNonblockingOrDie() {
#ifdef __linux__
        int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
#else
        int sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        // Set non-blocking on other systems
#endif
        if (sockfd < 0) {
            LOGGER_CRITICAL("sockets::createNonblockingOrDie");
        }
        return sockfd;
    }
}


TcpConnection::TcpConnection(EventLoop* loop,
                             std::string name,
                             int sockfd,
                             const InetAddress& localAddr,
                             const InetAddress& peerAddr)
    : loop_(loop),
      name_(std::move(name)),
      state_(kConnecting),
      channel_(std::make_unique<Channel>(loop, sockfd)),
      sockfd_(sockfd),
      localAddr_(localAddr),
      peerAddr_(peerAddr) {

    LOGGER_DEBUG("TcpConnection::ctor[{}] at {} fd={}", name_, fmt::ptr(this), sockfd);
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
}

TcpConnection::~TcpConnection() {
    LOGGER_DEBUG("TcpConnection::dtor[{}] at {} fd={}", name_, fmt::ptr(this), sockfd_);
    assert(state_ == kDisconnected);
}

void TcpConnection::send(const std::string& message) {
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(message.data(), message.size());
        } else {
            // To be thread-safe, copy the data and run in the loop
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, message.data(), message.size()));
        }
    }
}

void TcpConnection::send(Buffer* message) {
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(message->peek(), message->readableBytes());
            message->retrieveAll();
        } else {
            // To be thread-safe, use the buffer's string conversion
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, message->peek(), message->readableBytes()));
        }
    }
}


void TcpConnection::sendInLoop(const void* data, size_t len) {
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;

    if (state_ == kDisconnected) {
        LOGGER_WARN("disconnected, give up writing");
        return;
    }

    // If nothing in output buffer, try writing directly
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrote = ::write(sockfd_, data, len);
        if (nwrote >= 0) {
            remaining = len - nwrote;
            if (remaining == 0) {
                // Wrote everything
            }
        } else { // nwrote < 0
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                LOGGER_ERROR("TcpConnection::sendInLoop");
                if (errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
        }
    }

    if (!faultError && remaining > 0) {
        outputBuffer_.append(static_cast<const char*>(data) + nwrote, remaining);
        if (!channel_->isWriting()) {
            channel_->enableWriting();
        }
    }
}

void TcpConnection::shutdown() {
    if (state_ == kConnected) {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop() {
    loop_->assertInLoopThread();
    if (!channel_->isWriting()) {
        // We are not sending data, shut down now
        if (::shutdown(sockfd_, SHUT_WR) < 0) {
            LOGGER_ERROR("TcpConnection::shutdownInLoop");
        }
    }
}

void TcpConnection::connectEstablished() {
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);
    channel_->enableReading();
    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed() {
    loop_->assertInLoopThread();
    if (state_ == kConnected) {
        setState(kDisconnected);
        channel_->disableAll();
        connectionCallback_(shared_from_this());
    }
    channel_->remove();
}

void TcpConnection::handleRead(Timestamp receiveTime) {
    loop_->assertInLoopThread();
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(sockfd_, &savedErrno);
    if (n > 0) {
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    } else if (n == 0) {
        handleClose();
    } else {
        errno = savedErrno;
        LOGGER_ERROR("TcpConnection::handleRead");
        handleError();
    }
}

void TcpConnection::handleWrite() {
    loop_->assertInLoopThread();
    if (channel_->isWriting()) {
        ssize_t n = ::write(sockfd_, outputBuffer_.peek(), outputBuffer_.readableBytes());
        if (n > 0) {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0) {
                channel_->disableWriting();
                if (state_ == kDisconnecting) {
                    shutdownInLoop();
                }
            }
        } else {
            LOGGER_ERROR("TcpConnection::handleWrite");
        }
    }
}

void TcpConnection::handleClose() {
    loop_->assertInLoopThread();
    LOGGER_DEBUG("fd = {} state = {}", sockfd_, state_);
    assert(state_ == kConnected || state_ == kDisconnecting);
    setState(kDisconnected);
    channel_->disableAll();

    TcpConnectionPtr guardThis(shared_from_this());
    connectionCallback_(guardThis);
    closeCallback_(guardThis);
}

void TcpConnection::handleError() {
    int err;
    socklen_t len = sizeof(err);
    if (::getsockopt(sockfd_, SOL_SOCKET, SO_ERROR, &err, &len) < 0) {
        err = errno;
    }
    LOGGER_ERROR("TcpConnection::handleError [{}] - SO_ERROR = {}", name_, err);
}

} // namespace net
} // namespace lightvoice
