#include "Acceptor.h"
#include "Address.h"
#include "EventLoop.h"

#include <unistd.h>

Acceptor::Acceptor(EventLoop* loop, const Address& addr)
    : loop_(loop)
    , socket_(addr.getFamily(), SOCK_STREAM)
    , channel_(socket_.getFd(), loop)
{
    socket_.setNonblock(true);
    socket_.setReuseAddr(true);
    socket_.bind(addr);
    channel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
    channel_.close();
    socket_.close();
}

void Acceptor::listen()
{
    loop_->assertInOwningThread();
    socket_.listen();
    channel_.enableRead();
}

void Acceptor::handleRead()
{
    loop_->assertInOwningThread();
    Address addr;
    int connfd = -1;
    while ((connfd = socket_.accept(&addr)) != -1) {
        if (newConnCallback_) {
            newConnCallback_(connfd, addr);
        } else {
            ::close(connfd);
        }
    }
}
