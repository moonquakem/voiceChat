// ====================================================================
// LightVoice: Poller
// src/net/Poller.cc
//
// Implementation of the Poller base class and the epoll-based
// concrete Poller. This is Linux-specific.
//
// Author: Gemini
// ====================================================================

#include "net/Poller.h"
#include "net/Channel.h"
#include "common/Logger.h"

#ifdef __linux__
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>
#endif

namespace lightvoice {
namespace net {

// --- Base Poller Implementation ---

Poller::Poller(EventLoop* loop) : ownerLoop_(loop) {}

bool Poller::hasChannel(Channel* channel) const {
    auto it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel;
}

// --- EPollPoller Implementation ---
#ifdef __linux__
class EPollPoller : public Poller {
public:
    explicit EPollPoller(EventLoop* loop);
    ~EPollPoller() override;

    void poll(int timeoutMs, ChannelList* activeChannels) override;
    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;

private:
    static const int kInitEventListSize = 16;
    void update(int operation, Channel* channel);

    int epollfd_;
    std::vector<struct epoll_event> events_;
};

EPollPoller::EPollPoller(EventLoop* loop)
    : Poller(loop),
      epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize) {
    if (epollfd_ < 0) {
        LOGGER_CRITICAL("EPollPoller::EPollPoller failed in epoll_create1");
    }
}

EPollPoller::~EPollPoller() {
    ::close(epollfd_);
}

void EPollPoller::poll(int timeoutMs, ChannelList* activeChannels) {
    int numEvents = ::epoll_wait(epollfd_, events_.data(), static_cast<int>(events_.size()), timeoutMs);
    int savedErrno = errno;

    if (numEvents > 0) {
        for (int i = 0; i < numEvents; ++i) {
            Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
            channel->set_revents(events_[i].events);
            activeChannels->push_back(channel);
        }
        if (static_cast<size_t>(numEvents) == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    } else if (numEvents == 0) {
        // Nothing happened
    } else {
        if (savedErrno != EINTR) {
            errno = savedErrno;
            LOGGER_ERROR("EPollPoller::poll() error");
        }
    }
}

void EPollPoller::updateChannel(Channel* channel) {
    const int index = channel->index();
    if (index < 0) { // New channel
        assert(channels_.find(channel->fd()) == channels_.end());
        channels_[channel->fd()] = channel;
        channel->set_index(0); // Mark as added
        update(EPOLL_CTL_ADD, channel);
    } else { // Existing channel
        assert(channels_.find(channel->fd()) != channels_.end());
        assert(channels_[channel->fd()] == channel);
        if (channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(-1); // Mark as removed
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPollPoller::removeChannel(Channel* channel) {
    int fd = channel->fd();
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->isNoneEvent());
    size_t n = channels_.erase(fd);
    assert(n == 1);

    if (channel->index() >= 0) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(-1);
}

void EPollPoller::update(int operation, Channel* channel) {
    struct epoll_event event;
    bzero(&event, sizeof(event));
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();

    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
        LOGGER_ERROR("epoll_ctl op={} fd={} failed", operation, fd);
    }
}

#endif // __linux__

// --- Factory Method ---

Poller* Poller::newDefaultPoller(EventLoop* loop) {
#ifdef __linux__
    return new EPollPoller(loop);
#else
    // Fallback for other systems (e.g., a PollPoller or SelectPoller)
    LOGGER_CRITICAL("No default poller implementation for this OS.");
    return nullptr;
#endif
}

} // namespace net
} // namespace lightvoice
