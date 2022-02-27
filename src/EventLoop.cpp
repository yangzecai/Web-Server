#include "EventLoop.h"
#include "Channel.h"
#include "Log.h"
#include "Poller.h"
#include "TimerQueue.h"
#include "Waker.h"

#include <cassert>
#include <chrono>

#include <signal.h>

thread_local EventLoop* EventLoop::loopInThisThread_ = nullptr;

class IgnoreSigPipe {
public:
    IgnoreSigPipe() { ::signal(SIGPIPE, SIG_IGN); }
};

IgnoreSigPipe ignoreSigPipe;

EventLoop::EventLoop()
    : threadId_(std::this_thread::get_id())
    , looping_(false)
    , quit_(false)
    , poller_(std::make_unique<Poller>(this))
    , timerQueue_(std::make_unique<TimerQueue>(this))
    , waker_(std::make_unique<Waker>(this))
    , callingPendingFunc_(false)
    , mutex_()
    , funcQueue_()
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
    assert(looping_ == false);
    assertInOwningThread();

    LOG_TRACE << "EventLoop strat lopping ...";

    looping_ = true;
    while (!quit_) {
        std::vector<Channel*> activeChannels =
            std::move(poller_->poll(kPollTimeoutMs));

        for (Channel* channel : activeChannels) {
            channel->handleEvent();
        }
        callingPendingFunc_ = true;
        callPendingFunc();
        callingPendingFunc_ = false;
    }
    looping_ = false;
    LOG_TRACE << "Eventloop stop looping";
}

void EventLoop::quit()
{
    quit_ = true;
    if (!isInOwningThread()) {
        wakeup();
    }
}

TimerId EventLoop::runAt(TimePoint time, const CallbackFunc& cb)
{
    return timerQueue_->addTimer(cb, time, std::chrono::nanoseconds(0));
}

TimerId EventLoop::runAfter(TimeInterval delay, const CallbackFunc& cb)
{
    return timerQueue_->addTimer(
        cb, std::chrono::high_resolution_clock::now() + delay,
        std::chrono::nanoseconds(0));
}

TimerId EventLoop::runEvery(TimeInterval interval, const CallbackFunc& cb)
{
    return timerQueue_->addTimer(
        cb, std::chrono::high_resolution_clock::now() + interval, interval);
}

void EventLoop::cancel(const TimerId& timerid)
{
    timerQueue_->removeTimer(timerid);
}

void EventLoop::runInLoop(const CallbackFunc& cb)
{
    if (isInOwningThread()) {
        cb();
    } else {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(const CallbackFunc& cb)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        funcQueue_.push_back(cb);
    }
    if (!isInOwningThread() || callingPendingFunc_) {
        wakeup();
    }
}

void EventLoop::addChannel(Channel* channel)
{
    assert(channel->getOwnerLoop() == this);
    poller_->addChannel(channel);
}
void EventLoop::updateChannel(Channel* channel)
{
    assert(channel->getOwnerLoop() == this);
    assertInOwningThread();
    poller_->updateChannel(channel);
}
void EventLoop::removeChannel(Channel* channel)
{
    assert(channel->getOwnerLoop() == this);
    poller_->removeChannel(channel);
}

inline void EventLoop::assertInOwningThread()
{
    if (!isInOwningThread()) {
        abortNotInOwningThread();
    }
}

void EventLoop::abortNotInOwningThread()
{
    LOG_FATAL << "Eventloop " << this << " is not called in the owing thread";
}

void EventLoop::wakeup()
{
    waker_->wakeup();
}

void EventLoop::callPendingFunc()
{
    std::vector<CallbackFunc> processingFuncQueue;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        processingFuncQueue.swap(funcQueue_);
    }
    callingPendingFunc_ = true;
    for (auto& func : processingFuncQueue) {
        func();
    }
    callingPendingFunc_ = false;
}
