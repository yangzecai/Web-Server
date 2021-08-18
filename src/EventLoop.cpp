#include "EventLoop.h"

#include <iostream>
#include <chrono>

#include <assert.h>

thread_local EventLoop *EventLoop::loopInThisThread_ = nullptr;

EventLoop::EventLoop()
    : threadId_(std::this_thread::get_id())
    , isLooping_(false)
{
    if (loopInThisThread_ == nullptr) {
        loopInThisThread_ = this;
    } else {
        abortNotUniqueLoopInThisThread();
    }
}

EventLoop::~EventLoop()
{
}

inline void EventLoop::assertInOwningThread()
{
    if (!isInOwningThread()) {
        abortNotInOwningThread();
    }
}

void EventLoop::abortNotInOwningThread()
{
    std::cout << threadId_ << " : "
              << "abort not in owning thread" << std::endl;
    abort();
}

void EventLoop::abortNotUniqueLoopInThisThread()
{
    std::cout << threadId_ << " : "
              << "abort not unique loop in this thread" << std::endl;
    abort();
}

void EventLoop::loop()
{
    assert(isLooping_ == false);
    assertInOwningThread();

    isLooping_ = true;
    std::this_thread::sleep_for(std::chrono::seconds(2));
}