#include "Connector.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Log.h"
#include "TcpConnection.h"

#include <unistd.h>

Connector::Connector(EventLoop* loop, const Address& serverAddr)
    : loop_(loop)
    , serverAddr_(serverAddr)
    , channel_(nullptr)
    , newConnCallback_()
    , socket_(nullptr)
{
}

Connector::~Connector() {}

void Connector::connectInLoop()
{
    loop_->assertInOwningThread();
    assert(socket_ == nullptr);
    socket_.reset(new Socket(AF_INET, SOCK_STREAM));
    socket_->setNonblock(true);
    if (socket_->connect(serverAddr_) < 0) {
        int saveErrno = errno;
        LOG_ERROR << "Connector::connectInLoop connect errno = " << saveErrno;
        if (saveErrno == EINPROGRESS) {
            assert(channel_ == nullptr);
            channel_.reset(new Channel(socket_->getFd(), loop_));
            channel_->setWriteCallback(
                std::bind(&Connector::handleWrite, this));
            channel_->enableWrite();
        } else {
            LOG_FATAL << "Connector::connectInLoop unexpected connect error = "
                      << saveErrno;
        }
    } else {
        LOG_FATAL << "Connector::connectInLoop unexpected connect success";
    }
}

void Connector::connect()
{
    loop_->runInLoop(std::bind(&Connector::connectInLoop, this));
}

void Connector::handleWrite()
{
    loop_->assertInOwningThread();

    channel_->disableAll();
    loop_->removeChannel(channel_.get());
    loop_->queueInLoop([this]() { channel_.reset(nullptr); });

    int err = socket_->getSocketError();
    LOG_TRACE << "Connector::handleWrite err = " << err;
    int fd = socket_->release();
    socket_.reset(nullptr);

    if (err) {
        LOG_ERROR << "Connector::handleWrite " << log::strerror(err);
        // FIXME: retry
        return;
    }

    if (newConnCallback_) {
        newConnCallback_(fd);
    } else {
        ::close(fd);
    }
}
