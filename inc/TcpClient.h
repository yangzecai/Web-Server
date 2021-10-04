#pragma once

#include "Address.h"
#include "Callbacks.h"

#include <memory>

class Connector;
class EventLoop;
class Address;
class TcpConnection;

class TcpClient {
public:
    TcpClient(EventLoop* loop, const Address& serverAddr);
    ~TcpClient();

    TcpClient(const TcpClient&) = delete;
    TcpClient& operator=(const TcpClient&) = delete;

    void start();

    void setConnectionCallback(const ConnectionCallback&);
    void setMessageCallback(const MessageCallback&);
    void setCloseCallback(const CloseCallback&);
    void setWriteCompleteCallback(const WriteCompleteCallback&);

private:
    void addConnection(int sockfd);
    void removeConnection();

    EventLoop* loop_;
    Address serverAddr_;
    std::unique_ptr<Connector> connector_;
    std::shared_ptr<TcpConnection> connection_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    CloseCallback closeCallback_;
    WriteCompleteCallback writeCompleteCallback_;
};

inline void TcpClient::setConnectionCallback(const ConnectionCallback& cb)
{
    connectionCallback_ = cb;
}

inline void TcpClient::setMessageCallback(const MessageCallback& cb)
{
    messageCallback_ = cb;
}

inline void TcpClient::setCloseCallback(const CloseCallback& cb)
{
    closeCallback_ = cb;
}

inline void TcpClient::setWriteCompleteCallback(const WriteCompleteCallback& cb)
{
    writeCompleteCallback_ = cb;
}