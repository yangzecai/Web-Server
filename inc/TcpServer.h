#pragma once

#include "Callbacks.h"

#include <set>
#include <memory>

class EventLoop;
class Address;
class Socket;
class TcpConnection;
class Acceptor;
class EventLoopThreadPool;

class TcpServer {
public:
    TcpServer(EventLoop* loop, const Address& addr);
    ~TcpServer();

    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;

    void setConnectionCallback(const ConnectionCallback& cb);
    void setMessageCallback(const MessageCallback& cb);
    void setCloseCallback(const CloseCallback& cb);
    void setWriteCompleteCallback(const WriteCompleteCallback& cb);

    void start();

    void setThreadNum(int num);

private:
    void addConnection(int connfd, const Address& clientAddr);
    void removeConnection(const TcpConnectionPtr& connPtr);
    void removeConnectionInLoop(const TcpConnectionPtr& connPtr);

    EventLoop* loop_;
    std::unique_ptr<Acceptor> acceptor_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    CloseCallback closeCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    std::set<TcpConnectionPtr> connections_;
    std::unique_ptr<EventLoopThreadPool> threadPool_;
};

inline void TcpServer::setConnectionCallback(const ConnectionCallback& cb)
{
    connectionCallback_ = cb;
}

inline void TcpServer::setMessageCallback(const MessageCallback& cb)
{
    messageCallback_ = cb;
}

inline void TcpServer::setCloseCallback(const CloseCallback& cb)
{
    closeCallback_ = cb;
}

inline void TcpServer::setWriteCompleteCallback(const WriteCompleteCallback& cb)
{
    writeCompleteCallback_ = cb;
}