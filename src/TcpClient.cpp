#include "TcpClient.h"
#include "Connector.h"
#include "EventLoop.h"
#include "Log.h"
#include "TcpConnection.h"

#include <functional>

TcpClient::TcpClient(EventLoop* loop, const Address& serverAddr)
    : loop_(loop)
    , serverAddr_(serverAddr)
    , connector_(std::make_unique<Connector>(loop, serverAddr))
    , connection_(nullptr)
    , connectionCallback_()
    , messageCallback_()
    , closeCallback_()
    , writeCompleteCallback_()
{
    using namespace std::placeholders;
    connector_->setNewConnCallback(
        std::bind(&TcpClient::addConnection, this, _1));
}

TcpClient::~TcpClient()
{
}

void TcpClient::start()
{
    assert(connectionCallback_);
    assert(messageCallback_);
    assert(closeCallback_);
    loop_->assertInOwningThread();
    LOG_INFO << "TcpClient::start";
    connector_->connect();
}

void TcpClient::addConnection(int sockfd)
{
    loop_->assertInOwningThread();
    LOG_INFO << "TcpClient::addConnection new connection";
    connection_ = std::make_shared<TcpConnection>(loop_, sockfd, serverAddr_);
    connection_->setConnectionCallback(connectionCallback_);
    connection_->setMessageCallback(messageCallback_);
    connection_->setCloseCallback([this](const TcpConnectionPtr& conn) {
        closeCallback_(conn);
        removeConnection();
    });
    connection_->setWriteCompleteCallback(writeCompleteCallback_);
    connection_->establish();
}

void TcpClient::removeConnection()
{
    LOG_INFO << "TcpClient::addConnection connection was removed";
    connection_ = nullptr;
}