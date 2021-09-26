#pragma once

#include "Address.h"
#include "Callbacks.h"
#include "Buffer.h"

#include <memory>

class Channel;
class Socket;
class EventLoop;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(EventLoop* loop, int connfd, const Address& addr);
    ~TcpConnection();

    TcpConnection(const TcpConnection&) = delete;
    TcpConnection& operator=(const TcpConnection&) = delete;

    void setConnectionCallback(const ConnectionCallback& cb);
    void setMessageCallback(const MessageCallback& cb);
    void setCloseCallback(const CloseCallback& cb);

    void establish();

    const Address& getClientAddr() { return clientAddr_; }

private:
    void handleRead();
    void handleError();
    void handleClose();
    void handleWrite();

    EventLoop* loop_;
    std::unique_ptr<Socket> connSocket_;
    Address clientAddr_;
    std::unique_ptr<Channel> channel_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    CloseCallback closeCallback_;
    Buffer recvBuffer_;
};

inline void TcpConnection::setConnectionCallback(const ConnectionCallback& cb)
{
    connectionCallback_ = cb;
}

inline void TcpConnection::setMessageCallback(const MessageCallback& cb)
{
    messageCallback_ = cb;
}

inline void TcpConnection::setCloseCallback(const CloseCallback& cb)
{
    closeCallback_ = cb;
}