#pragma once

#include "TimerQueue.h"

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

class Poller;
class Channel;
class PendingFuncQueue;

class EventLoop {
public:
    using CallbackFunc = std::function<void()>;
    using TimePoint = std::chrono::high_resolution_clock::time_point;

    EventLoop();
    ~EventLoop();

    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    void loop();
    void quit() { quit_ = true; }

    void runAt(const TimePoint& time, const CallbackFunc& cb);

    template <typename Rep, typename Period>
    void runAfter(const std::chrono::duration<Rep, Period>& delay,
                  const CallbackFunc& cb)
    {
        timerQueue_->addTimer(cb,
                              std::chrono::high_resolution_clock::now() + delay,
                              std::chrono::seconds(0));
    }

    template <typename Rep, typename Period>
    void runEvery(const std::chrono::duration<Rep, Period>& interval,
                  const CallbackFunc& cb)
    {
        timerQueue_->addTimer(
            cb, std::chrono::high_resolution_clock::now() + interval, interval);
    }

    void runInLoop(const CallbackFunc& cb);
    void queueInLoop(const CallbackFunc& cb);

    void addChannel(Channel* channel);
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);

    void assertInOwningThread();
    bool isInOwningThread() { return threadId_ == std::this_thread::get_id(); }

private:
    void abortNotInOwningThread();
    void wakeup();
    void enqueuePendingFuncQueue(const CallbackFunc& cb);

    std::thread::id threadId_;
    bool looping_;
    std::atomic<bool> quit_;
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;
    std::unique_ptr<PendingFuncQueue> funcQueue_;

    static const int kPollTimeoutMs = 10000;
    static thread_local EventLoop* loopInThisThread_;
};