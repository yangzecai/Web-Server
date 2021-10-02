#include "TcpServer.h"
#include "Acceptor.h"
#include "Address.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "Log.h"
#include "TcpConnection.h"

#include <memory>

TcpServer::TcpServer(EventLoop* loop, const Address& addr)
    : loop_(loop)
    , acceptor_(std::make_unique<Acceptor>(loop, addr))
    , connectionCallback_()
    , messageCallback_()
    , closeCallback_()
    , writeCompleteCallback_()
    , connections_()
    , threadPool_(std::make_unique<EventLoopThreadPool>(loop))
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
    threadPool_->start();
    acceptor_->listen();
}

void TcpServer::addConnection(int connfd, const Address& clientAddr)
{
    loop_->assertInOwningThread();
    LOG_INFO << "TcpServer::addConnection new connect from "
             << clientAddr.getAddressStr() << " connfd = " << connfd;
    EventLoop* connHandleLoop = threadPool_->getNextLoop();
    TcpConnectionPtr conn =
        std::make_shared<TcpConnection>(connHandleLoop, connfd, clientAddr);

    connections_.insert(conn);
    conn->setConnectionCallback(connectionCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setCloseCallback([this](const TcpConnectionPtr& conn) {
        closeCallback_(conn);
        removeConnection(conn);
    });
    connHandleLoop->queueInLoop(
        std::bind(&TcpConnection::establish, std::move(conn)));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& connPtr)
{
    loop_->assertInOwningThread();
    LOG_INFO << "TcpServer::removeConnection remove connect from "
             << connPtr->getClientAddr().getAddressStr();
    connections_.erase(connPtr);
}

void TcpServer::removeConnection(const TcpConnectionPtr& connPtr)
{
    loop_->runInLoop(
        std::bind(&TcpServer::removeConnectionInLoop, this, connPtr));
}

void TcpServer::setThreadNum(int num)
{
    threadPool_->setThreadNum(num);
}