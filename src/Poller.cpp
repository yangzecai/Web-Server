#include "Poller.h"
#include "Channel.h"
#include "Log.h"

#include <sys/epoll.h>
#include <unistd.h>

Poller::Poller()
    : epollfd_(::epoll_create1(EPOLL_CLOEXEC))
    , events_(kInitEventListSize)
{
    if (epollfd_ < 0) {
        LOG_SYSFATAL << "Poller::Poller";
    }
}

Poller::~Poller() { ::close(epollfd_); }

Poller::ChannelList Poller::poll(int timeoutMs)
{
    int numEvents =
        ::epoll_wait(epollfd_, events_.data(), events_.size(), timeoutMs);
    if (numEvents == -1) {
        LOG_SYSFATAL << "Poller::poll";
    }

    LOG_TRACE << numEvents << " events happend";
    ChannelList activeChannels;
    activeChannels.reserve(numEvents);
    assert(activeChannels.capacity() >= static_cast<size_t>(numEvents));
    for (int i = 0; i < numEvents; ++i) {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->setRevent(events_[i].events);
        activeChannels.push_back(channel);
    }

    // FIXME : 限制 events_.size() 最大值
    if (static_cast<size_t>(numEvents) == events_.size()) {
        events_.resize(events_.size() * 2);
    }

    return activeChannels;
}

void Poller::addChannel(Channel* ch) { epollCtl(ch, EPOLL_CTL_ADD); }
void Poller::updateChannel(Channel* ch) { epollCtl(ch, EPOLL_CTL_MOD); }
void Poller::removeChannel(Channel* ch) { epollCtl(ch, EPOLL_CTL_DEL); }

void Poller::epollCtl(Channel* channel, int operation)
{
    epoll_event event;
    event.events = channel->getEvent();
    event.data.ptr = channel;
    if (::epoll_ctl(epollfd_, operation, channel->getFd(), &event) != 0) {
        LOG_SYSFATAL << "Poller::epollCtl " << operationToString(operation);
    }
}

const char* Poller::operationToString(int operation) const
{
    switch (operation) {
    case EPOLL_CTL_ADD:
        return "ADD";
        break;
    case EPOLL_CTL_MOD:
        return "MOD";
        break;
    case EPOLL_CTL_DEL:
        return "DEL";
        break;
    default:
        assert(false);
        return "Unknow";
    }
}
