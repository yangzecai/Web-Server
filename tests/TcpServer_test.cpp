#include "Address.h"
#include "EventLoop.h"
#include "Log.h"
#include "TcpConnection.h"
#include "TcpServer.h"

#include <cstdio>

#include <unistd.h>

void onConnection(const TcpConnectionPtr& conn)
{
    printf("onConnection(): new connection from %s\n",
           conn->getClientAddr().getAddressStr().c_str());
}

void onMessage(const TcpConnectionPtr& conn, Buffer& recvBuffer)
{
    printf("onMessage(): received %zd bytes from connection [%s]\n",
           recvBuffer.getReadableBytes(),
           conn->getClientAddr().getAddressStr().c_str());
    recvBuffer.retrieveAll();
}

int main()
{
    log::setLevel(log::TRACE);

    printf("main(): pid = %d\n", ::getpid());

    Address listenAddr(Address::createIPv4Address(9981));
    EventLoop loop;

    TcpServer server(&loop, listenAddr);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.start();

    loop.loop();
}