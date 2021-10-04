#pragma once

#include "Address.h"
#include "Socket.h"

#include <chrono>
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
    static const std::chrono::milliseconds kInitRetryTime_;
    static const std::chrono::milliseconds kMaxRetryTime_;

    void connectInLoop();
    void handleWrite();
    void removeChannel();
    int releaseSocket();
    void retry();

    EventLoop* loop_;
    Address serverAddr_;
    std::unique_ptr<Channel> channel_;
    NewConnCallback newConnCallback_;
    std::unique_ptr<Socket> socket_;
    std::chrono::milliseconds retryDelayTime_;
};

inline void Connector::setNewConnCallback(const NewConnCallback& cb)
{
    newConnCallback_ = cb;
}