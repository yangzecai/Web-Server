#include "Channel.h"
#include "EventLoop.h"
#include "Log.h"
#include "Poller.h"

#include <sys/epoll.h>

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(int fd, EventLoop* loop)
    : fd_(fd)
    , loop_(loop)
    , event_(kNoneEvent)
    , revent_(kNoneEvent)
    , readCallback_()
    , writeCallback_()
    , errorCallback_()
    , closeCallback_()
{
    loop_->addChannel(this);
}

// FIXME: fd_ 可能被别人 close 了
Channel::~Channel() { loop_->removeChannel(this); }

void Channel::handleEvent()
{
    LOG_TRACE << reventToString();
    if (revent_ & EPOLLERR) {
        if (errorCallback_) {
            errorCallback_();
        }
    }
    if (revent_ & (EPOLLIN)) {
        if (readCallback_) {
            readCallback_();
        }
    }
    if (revent_ & EPOLLOUT) {
        if (writeCallback_) {
            writeCallback_();
        }
    }
}

void Channel::enableRead()
{
    event_ |= kReadEvent;
    update();
}

void Channel::disableRead()
{
    event_ &= ~kReadEvent;
    update();
}

void Channel::enableWrite()
{
    event_ |= kWriteEvent;
    update();
}

void Channel::disableWrite()
{
    event_ &= ~kWriteEvent;
    update();
}

void Channel::disableAll()
{
    event_ = kNoneEvent;
    update();
}

inline void Channel::update() { loop_->updateChannel(this); }

std::string Channel::reventToString()
{
    std::string str("revent : ");
    if (revent_ & EPOLLIN) {
        str += "IN ";
    }
    if (revent_ & EPOLLOUT) {
        str += "OUT ";
    }
    if (revent_ & EPOLLRDHUP) {
        str += "RDHUP ";
    }
    if (revent_ & EPOLLPRI) {
        str += "PRI ";
    }
    if (revent_ & EPOLLERR) {
        str += "ERR ";
    }
    if (revent_ & EPOLLHUP) {
        str += "HUP ";
    }
    if (revent_ & EPOLLET) {
        str += "ET ";
    }
    if (revent_ & EPOLLONESHOT) {
        str += "ONESHOT ";
    }
    if (revent_ & EPOLLWAKEUP) {
        str += "WAKEUP ";
    }
    if (revent_ & EPOLLEXCLUSIVE) {
        str += "EXCLUSIVE ";
    }
    return str;
}