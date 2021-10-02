#include "TcpConnection.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Log.h"
#include "Socket.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

TcpConnection::TcpConnection(EventLoop* loop, int connfd, const Address& addr)
    : loop_(loop)
    , connSocket_(std::make_unique<Socket>(connfd))
    , clientAddr_(addr)
    , channel_(std::make_unique<Channel>(connfd, loop))
    , connectionCallback_()
    , messageCallback_()
    , closeCallback_()
    , writeCompleteCallback_()
    , recvBuffer_()
    , sendBuffer_()
    , disconnecting_(false)
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

void TcpConnection::sendInLoop(const char* str, size_t len)
{
    loop_->assertInOwningThread();
    // FIXME: 在 sendBuffer 没字节的时候直接发送。
    sendBuffer_.append(str, len);
    if (!channel_->isWriting()) {
        channel_->enableWrite();
    }
}

void TcpConnection::sendInLoop(const std::string& str)
{
    sendInLoop(str.data(), str.size());
}

void TcpConnection::send(const char* str, size_t len)
{
    if (!disconnecting_) {
        if (loop_->isInOwningThread()) {
            sendInLoop(str, len);
        } else {
            loop_->queueInLoop(
                std::bind((void (TcpConnection::*)(const std::string&)) &
                              TcpConnection::sendInLoop,
                          this, std::string(str, len)));
        }
    }
}

void TcpConnection::send(std::string&& str)
{
    if (!disconnecting_) {
        if (loop_->isInOwningThread()) {
            sendInLoop(str);
        } else {
            loop_->queueInLoop(
                std::bind((void (TcpConnection::*)(const std::string&)) &
                              TcpConnection::sendInLoop,
                          this, std::move(str)));
        }
    }
}

void TcpConnection::send(const std::string& str)
{
    std::string tmpStr = str;
    send(std::move(tmpStr));
}

void TcpConnection::shutdownInLoop()
{
    loop_->assertInOwningThread();
    if (!channel_->isWriting()) {
        LOG_TRACE << "TcpConnection::shutdownInLoop";
        connSocket_->shutdownWrite();
    }
}

void TcpConnection::shutdown()
{
    disconnecting_ = true;
    loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
}

void TcpConnection::establish()
{
    loop_->assertInOwningThread();
    connectionCallback_(shared_from_this());
    channel_->enableRead();
}

void TcpConnection::handleRead()
{
    ssize_t len = recvBuffer_.writeFromFd(connSocket_->getFd());
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
    LOG_ERROR << "TcpConnection::handleError "
              << log::strerror(connSocket_->getSocketError());
}

void TcpConnection::handleClose()
{
    LOG_TRACE << "TcpConnection::handleClose";
    channel_->disableAll();
    loop_->queueInLoop([this]() { closeCallback_(shared_from_this()); });
}

void TcpConnection::handleWrite()
{
    if (channel_->isWriting()) {
        ssize_t len =
            ::write(connSocket_->getFd(), sendBuffer_.beginOfReadableBytes(),
                    sendBuffer_.getReadableBytes());
        LOG_TRACE << "TcpConnection::handleWrite write " << len << " bytes";
        if (len >= 0) {
            sendBuffer_.retrieve(len);
        } else {
            LOG_SYSERROR << "TcpConnection::handleWrite";
        }
        if (sendBuffer_.getReadableBytes() == 0) {
            channel_->disableWrite();
            if (writeCompleteCallback_) {
                writeCompleteCallback_(shared_from_this());
            }
            if (disconnecting_) {
                shutdownInLoop();
            }
        } else {
            LOG_TRACE << "TcpConnection::handleWrite "
                      << sendBuffer_.getReadableBytes()
                      << " bytes that should be sent was remained";
        }
    } else {
        LOG_TRACE
            << "TcpConnection::handleWrite write channel has been disabled";
    }
}