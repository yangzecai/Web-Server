#include "Address.h"
#include "EventLoop.h"
#include "Log.h"
#include "TcpConnection.h"
#include "TcpServer.h"

#include <cstdio>
#include <chrono>

#include <unistd.h>

EventLoop* g_loop;

std::string message1;
std::string message2;

void onConnection(const TcpConnectionPtr& conn)
{
    printf("onConnection(): new connection from %s\n",
           conn->getClientAddr().getAddressStr().c_str());
    conn->send(message1);
    conn->send(message2);
    conn->shutdown();
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
    recvBuffer.retrieveAll();
}

void onWriteComplete(const TcpConnectionPtr& conn)
{
    printf("onWriteComplete() write completement\n");
}

int main(int argc, char* argv[])
{
    log::setLevel(log::TRACE);

    printf("main(): pid = %d\n", ::getpid());

    int len1 = 100;
    int len2 = 200;

    if (argc > 2) {
        len1 = atoi(argv[1]);
        len2 = atoi(argv[2]);
    }

    message1.resize(len1);
    message2.resize(len2);
    std::fill(message1.begin(), message2.end(), 'A');
    std::fill(message2.begin(), message2.end(), 'B');

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