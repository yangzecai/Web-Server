#pragma once

#include <atomic>
#include <memory>
#include <thread>

class Poller;
class Channel;

class EventLoop {
public:
    EventLoop();
    ~EventLoop();

    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    void loop();
    void quit() { quit_ = true; }

    void addChannel(Channel* channel);
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);

private:
    std::thread::id threadId_;
    bool looping_;
    std::atomic<bool> quit_;
    std::unique_ptr<Poller> poller_;
    static const int kPollTimeoutMs = 10000;

    static thread_local EventLoop* loopInThisThread_;

    bool isInOwningThread() { return threadId_ == std::this_thread::get_id(); }
    void assertInOwningThread();
    void abortNotInOwningThread();
};