// ====================================================================
// LightVoice: InetAddress
// src/net/InetAddress.h
//
// A wrapper for the socket address structures (sockaddr_in and
// sockaddr_in6). It provides a more C++ friendly interface for
// handling network addresses.
//
// Author: Gemini
// ====================================================================

#pragma once

#include <string>
#include <netinet/in.h>

namespace lightvoice {
namespace net {
namespace sockets {
    const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);
}

class InetAddress {
public:
    // Constructs an endpoint with given port.
    explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false, bool ipv6 = false);

    // Constructs an endpoint with given ip and port.
    InetAddress(std::string ip, uint16_t port, bool ipv6 = false);

    // Constructs an endpoint with given struct sockaddr_in.
    explicit InetAddress(const struct sockaddr_in& addr) : addr_(addr) {}
    explicit InetAddress(const struct sockaddr_in6& addr) : addr6_(addr) {}

    sa_family_t family() const { return addr_.sin_family; }
    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t toPort() const;

    const struct sockaddr* getSockAddr() const { return sockets::sockaddr_cast(&addr6_); }
    void setSockAddrInet6(const struct sockaddr_in6& addr6) { addr6_ = addr6; }

private:
    union {
        struct sockaddr_in addr_;
        struct sockaddr_in6 addr6_;
    };
};

} // namespace net
} // namespace lightvoice
