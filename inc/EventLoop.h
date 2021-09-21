#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

class Poller;
class Channel;
class PendingFuncQueue;
class TimerQueue;

class EventLoop {
public:
    using CallbackFunc = std::function<void()>;
    using TimePoint = std::chrono::high_resolution_clock::time_point;
    using TimeInterval = std::chrono::nanoseconds;

    EventLoop();
    ~EventLoop();

    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    void loop();
    void quit();

    void runAt(const TimePoint& time, const CallbackFunc& cb);
    void runAfter(const TimeInterval& delay, const CallbackFunc& cb);
    void runEvery(const TimeInterval& interval, const CallbackFunc& cb);

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

    std::thread::id threadId_;
    bool looping_;
    std::atomic<bool> quit_;
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;
    std::unique_ptr<PendingFuncQueue> funcQueue_;
    bool callingPendingFunc_;

    static const int kPollTimeoutMs = 10000;
    static thread_local EventLoop* loopInThisThread_;
};