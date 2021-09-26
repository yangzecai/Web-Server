#include "TcpServer.h"
#include "Acceptor.h"
#include "Address.h"
#include "EventLoop.h"
#include "Log.h"
#include "TcpConnection.h"

#include <memory>

TcpServer::TcpServer(EventLoop* loop, const Address& addr)
    : loop_(loop)
    , acceptor_(std::make_unique<Acceptor>(loop, addr))
    , connectionCallback_()
    , messageCallback_()
    , closeCallback_()
    , connections_()
{
    using namespace std::placeholders;
    acceptor_->setNewConnCallback(
        std::bind(&TcpServer::addConnection, this, _1, _2));
}

TcpServer::~TcpServer()
{
    assert(connections_.empty());
}

void TcpServer::start()
{
    assert(connectionCallback_ != nullptr);
    assert(messageCallback_ != nullptr);
    assert(closeCallback_ != nullptr);
    LOG_TRACE << "TcpServer::start";
    loop_->assertInOwningThread();
    acceptor_->listen();
}

void TcpServer::addConnection(int connfd, const Address& clientAddr)
{
    loop_->assertInOwningThread();
    LOG_INFO << "TcpServer::addConnection new connect from "
             << clientAddr.getAddressStr() << " connfd = " << connfd;
    using namespace std::placeholders;
    auto conn = std::make_shared<TcpConnection>(loop_, connfd, clientAddr);
    connections_.insert(conn);
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setCloseCallback([this](const TcpConnectionPtr& conn) {
        closeCallback_(conn);
        removeConnection(conn);
    });
    conn->establish();
}

void TcpServer::removeConnection(const TcpConnectionPtr& connPtr)
{
    LOG_INFO << "TcpServer::removeConnection remove connect from "
             << connPtr->getClientAddr().getAddressStr();
    loop_->assertInOwningThread();
    connections_.erase(connPtr);
}