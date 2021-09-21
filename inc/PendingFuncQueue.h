#pragma once

#include "Channel.h"

#include <functional>
#include <vector>
#include <mutex>

class Channel;
class EventLoop;

class PendingFuncQueue {
public:
    using CallbackFunc = std::function<void()>;

    PendingFuncQueue(EventLoop* loop);
    ~PendingFuncQueue();

    PendingFuncQueue(const PendingFuncQueue&) = delete;
    PendingFuncQueue& operator=(const PendingFuncQueue&) = delete;

    void enqueue(const CallbackFunc& cb);
    void callPendingFunc();
    void notify();

private:
    int createEventfdOrDie() const;
    void handleRead();

    EventLoop* loop_;
    int eventfd_;
    Channel channel_;
    std::vector<CallbackFunc> funcQueue_;
    std::mutex mutex_;
    bool callingPendingFunc_;
};