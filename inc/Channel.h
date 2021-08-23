#pragma once

#include <functional>

class EventLoop;

class Channel {
public:
    using CallbackFunc = std::function<void()>;

    Channel(int fd, EventLoop *loop);
    ~Channel();

    Channel(const Channel &) = delete;
    Channel &operator=(const Channel &) = delete;

    void handleEvent();

    void setReadCallback(const CallbackFunc &cb) { readCallback_ = cb; }
    void setWriteCallback(const CallbackFunc &cb) { writeCallback_ = cb; }
    void setErrorCallback(const CallbackFunc &cb) { errorCallback_ = cb; }

    void setRevent(int revent) { revent_ = revent; }

private:
    int fd_;
    EventLoop *loop_;

    int event_;
    int revent_;

    CallbackFunc readCallback_;
    CallbackFunc writeCallback_;
    CallbackFunc errorCallback_;
};