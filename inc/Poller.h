#pragma once

#include <vector>

class EventLoop;
class Channel;
struct epoll_event;

class Poller {
public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop* loop);
    ~Poller();

    Poller(const Poller&) = delete;
    Poller& operator=(const Poller&) = delete;

    ChannelList poll(int timeoutMs);

    void addChannel(Channel* channel);
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);

private:
    void epollCtl(Channel* channel, int operation);
    const char* operationToString(int operation) const;

    using EventList = std::vector<epoll_event>;

    EventLoop* loop_;
    int epollfd_;
    EventList events_;
    static const int kInitEventListSize = 16;
};