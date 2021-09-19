#pragma once

#include <functional>
#include <string>

class EventLoop;

class Channel {
public:
    using CallbackFunc = std::function<void()>;

    Channel(int fd, EventLoop* loop);
    ~Channel();

    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;

    void handleEvent();

    void setReadCallback(const CallbackFunc& cb) { readCallback_ = cb; }
    void setWriteCallback(const CallbackFunc& cb) { writeCallback_ = cb; }
    void setErrorCallback(const CallbackFunc& cb) { errorCallback_ = cb; }
    void setCloseCallback(const CallbackFunc& cb) { closeCallback_ = cb; }

    void enableRead();
    void disableRead();
    void enableWrite();
    void disableWrite();
    void disableAll();

    void close(); // Channel 拥有者析构自觉调用

    int getFd() const { return fd_; }
    int getEvent() const { return event_; }
    void setRevent(int revent) { revent_ = revent; }
    EventLoop* getOwnerLoop() { return loop_; }

private:
    void update();

    std::string reventToString();

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    int fd_;
    EventLoop* loop_;
    int event_;
    int revent_;

    CallbackFunc readCallback_;
    CallbackFunc writeCallback_;
    CallbackFunc errorCallback_;
    CallbackFunc closeCallback_;
};