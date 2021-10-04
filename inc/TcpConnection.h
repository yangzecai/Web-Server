#pragma once

#include "Address.h"
#include "Buffer.h"
#include "Callbacks.h"

#include <memory>

class Channel;
class Socket;
class EventLoop;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
    friend class TcpServer;
    friend class TcpClient;

public:
    TcpConnection(EventLoop* loop, int connfd, const Address& addr);
    ~TcpConnection();

    TcpConnection(const TcpConnection&) = delete;
    TcpConnection& operator=(const TcpConnection&) = delete;

    void setMessageCallback(const MessageCallback& cb);
    void setWriteCompleteCallback(const WriteCompleteCallback& cb);

    const Address& getClientAddr() { return clientAddr_; }

    void send(const std::string& str);
    void send(std::string&& str);
    void send(const char* str, size_t len);

    void shutdown();

private:
    void handleRead();
    void handleError();
    void handleClose();
    void handleWrite();

    void setConnectionCallback(const ConnectionCallback& cb);
    void setCloseCallback(const CloseCallback& cb);
    void establish();

    void sendInLoop(const std::string& str);
    void sendInLoop(const char* str, size_t len);

    void shutdownInLoop();

    EventLoop* loop_;
    std::unique_ptr<Socket> connSocket_;
    Address clientAddr_;
    std::unique_ptr<Channel> channel_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    CloseCallback closeCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    Buffer recvBuffer_;
    Buffer sendBuffer_;
    bool disconnecting_; // FIXME: atomic
};

inline void TcpConnection::setConnectionCallback(const ConnectionCallback& cb)
{
    connectionCallback_ = cb;
}

inline void
TcpConnection::setWriteCompleteCallback(const WriteCompleteCallback& cb)
{
    writeCompleteCallback_ = cb;
}

inline void TcpConnection::setMessageCallback(const MessageCallback& cb)
{
    messageCallback_ = cb;
}

inline void TcpConnection::setCloseCallback(const CloseCallback& cb)
{
    closeCallback_ = cb;
}