#pragma once

#include "Socket.h"
#include "Address.h"

#include <functional>

class EventLoop;
class TcpConnection;
class Channel;

class Connector {
public:
    using NewConnCallback = std::function<void(int)>;

    Connector(EventLoop* loop, const Address& serverAddr);
    ~Connector();

    Connector(const Connector&) = delete;
    Connector& operator=(const Connector&) = delete;

    void setNewConnCallback(const NewConnCallback& cb);

    void connect();

private:
    void connectInLoop();

    void handleWrite();

    EventLoop* loop_;
    Address serverAddr_;
    std::unique_ptr<Channel> channel_;
    NewConnCallback newConnCallback_;
    std::unique_ptr<Socket> socket_;
};

inline void Connector::setNewConnCallback(const NewConnCallback& cb)
{
    newConnCallback_ = cb;
}