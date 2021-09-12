#include "Socket.h"

#include <sys/types.h>
#include <sys/socket.h>

Socket::Socket()
    : fd_()
{

}

int Socket::createSocketFd()
{
    int ret = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (ret < 0) {
        
    }
}