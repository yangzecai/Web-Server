#include "Channel.h"
#include "EventLoop.h"
#include "Log.h"
#include "Poller.h"
#include "TimerQueue.h"

#include <chrono>

#include <assert.h>

thread_local EventLoop* EventLoop::loopInThisThread_ = nullptr;

EventLoop::EventLoop()
    : threadId_(std::this_thread::get_id())
    , looping_(false)
    , quit_(false)
    , poller_(std::make_unique<Poller>(this))
    , timerQueue_(std::make_unique<TimerQueue>(this))
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
    }
    looping_ = false;
    LOG_TRACE << "Eventloop stop looping";
}

void EventLoop::runAt(const TimePoint& time, const TimerCallback& cb)
{
    timerQueue_->addTimer(cb, time, std::chrono::seconds(0));
}

void EventLoop::addChannel(Channel* channel)
{
    assert(channel->getOwnerLoop() == this);
    assertInOwningThread();
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
    assertInOwningThread();
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
    LOG_FATAL << "Eventloop " << loopInThisThread_
              << " is not called in the owing thread";
}
