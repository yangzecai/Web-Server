#include "Connector.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Log.h"
#include "TcpConnection.h"

#include <unistd.h>

const std::chrono::milliseconds Connector::kInitRetryTime_ =
    std::chrono::milliseconds(500);
const std::chrono::milliseconds Connector::kMaxRetryTime_ =
    std::chrono::seconds(30);

Connector::Connector(EventLoop* loop, const Address& serverAddr)
    : loop_(loop)
    , serverAddr_(serverAddr)
    , channel_(nullptr)
    , newConnCallback_()
    , socket_(nullptr)
    , retryDelayTime_(kInitRetryTime_)
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

void Connector::removeChannel()
{
    assert(channel_ != nullptr);
    channel_->disableAll();
    loop_->removeChannel(channel_.get());
    loop_->queueInLoop([this]() { channel_.reset(nullptr); });
}

int Connector::releaseSocket()
{
    assert(socket_ != nullptr);
    int fd = socket_->release();
    socket_.reset(nullptr);
    return fd;
}

void Connector::retry()
{
    LOG_INFO << "Connector::retry retry connecting to "
             << serverAddr_.getAddressStr() << " in " << retryDelayTime_.count()
             << "ms";
    loop_->runAfter(retryDelayTime_,
                    std::bind(&Connector::connectInLoop, this));
    retryDelayTime_ = std::min(retryDelayTime_ * 2, kMaxRetryTime_);
}

void Connector::handleWrite()
{
    // FIXME: 判断自连接
    loop_->assertInOwningThread();

    removeChannel();

    int err = socket_->getSocketError();
    if (err) {
        LOG_ERROR << "Connector::handleWrite " << log::strerror(err);
        socket_.reset(nullptr);
        retry();
        return;
    }

    int fd = releaseSocket();
    if (newConnCallback_) {
        newConnCallback_(fd);
    } else {
        ::close(fd);
    }
}
