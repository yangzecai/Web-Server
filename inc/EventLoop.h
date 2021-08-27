#pragma once

#include <memory>
#include <thread>

class Poller;

class EventLoop {
public:
    EventLoop();
    ~EventLoop();

    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    void loop();

private:
    std::thread::id threadId_;
    bool isLooping_;
    std::unique_ptr<Poller> poller_;

    static thread_local EventLoop* loopInThisThread_;

    bool isInOwningThread() { return threadId_ == std::this_thread::get_id(); }
    void assertInOwningThread();
    void abortNotInOwningThread();
};