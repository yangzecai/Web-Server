#include "TcpClient.h"
#include "EventLoop.h"
#include "TcpConnection.h"
#include "Buffer.h"
#include <cstdio>

void onConnection(const TcpConnectionPtr& conn)
{
    std::printf("onConnection()\n");
    conn->send("hi");
    // conn->shutdown();
}

void onClose(const TcpConnectionPtr& conn)
{
    std::printf("onClose()\n");
}

void onMessage(const TcpConnectionPtr& conn, Buffer& recvBuffer)
{
    std::printf("onMessage()\n");
    conn->send(recvBuffer.beginOfReadableBytes(), recvBuffer.getReadableBytes());
    recvBuffer.retrieveAll();
}

int main()
{
    EventLoop loop;
    TcpClient client(&loop, Address::createIPv4Address("127.0.0.1", 9981));
    client.setConnectionCallback(onConnection);
    client.setCloseCallback(onClose);
    client.setMessageCallback(onMessage);
    client.start();
    loop.loop();
}