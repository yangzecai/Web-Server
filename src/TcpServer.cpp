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
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, _1));
    conn->establish();
}

void TcpServer::removeConnection(const TcpConnectionPtr& connPtr)
{
    LOG_INFO << "TcpServer::removeConnection remove connect from "
             << connPtr->getClientAddr().getAddressStr();
    loop_->assertInOwningThread();
    connections_.erase(connPtr);
}