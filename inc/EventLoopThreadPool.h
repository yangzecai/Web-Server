#pragma once

#include <memory>
#include <vector>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool {
public:
    EventLoopThreadPool(EventLoop* loop);
    ~EventLoopThreadPool();

    EventLoopThreadPool(const EventLoopThreadPool&) = delete;
    EventLoopThreadPool& operator=(const EventLoopThreadPool&) = delete;

    void setThreadNum(int num) { threadNum_ = num; }
    void start();
    EventLoop* getNextLoop();
    const std::vector<EventLoop*>& getLoops() const { return loops_; }

private:
    using EventLoopThreadPtr = std::unique_ptr<EventLoopThread>;

    EventLoop* loop_;
    int threadNum_;
    std::vector<EventLoopThreadPtr> threads_;
    std::vector<EventLoop*> loops_;
    int next_;
};