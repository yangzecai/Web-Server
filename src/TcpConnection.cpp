#include "TcpConnection.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Log.h"
#include "Socket.h"

#include <sys/socket.h>
#include <sys/types.h>

TcpConnection::TcpConnection(EventLoop* loop, int connfd, const Address& addr)
    : loop_(loop)
    , connSocket_(std::make_unique<Socket>(connfd))
    , clientAddr_(addr)
    , channel_(std::make_unique<Channel>(connfd, loop))
    , connectionCallback_()
    , messageCallback_()
    , recvBuffer_()
{
    connSocket_->setNonblock(true);
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
}

TcpConnection::~TcpConnection()
{
    channel_->close();
}

void TcpConnection::establish()
{
    loop_->assertInOwningThread();
    connectionCallback_(shared_from_this());
    channel_->enableRead();
}

void TcpConnection::handleRead()
{
    size_t len = recvBuffer_.writeFromFd(connSocket_->getFd());
    LOG_TRACE << "TcpConnection::handleRead read " << len << " bytes";
    if (len > 0) {
        messageCallback_(shared_from_this(), recvBuffer_);
    } else if (len == 0) {
        handleClose();
    } else {
        handleError();
    }
}

void TcpConnection::handleError()
{
    LOG_FATAL << "TcpConnection::handleError";
}

void TcpConnection::handleClose()
{
    LOG_TRACE << "TcpConnection::handleClose";
    channel_->disableAll();
    loop_->queueInLoop([this]() { closeCallback_(shared_from_this()); });
}

void TcpConnection::handleWrite() {}