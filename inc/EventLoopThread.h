#pragma once

#include <thread>
#include <condition_variable>

class EventLoop;

class EventLoopThread {
public:
    EventLoopThread();
    ~EventLoopThread();

    EventLoopThread(const EventLoopThread&) = delete;
    EventLoopThread& operator=(const EventLoopThread&) = delete;

    EventLoop* startLoop();
    void stopLoop();

private:
    void threadFunc();

    std::thread loopThread_;
    EventLoop* loop_;
    std::mutex mutex_;
    std::condition_variable cond_;
};