#pragma once

#include <thread>

class EventLoop {
public:
    EventLoop();
    ~EventLoop();

    EventLoop(const EventLoop &) = delete;
    EventLoop &operator=(const EventLoop &) = delete;

    bool isInOwningThread() { return threadId_ == std::this_thread::get_id(); }
    void assertInOwningThread();

    void loop();

private:
    std::thread::id threadId_;
    bool isLooping_;
    static thread_local EventLoop *loopInThisThread_;

    void abortNotInOwningThread();
    void abortNotUniqueLoopInThisThread();
};