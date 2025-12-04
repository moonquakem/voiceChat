// ====================================================================
// LightVoice: SocketsOps
// src/net/SocketsOps.cc
//
// Implementation of the low-level socket operations.
//
// Author: Gemini
// ====================================================================

#include "net/SocketsOps.h"
#include "common/Logger.h"
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>

namespace lightvoice {
namespace net {
namespace sockets {

int createNonblockingOrDie() {
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0) {
        LOGGER_CRITICAL("sockets::createNonblockingOrDie");
    }
    return sockfd;
}

void bindOrDie(int sockfd, const struct sockaddr* addr) {
    int ret = ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
    if (ret < 0) {
        LOGGER_CRITICAL("sockets::bindOrDie");
    }
}

void listenOrDie(int sockfd) {
    int ret = ::listen(sockfd, SOMAXCONN);
    if (ret < 0) {
        LOGGER_CRITICAL("sockets::listenOrDie");
    }
}

int accept(int sockfd, struct sockaddr_in6* addr) {
    socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
    int connfd = ::accept4(sockfd, sockaddr_cast(addr), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd < 0) {
        int savedErrno = errno;
        switch (savedErrno) {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO:
            case EPERM:
            case EMFILE:
                errno = savedErrno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                LOGGER_ERROR("unexpected error of ::accept {}", savedErrno);
                break;
            default:
                LOGGER_ERROR("unknown error of ::accept {}", savedErrno);
                break;
        }
    }
    return connfd;
}

void close(int sockfd) {
    if (::close(sockfd) < 0) {
        LOGGER_ERROR("sockets::close");
    }
}

void toIpPort(char* buf, size_t size, const struct sockaddr* addr) {
    toIp(buf, size, addr);
    size_t end = ::strlen(buf);
    const struct sockaddr_in* addr4 = reinterpret_cast<const struct sockaddr_in*>(addr);
    uint16_t port = sockets::networkToHost16(addr4->sin_port);
    snprintf(buf + end, size - end, ":%u", port);
}

void toIp(char* buf, size_t size, const struct sockaddr* addr) {
    if (addr->sa_family == AF_INET) {
        const struct sockaddr_in* addr4 = reinterpret_cast<const struct sockaddr_in*>(addr);
        ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
    } else if (addr->sa_family == AF_INET6) {
        const struct sockaddr_in6* addr6 = reinterpret_cast<const struct sockaddr_in6*>(addr);
        ::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(size));
    }
}

struct sockaddr_in6 getLocalAddr(int sockfd) {
    struct sockaddr_in6 localaddr;
    memset(&localaddr, 0, sizeof localaddr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
    if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0) {
        LOGGER_ERROR("sockets::getLocalAddr");
    }
    return localaddr;
}

int getSocketError(int sockfd) {
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);
    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        return errno;
    } else {
        return optval;
    }
}

const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr) {
  return static_cast<const struct sockaddr*>((const void*)(addr));
}

const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr) {
  return static_cast<const struct sockaddr*>((const void*)(addr));
}

struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr) {
  return static_cast<struct sockaddr*>((void*)(addr));
}

void setReuseAddr(int sockfd, bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof optval));
}

void setReusePort(int sockfd, bool on) {
#ifdef SO_REUSEPORT
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof optval));
    if (ret < 0 && on) {
        LOGGER_ERROR("SO_REUSEPORT failed.");
    }
#endif
}

uint64_t hostToNetwork64(uint64_t host64) { return htobe64(host64); }
uint32_t hostToNetwork32(uint32_t host32) { return htobe32(host32); }
uint16_t hostToNetwork16(uint16_t host16) { return htobe16(host16); }
uint64_t networkToHost64(uint64_t net64) { return be64toh(net64); }
uint32_t networkToHost32(uint32_t net32) { return be32toh(net32); }
uint16_t networkToHost16(uint16_t net16) { return be16toh(net16); }

} // namespace sockets
} // namespace net
} // namespace lightvoice
