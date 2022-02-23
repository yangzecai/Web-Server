#pragma once

#include "Channel.h"

class Waker {
public:
    Waker(EventLoop* loop);
    ~Waker();

    Waker(const Waker&) = delete;
    Waker& operator=(const Waker&) = delete;

    void wakeup();

private:
    int createEventfdOrDie() const;
    void handleRead();

    EventLoop* loop_;
    int eventfd_;
    Channel channel_;
};