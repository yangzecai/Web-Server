#pragma once

#include "Channel.h"
#include "Socket.h"

#include <functional>

class EventLoop;
class Address;

class Acceptor {
public:
    using NewConnCallback = std::function<void(int, const Address&)>;

    Acceptor(EventLoop* loop, const Address& addr);
    ~Acceptor();

    Acceptor(const Acceptor&) = delete;
    Acceptor& operator=(const Acceptor&) = delete;

    void listen();

    void setNewConnCallback(const NewConnCallback& cb);

private:
    void handleRead();

    EventLoop* loop_;
    Socket socket_;
    Channel channel_;
    NewConnCallback newConnCallback_;
};

inline void Acceptor::setNewConnCallback(const NewConnCallback& cb)
{
    newConnCallback_ = cb;
}