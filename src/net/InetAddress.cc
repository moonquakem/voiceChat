// ====================================================================
// LightVoice: InetAddress
// src/net/InetAddress.cc
//
// Implementation for the InetAddress class.
//
// Author: Gemini
// ====================================================================

#include "net/InetAddress.h"
#include "common/Logger.h"
#include <cstring>
#include <arpa/inet.h>
#include "SocketsOps.h"


namespace lightvoice {
namespace net {

static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;

InetAddress::InetAddress(uint16_t port, bool loopbackOnly, bool ipv6) {
    static_assert(offsetof(InetAddress, addr6_) == 0, "addr6_ offset is 0");
    static_assert(offsetof(InetAddress, addr_) == 0, "addr_ offset is 0");
    if (ipv6) {
        memset(&addr6_, 0, sizeof addr6_);
        addr6_.sin6_family = AF_INET6;
        in6_addr ip = loopbackOnly ? in6addr_loopback : in6addr_any;
        addr6_.sin6_addr = ip;
        addr6_.sin6_port = sockets::hostToNetwork16(port);
    } else {
        memset(&addr_, 0, sizeof addr_);
        addr_.sin_family = AF_INET;
        in_addr_t ip = loopbackOnly ? kInaddrLoopback : kInaddrAny;
        addr_.sin_addr.s_addr = sockets::hostToNetwork32(ip);
        addr_.sin_port = sockets::hostToNetwork16(port);
    }
}

InetAddress::InetAddress(std::string ip, uint16_t port, bool ipv6) {
    if (ipv6) {
        memset(&addr6_, 0, sizeof addr6_);
        addr6_.sin6_family = AF_INET6;
        addr6_.sin6_port = sockets::hostToNetwork16(port);
        if (::inet_pton(AF_INET6, ip.c_str(), &addr6_.sin6_addr) <= 0) {
            LOGGER_ERROR("sockets::fromIpPort");
        }
    } else {
        memset(&addr_, 0, sizeof addr_);
        addr_.sin_family = AF_INET;
        addr_.sin_port = sockets::hostToNetwork16(port);
        if (::inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr) <= 0) {
            LOGGER_ERROR("sockets::fromIpPort");
        }
    }
}

std::string InetAddress::toIpPort() const {
    char buf[64] = "";
    sockets::toIpPort(buf, sizeof buf, getSockAddr());
    return buf;
}

std::string InetAddress::toIp() const {
    char buf[64] = "";
    sockets::toIp(buf, sizeof buf, getSockAddr());
    return buf;
}

uint16_t InetAddress::toPort() const {
    return sockets::networkToHost16(addr_.sin_port);
}

} // namespace net
} // namespace lightvoice
