#include "EventLoop.h"
#include "Channel.h"
#include "Log.h"
#include "PendingFuncQueue.h"
#include "Poller.h"
#include "TimerQueue.h"

#include <cassert>
#include <chrono>

#include <signal.h>

thread_local EventLoop* EventLoop::loopInThisThread_ = nullptr;

class IgnoreSigPipe {
public:
    IgnoreSigPipe() {
        ::signal(SIGPIPE, SIG_IGN);
    }
};

IgnoreSigPipe ignoreSigPipe;

EventLoop::EventLoop()
    : threadId_(std::this_thread::get_id())
    , looping_(false)
    , quit_(false)
    , poller_(std::make_unique<Poller>(this))
    , timerQueue_(std::make_unique<TimerQueue>(this))
    , funcQueue_(std::make_unique<PendingFuncQueue>(this))
    , callingPendingFunc_(false)
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
        funcQueue_->callPendingFunc();
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

void EventLoop::runAt(const TimePoint& time, const CallbackFunc& cb)
{
    timerQueue_->addTimer(cb, time, std::chrono::nanoseconds(0));
}

void EventLoop::runAfter(const TimeInterval& delay, const CallbackFunc& cb)
{
    timerQueue_->addTimer(cb, std::chrono::high_resolution_clock::now() + delay,
                          std::chrono::nanoseconds(0));
}

void EventLoop::runEvery(const TimeInterval& interval, const CallbackFunc& cb)
{
    timerQueue_->addTimer(
        cb, std::chrono::high_resolution_clock::now() + interval, interval);
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
    funcQueue_->enqueue(cb);
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
    funcQueue_->notify();
}