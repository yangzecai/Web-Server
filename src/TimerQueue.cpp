#include "TimerQueue.h"
#include "EventLoop.h"
#include "Log.h"
#include "Timer.h"
#include "TimerId.h"

#include <algorithm>
#include <vector>

#include <sys/timerfd.h>
#include <unistd.h>

TimerQueue::TimerQueue(EventLoop* loop)
    : loop_(loop)
    , timerfd_(createTimerfdOrDie())
    , channel_(timerfd_, loop)
    , timers_()
    , curTime_(std::chrono::high_resolution_clock::now())
{
    channel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
    channel_.enableRead();
}

TimerQueue::~TimerQueue()
{
    channel_.close();
    ::close(timerfd_);
}

void TimerQueue::updateCurTime()
{
    curTime_ = std::chrono::high_resolution_clock::now();
}

void TimerQueue::readTimerfd()
{
    uint64_t unused;
    ssize_t n = ::read(timerfd_, &unused, sizeof(unused));
    if (n != sizeof(unused)) {
        LOG_FATAL << "TimerQueue::readTimerfd reads " << n
                  << " bytes instead of " << sizeof(unused);
    }
}

TimerQueue::TimePoint TimerQueue::getNextExpiration() const
{
    assert(!timers_.empty());
    return timers_.begin()->second->getExpiration();
}

std::vector<TimerQueue::TimerPtr> TimerQueue::getExpiredAndRemove()
{
    std::vector<TimerPtr> expired;
    std::pair<TimePoint, TimerPtr> node(curTime_ + std::chrono::nanoseconds(1),
                                        nullptr);
    auto end = timers_.upper_bound(node);
    assert(end == timers_.end() || curTime_ < end->second->getExpiration());
    for (auto iter = timers_.begin(); iter != end;) {
        expired.emplace_back(std::move(timers_.extract(iter++).value().second));
    }
    return expired;
}

itimerspec TimerQueue::getIntervalFromNowToNextExpiration() const
{
    static const auto minInterval = std::chrono::microseconds(100);
    static const uint32_t kNanoPerSec = 1000UL * 1000 * 1000;

    assert(!timers_.empty());
    auto interval = getNextExpiration() - curTime_;
    interval = interval < minInterval ? minInterval : interval;

    itimerspec newValue;
    bzero(&newValue, sizeof(newValue));
    newValue.it_value.tv_sec = interval.count() / kNanoPerSec;
    newValue.it_value.tv_nsec = interval.count() % kNanoPerSec;
    return newValue;
}

void TimerQueue::resetTimer()
{
    if (!timers_.empty()) {
        auto newValue = getIntervalFromNowToNextExpiration();
        if (::timerfd_settime(timerfd_, 0, &newValue, NULL) != 0) {
            LOG_SYSFATAL << "TimerQueue::resetTimer";
        }
    }
}

void TimerQueue::addTimerInLoop(TimerPtr timer)
{
    loop_->assertInOwningThread();
    if (timers_.empty() || timer->getExpiration() < getNextExpiration()) {
        timers_.emplace(timer->getExpiration(), std::move(timer));
        updateCurTime();
        resetTimer();
    } else {
        timers_.emplace(timer->getExpiration(), std::move(timer));
    }
}

TimerId TimerQueue::addTimer(const TimerCallback& cb, TimePoint tp,
                             TimeInterval ti)
{
    TimerPtr timer = std::make_shared<Timer>(cb, tp, ti);
    TimerId timerId(timer);
    loop_->runInLoop(
        std::bind(&TimerQueue::addTimerInLoop, this, timer));
    return timerId;
}

void TimerQueue::handleRead()
{
    LOG_TRACE << "TimerQueue::handleRead";
    loop_->assertInOwningThread();
    updateCurTime();
    readTimerfd();
    std::vector<TimerPtr> expired = getExpiredAndRemove();
    for (size_t i = 0; i < expired.size(); ++i) {
        expired[i]->callTimerCallback();
        if (expired[i]->isRepeated()) {
            expired[i]->updateExpiration(curTime_);
            TimePoint expiration = expired[i]->getExpiration();
            timers_.emplace(expiration, std::move(expired[i]));
        }
    }
    resetTimer();
}

int TimerQueue::createTimerfdOrDie() const
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd == -1) {
        LOG_SYSFATAL << "TimerQueue::TimerQueue";
    }
    return timerfd;
}