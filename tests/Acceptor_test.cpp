#include "Acceptor.h"
#include "EventLoop.h"
#include "Address.h"
#include "unistd.h"
#include "Log.h"

void newConnFunc(Socket&& socket, const Address& addr)
{
    ::write(socket.getFd(), "How are you?\n", 13);
    socket.close();
}

int main()
{
    log::setLevel(log::TRACE);
    EventLoop loop;
    Acceptor acceptor(&loop, Address::createIPv4Address(9981));
    acceptor.setNewConnCallback(newConnFunc);
    acceptor.listen();

    loop.loop();
}