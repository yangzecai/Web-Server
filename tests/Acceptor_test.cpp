#include "Acceptor.h"
#include "EventLoop.h"
#include "Address.h"
#include "unistd.h"

void newConnFunc(Socket&& socket, const Address& addr)
{
    ::write(socket.getFd(), "How are you?\n", 13);
    socket.close();
}

int main()
{
    EventLoop loop;   
    Acceptor acceptor(&loop, IPv4Address("127.0.0.1", 9981));
    acceptor.setNewConnCallback(newConnFunc);
    acceptor.listen();

    loop.loop();
}