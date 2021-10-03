#include "Connector.h"
#include "EventLoop.h"
#include "Address.h"
#include "Log.h"

void newConnCallback(int fd)
{
    const char buf[] = "haha";
    printf("connected\n");
    int len = ::send(fd, buf, sizeof(buf), 0);
    if (len == -1) {
        printf("send error\n");
    }
}

int main()
{
    log::setLevel(log::TRACE);
    EventLoop loop;
    Connector connector(&loop, Address::createIPv4Address("127.0.0.1", 9981));
    connector.setNewConnCallback(newConnCallback);
    connector.connect();
    loop.loop();
}