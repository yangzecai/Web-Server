#include "HttpServer.h"

HttpServer::HttpServer(EventLoop* loop, const Address& addr)
    : server_(loop, addr)
    , requestCallback_()
{
}

HttpServer::~HttpServer() {}

void HttpServer::start()
{

}

void HttpServer::onConnection(const TcpConnectionPtr& conn)
{

}

void HttpServer::onMessage(const TcpConnectionPtr& conn, Buffer& buffer)
{
    
}