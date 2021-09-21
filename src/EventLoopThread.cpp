#include "EventLoopThread.h"
#include "EventLoop.h"

#include <cassert>

EventLoopThread::EventLoopThread()
    : loopThread_()
    , loop_(nullptr)
    , mutex_()
    , cond_()
{
}

EventLoopThread::~EventLoopThread()
{
    if (loop_ != nullptr) {
        stopLoop();
    }
}

void EventLoopThread::threadFunc()
{
    EventLoop loop;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }
    loop.loop();
}

EventLoop* EventLoopThread::startLoop()
{
    loopThread_ = std::thread(std::bind(&EventLoopThread::threadFunc, this));
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (loop_ == nullptr) {
            cond_.wait(lock);
        }
    }
    return loop_;
}

void EventLoopThread::stopLoop()
{
    assert(loop_ != nullptr);
    loop_->quit();
    loopThread_.join();
    loop_ = nullptr;
}