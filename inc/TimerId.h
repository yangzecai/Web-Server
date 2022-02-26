#pragma once

#include <memory>
class Timer;
class TimerQueue;

class TimerId {
    friend class TimerQueue;
    TimerId(const std::shared_ptr<Timer>& timer)
        : timer_(timer)
    {
    }
    std::weak_ptr<Timer> timer_;
};
