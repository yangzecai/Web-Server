#include "Socket.h"
#include "Address.h"
#include "Log.h"

#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

Socket::Socket(int sockfd)
    : fd_(sockfd)
{
}

Socket::Socket(int domain, int type)
    : fd_(socketOrDie(domain, type))
{
}

Socket::~Socket()
{
    if (isValid()) {
        close();
    }
}

void Socket::close()
{
    if (::close(fd_) < 0) {
        LOG_SYSERROR << "Socket::close";
    }
    fd_ = -1;
}

void Socket::setNonblock(bool on)
{
    int arg = fcntlOrDie(F_GETFL);
    arg = on ? arg | O_NONBLOCK : arg & ~O_NONBLOCK;
    fcntlOrDie(F_SETFL, arg);
}

void Socket::setReuseAddr(bool on)
{
    setsockoptOrDie(SOL_SOCKET, SO_REUSEADDR, on);
}

void Socket::setReusePort(bool on)
{
    setsockoptOrDie(SOL_SOCKET, SO_REUSEPORT, on);
}

void Socket::setTcpNoDelay(bool on)
{
    setsockoptOrDie(IPPROTO_TCP, TCP_NODELAY, on);
}

void Socket::setKeepAlive(bool on)
{
    setsockoptOrDie(SOL_SOCKET, SO_KEEPALIVE, on);
}

int Socket::socketOrDie(int domain, int type)
{
    int sockfd = ::socket(domain, type | SOCK_CLOEXEC, 0);
    if (sockfd < 0) {
        LOG_SYSFATAL << "Socket::createNonblockSockFdOrDie";
    }
    return sockfd;
}

void Socket::bindOrDie(const Address& addr)
{
    int ret = ::bind(fd_, addr.getSockAddr(), addr.getSockLen());
    if (ret < 0) {
        LOG_SYSFATAL << "Socket::bindOrDie";
    }
}

void Socket::listenOrDie()
{
    int ret = ::listen(fd_, SOMAXCONN);
    if (ret < 0) {
        LOG_SYSFATAL << "Socket::listenOrDie";
    }
}

int Socket::acceptOrDie(Address* clientAddr)
{
    sockaddr addr;
    socklen_t addrlen = sizeof(addr);
    int clientFd = ::accept(fd_, &addr, &addrlen);
    if (clientFd < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            LOG_SYSERROR << "Socket::acceptOrDie";
            return -1;
        } else {
            LOG_SYSFATAL << "Socket::acceptOrDie";
        }
    }
    if (addr.sa_family == AF_INET) {
        *clientAddr = IPv4Address(&addr, addrlen);
    } else if (addr.sa_family == AF_INET6) {
        *clientAddr = IPv6Address(&addr, addrlen);
    }
    return clientFd;
}

void Socket::connectOrDie(const Address& addr)
{
    if (::connect(fd_, addr.getSockAddr(), addr.getSockLen()) != 0) {
        LOG_SYSFATAL << "Socket::connectOrDie";
    }
}

void Socket::shutdownOrDie(int how)
{
    if (::shutdown(fd_, how) != 0) {
        LOG_SYSFATAL << "Socket::shutdownOrDie";
    }
}

int Socket::fcntlOrDie(int cmd, ...)
{

    va_list va;
    va_start(va, cmd);
    int ret = ::fcntl(fd_, cmd, va);
    va_end(va);
    if (ret == -1) {
        LOG_SYSFATAL << "Socket::fcntlOrDie";
    }
    return ret;
}

void Socket::setsockoptOrDie(int level, int optname, bool on)
{
    int optval = on ? 1 : 0;
    if (0 != ::setsockopt(fd_, level, optname, &optval, sizeof(optval))) {
        LOG_SYSFATAL << "Socket::setsockoptOrDie";
    }
}