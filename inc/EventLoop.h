#pragma once

#include "TimerQueue.h"

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <thread>

class Poller;
class Channel;

class EventLoop {
public:
    using TimerCallback = std::function<void()>;
    using TimePoint = std::chrono::high_resolution_clock::time_point;

    EventLoop();
    ~EventLoop();

    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    void loop();
    void quit() { quit_ = true; }

    void runAt(const TimePoint& time, const TimerCallback& cb);

    template <typename Rep, typename Period>
    void runAfter(const std::chrono::duration<Rep, Period>& delay,
                  const TimerCallback& cb)
    {
        timerQueue_->addTimer(cb,
                              std::chrono::high_resolution_clock::now() + delay,
                              std::chrono::seconds(0));
    }

    template <typename Rep, typename Period>
    void runEvery(const std::chrono::duration<Rep, Period>& interval,
                  const TimerCallback& cb)
    {
        timerQueue_->addTimer(
            cb, std::chrono::high_resolution_clock::now() + interval, interval);
    }

    void addChannel(Channel* channel);
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);

    void assertInOwningThread();

private:
    std::thread::id threadId_;
    bool looping_;
    std::atomic<bool> quit_;
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;

    static const int kPollTimeoutMs = 10000;
    static thread_local EventLoop* loopInThisThread_;

    bool isInOwningThread() { return threadId_ == std::this_thread::get_id(); }
    void abortNotInOwningThread();
};