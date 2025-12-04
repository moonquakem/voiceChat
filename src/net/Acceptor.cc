// ====================================================================
// LightVoice: Acceptor
// src/net/Acceptor.cc
//
// Implementation for the Acceptor class.
//
// Author: Gemini
// ====================================================================

#include "net/Acceptor.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include "net/SocketsOps.h"
#include "common/Logger.h"

#include <fcntl.h>
#include <unistd.h>

namespace lightvoice {
namespace net {

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
    : loop_(loop),
      acceptSocket_(sockets::createNonblockingOrDie()),
      acceptChannel_(loop, acceptSocket_),
      listenning_(false),
      idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
    assert(idleFd_ >= 0);
    sockets::setReuseAddr(acceptSocket_, true);
    sockets::setReusePort(acceptSocket_, reuseport);
    sockets::bindOrDie(acceptSocket_, listenAddr.getSockAddr());
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
    acceptChannel_.disableAll();
    acceptChannel_.remove();
    ::close(idleFd_);
}

void Acceptor::listen() {
    loop_->assertInLoopThread();
    listenning_ = true;
    sockets::listenOrDie(acceptSocket_);
    acceptChannel_.enableReading();
}

void Acceptor::handleRead() {
    loop_->assertInLoopThread();
    InetAddress peerAddr;
    int connfd = sockets::accept(acceptSocket_, peerAddr.getMutableSockAddrInet6());
    if (connfd >= 0) {
        if (newConnectionCallback_) {
            newConnectionCallback_(connfd, peerAddr);
        } else {
            sockets::close(connfd);
        }
    } else {
        LOGGER_ERROR("in Acceptor::handleRead");
        if (errno == EMFILE) {
            ::close(idleFd_);
            idleFd_ = ::accept(acceptSocket_, NULL, NULL);
            ::close(idleFd_);
            idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}

} // namespace net
} // namespace lightvoice
