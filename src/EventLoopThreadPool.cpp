#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"
#include "EventLoop.h"

#include <cassert>

EventLoopThreadPool::EventLoopThreadPool(EventLoop* loop)
    : loop_(loop)
    , threadNum_(0)
    , threads_()
    , loops_()
    , next_(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
}

void EventLoopThreadPool::start()
{
    loop_->assertInOwningThread();
    threads_.reserve(threadNum_);
    loops_.reserve(threadNum_);
    for (int i = 0; i < threadNum_; ++i) {
        threads_.emplace_back(std::make_unique<EventLoopThread>());
        loops_.push_back(threads_.back()->startLoop());
    }
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
    loop_->assertInOwningThread();
    if (threadNum_) {
        int next = next_;
        next_ = (next_ + 1) % threadNum_;
        return loops_[next];
    } else {
        return loop_;
    }
}