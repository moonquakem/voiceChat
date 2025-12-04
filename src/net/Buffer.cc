// ====================================================================
// LightVoice: Buffer
// src/net/Buffer.cc
//
// Implementation for the Buffer class, specifically the readFd
// method for reading from a file descriptor.
//
// Author: Gemini
// ====================================================================

#include "net/Buffer.h"
#include "common/Logger.h"

#ifdef __linux__
#include <sys/uio.h>
#endif

namespace lightvoice {
namespace net {

ssize_t Buffer::readFd(int fd, int* savedErrno) {
#ifdef __linux__
    // Use a temporary stack buffer to avoid extra copies. This is a form
    // of "zero-copy" reading, where data goes from kernel directly to our buffer.
    char extrabuf[65536]; // 64KB
    struct iovec vec[2];
    const size_t writable = writableBytes();

    vec[0].iov_base = beginWrite();
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);

    // When writable is large enough, readv will only use the first vector.
    // If not, it will also fill extrabuf, which we then append to our buffer.
    const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);

    if (n < 0) {
        *savedErrno = errno;
    } else if (static_cast<size_t>(n) <= writable) {
        writerIndex_ += n;
    } else {
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable);
    }
    return n;
#else
    // Fallback for non-Linux systems
    char buf[65536];
    ssize_t n = ::recv(fd, buf, sizeof(buf), 0);
    if (n < 0) {
        *savedErrno = errno;
    } else if (n > 0) {
        append(buf, n);
    }
    return n;
#endif
}

} // namespace net
} // namespace lightvoice
