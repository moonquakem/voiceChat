// ====================================================================
// LightVoice: SocketsOps
// src/net/SocketsOps.h
//
// A collection of low-level socket operations, providing a thin
// C++ wrapper around the underlying C socket API. These are used
// by other classes in the networking layer.
//
// Author: Gemini
// ====================================================================

#pragma once

#include <arpa/inet.h>

namespace lightvoice {
namespace net {
namespace sockets {

// Create a non-blocking socket file descriptor, abort if failed.
int createNonblockingOrDie();

void bindOrDie(int sockfd, const struct sockaddr* addr);
void listenOrDie(int sockfd);
int accept(int sockfd, struct sockaddr_in6* addr);
void close(int sockfd);

void toIpPort(char* buf, size_t size, const struct sockaddr* addr);
void toIp(char* buf, size_t size, const struct sockaddr* addr);

struct sockaddr_in6 getLocalAddr(int sockfd);

int getSocketError(int sockfd);

const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);
struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr);

void setReuseAddr(int sockfd, bool on);
void setReusePort(int sockfd, bool on);

uint64_t hostToNetwork64(uint64_t host64);
uint32_t hostToNetwork32(uint32_t host32);
uint16_t hostToNetwork16(uint16_t host16);
uint64_t networkToHost64(uint64_t net64);
uint32_t networkToHost32(uint32_t net32);
uint16_t networkToHost16(uint16_t net16);

} // namespace sockets
} // namespace net
} // namespace lightvoice
