#include "Address.h"
#include "EventLoop.h"
#include "Log.h"
#include "TcpConnection.h"
#include "TcpServer.h"

#include <cstdio>
#include <chrono>

#include <unistd.h>

EventLoop* g_loop;

void onConnection(const TcpConnectionPtr& conn)
{
    printf("onConnection(): new connection from %s\n",
           conn->getClientAddr().getAddressStr().c_str());
    g_loop->runAfter(std::chrono::seconds(10), [](){g_loop->quit();} );
}

void onClose(const TcpConnectionPtr& conn)
{
    printf("onClose() : disconnect connection from %s\n",
           conn->getClientAddr().getAddressStr().c_str());
}

void onMessage(const TcpConnectionPtr& conn, Buffer& recvBuffer)
{
    printf("onMessage(): received %zd bytes from connection [%s]\n",
           recvBuffer.getReadableBytes(),
           conn->getClientAddr().getAddressStr().c_str());
    std::string str(recvBuffer.beginOfReadableBytes(), recvBuffer.getReadableBytes());
    conn->send(str);
    recvBuffer.retrieveAll();
}

void onWriteComplete(const TcpConnectionPtr& conn)
{
    printf("onWriteComplete() write completement\n");
}

int main()
{
    log::setLevel(log::TRACE);

    printf("main(): pid = %d\n", ::getpid());

    Address listenAddr(Address::createIPv4Address(9981));
    EventLoop loop;
    g_loop = &loop;

    TcpServer server(&loop, listenAddr);
    server.setConnectionCallback(onConnection);
    server.setWriteCompleteCallback(onWriteComplete);
    server.setCloseCallback(onClose);
    server.setMessageCallback(onMessage);
    server.start();

    loop.loop();
}