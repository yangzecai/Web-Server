#pragma once

#include <chrono>
#include <functional>
#include <memory>

class Timer {
public:
    using TimePoint = std::chrono::high_resolution_clock::time_point;
    using TimeInterval = std::chrono::nanoseconds;
    using TimerCallback = std::function<void()>;

    Timer(const TimerCallback& cb, const TimePoint& expiration,
          const TimeInterval& interval);

    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    bool operator<(const Timer& rhs) const
    {
        return expiration_ < rhs.expiration_;
    }

    const TimePoint& getExpiration() const { return expiration_; }
    void updateExpiration(const TimePoint&);
    void callTimerCallback() { callback_(); }
    bool isRepeated() const { return repeat_; }

private:
    const TimerCallback callback_;
    TimePoint expiration_;
    const TimeInterval interval_;
    const bool repeat_;
};
