#include "TimerQueue.h"
#include "EventLoop.h"
#include "Log.h"
#include "Timer.h"

#include <algorithm>
#include <vector>

#include <sys/timerfd.h>
#include <unistd.h>

bool TimerQueue::TimerPtrCmp::operator()(const TimerPtr& lhs,
                                         const TimerPtr& rhs) const
{
    return *lhs < *rhs;
}

TimerQueue::TimerQueue(EventLoop* loop)
    : loop_(loop)
    , timerfd_(createTimerfdOrDie())
    , channel_(timerfd_, loop)
    , timers_()
    , curTime_(std::make_unique<Timer>(
          []() {}, std::chrono::high_resolution_clock::now(),
          std::chrono::seconds(0)))
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
    curTime_->updateExpiration(
        std::chrono::high_resolution_clock::now());
}

const TimerQueue::TimePoint& TimerQueue::getCurTime() const
{
    return curTime_->getExpiration();
}

std::vector<TimerQueue::TimerPtr> TimerQueue::getExpiredAndRemove()
{
    std::vector<TimerPtr> expired;
    auto end = timers_.upper_bound(curTime_);
    assert(end == timers_.end() || *curTime_ < **end);
    for (auto iter = timers_.begin(); iter != end;) {
        expired.emplace_back(std::move(timers_.extract(iter++).value()));
    }
    return expired;
}

itimerspec TimerQueue::getIntervalFromNowToNextExpiration() const
{
    static const auto minInterval = std::chrono::microseconds(100);
    static const uint32_t kNanoPerSec = 1000UL * 1000 * 1000;

    assert(timers_.begin() != timers_.end());
    const TimePoint& nextExpiration = timers_.begin()->get()->getExpiration();
    auto interval = nextExpiration - getCurTime();
    interval = interval < minInterval ? minInterval : interval;

    itimerspec newValue;
    bzero(&newValue, sizeof(newValue));
    newValue.it_value.tv_sec = interval.count() / kNanoPerSec;
    newValue.it_value.tv_nsec = interval.count() % kNanoPerSec;
    return newValue;
}

void TimerQueue::resetTimer()
{
    if (timers_.begin() != timers_.end()) {
        auto newValue = getIntervalFromNowToNextExpiration();
        if (::timerfd_settime(timerfd_, 0, &newValue, NULL) != 0) {
            LOG_SYSFATAL << "TimerQueue::resetTimer";
        }
    }
}

void TimerQueue::addTimer(const TimerCallback& cb, const TimePoint& tp,
                          const TimeInterval& ti)
{
    loop_->assertInOwningThread();
    timers_.insert(std::make_unique<Timer>(cb, tp, ti));
    updateCurTime();
    resetTimer();
}

void TimerQueue::handleRead()
{
    loop_->assertInOwningThread();
    updateCurTime();
    std::vector<TimerPtr> expired = getExpiredAndRemove();
    for (size_t i = 0; i < expired.size(); ++i) {
        expired[i]->callTimerCallback();
        if (expired[i]->isRepeated()) {
            expired[i]->updateExpiration(getCurTime());
            timers_.insert(std::move(expired[i]));
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