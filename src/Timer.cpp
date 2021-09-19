#include "Timer.h"

Timer::Timer(const TimerCallback& cb, const TimePoint& expiration,
             const TimeInterval& interval)
    : callback_(cb)
    , expiration_(expiration)
    , interval_(interval)
    , repeat_(interval.count() != 0)
{
}

void Timer::updateExpiration(const TimePoint& timepoint)
{
    expiration_ = timepoint + interval_;
}
