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
class TimerId;

class TimerQueue {
public:
    using TimePoint = std::chrono::high_resolution_clock::time_point;
    using TimeInterval = std::chrono::nanoseconds;
    using TimerCallback = std::function<void()>;
    using TimerPtr = std::shared_ptr<Timer>;

    TimerQueue(EventLoop* loop);
    ~TimerQueue();

    TimerQueue(const TimerQueue&) = delete;
    TimerQueue& operator=(const TimerQueue&) = delete;

    TimerId addTimer(const TimerCallback& cb, TimePoint tp, TimeInterval ti);
    void removeTimer(const TimerId& timerid);

private:
    int createTimerfdOrDie() const;
    void handleRead();
    void updateCurTime();
    void readTimerfd();
    TimePoint getNextExpiration() const;
    std::vector<TimerPtr> getExpiredAndRemove();
    itimerspec getIntervalFromNowToNextExpiration() const;
    void resetTimer();
    void addTimerInLoop(TimerPtr timer);
    void removeTimerInLoop(const TimerId& timerid);

    EventLoop* loop_;
    int timerfd_;
    Channel channel_;
    std::set<std::pair<TimePoint, TimerPtr>> timers_;
    TimePoint curTime_;
};