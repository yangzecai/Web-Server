#pragma once

#include "Callbacks.h"

#include <set>
#include <memory>

class EventLoop;
class Address;
class Socket;
class TcpConnection;
class Acceptor;

class TcpServer {
public:
    TcpServer(EventLoop* loop, const Address& addr);
    ~TcpServer() {}

    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;

    void setConnectionCallback(const ConnectionCallback& cb);
    void setMessageCallback(const MessageCallback& cb);

    void start();

private:
    void addConnection(int connfd, const Address& clientAddr);
    void removeConnection(const TcpConnectionPtr& connPtr);

    EventLoop* loop_;
    std::unique_ptr<Acceptor> acceptor_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    std::set<TcpConnectionPtr> connections_;
};

inline void TcpServer::setConnectionCallback(const ConnectionCallback& cb)
{
    connectionCallback_ = cb;
}

inline void TcpServer::setMessageCallback(const MessageCallback& cb)
{
    messageCallback_ = cb;
}