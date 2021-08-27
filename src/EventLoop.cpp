#include "EventLoop.h"
#include "Log.h"
#include "Poller.h"

#include <chrono>
#include <iostream>

#include <assert.h>

thread_local EventLoop* EventLoop::loopInThisThread_ = nullptr;

EventLoop::EventLoop()
    : threadId_(std::this_thread::get_id())
    , isLooping_(false)
    , poller_(new Poller())
{
    LOG_TRACE << "EventLoop is created";
    if (loopInThisThread_ != nullptr) {
        LOG_FATAL << "Another EventLoop " << loopInThisThread_
                  << " exists in this thread";
    }
    loopInThisThread_ = this;
}

EventLoop::~EventLoop() {}

void EventLoop::loop()
{
    assert(isLooping_ == false);
    assertInOwningThread();

    isLooping_ = true;
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

inline void EventLoop::assertInOwningThread()
{
    if (!isInOwningThread()) {
        abortNotInOwningThread();
    }
}

void EventLoop::abortNotInOwningThread()
{
    LOG_FATAL << "Eventloop " << loopInThisThread_
              << " is not called in the owing thread";
}
