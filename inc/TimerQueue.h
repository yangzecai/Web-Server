#pragma once

#include "Channel.h"

#include <chrono>
#include <functional>
#include <memory>
#include <set>
#include <vector>

class EventLoop;
class Channel;
class Timer;

class TimerQueue {
public:
    using TimePoint = std::chrono::high_resolution_clock::time_point;
    using TimeInterval = std::chrono::nanoseconds;
    using TimerCallback = std::function<void()>;
    using TimerPtr = std::unique_ptr<Timer>;

    TimerQueue(EventLoop* loop);
    ~TimerQueue();

    TimerQueue(const TimerQueue&) = delete;
    TimerQueue& operator=(const TimerQueue&) = delete;

    void addTimer(const TimerCallback& cb, const TimePoint& tp,
                  const TimeInterval& ti);

private:
    int createTimerfdOrDie() const;
    void handleRead();
    void updateCurTime();
    const TimePoint& getCurTime() const;
    const TimePoint& getNextExpiration() const;
    std::vector<TimerPtr> getExpiredAndRemove();
    itimerspec getIntervalFromNowToNextExpiration() const;
    void resetTimer();

    class TimerPtrCmp {
    public:
        bool operator()(const TimerPtr& lhs, const TimerPtr& rhs) const;
    };

    EventLoop* loop_;
    int timerfd_;
    Channel channel_;
    std::multiset<TimerPtr, TimerPtrCmp> timers_;
    TimerPtr curTime_;
};