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

void Socket::swap(Socket& rhs) noexcept
{
    using std::swap;
    swap(fd_, rhs.fd_);
}

Socket::Socket(Socket&& rhs)
    : fd_(rhs.fd_)
{
    rhs.fd_ = -1;
}

Socket& Socket::operator=(Socket&& rhs)
{
    swap(rhs);
    return *this;
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
    int arg = ::fcntl(fd_, F_GETFL);
    if (arg == -1) {
        LOG_SYSFATAL << "Socket::setNonblock";
    }
    arg = on ? arg | O_NONBLOCK : arg & ~O_NONBLOCK;
    if (::fcntl(fd_, F_SETFL, arg) == -1) {
        LOG_SYSFATAL << "Socket::setNonblock";
    }
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
    *clientAddr = Address(&addr, addrlen);
    return clientFd;
}

void Socket::connectOrDie(const Address& addr)
{
    if (::connect(fd_, addr.getSockAddr(), addr.getSockLen()) != 0) {
        LOG_SYSFATAL << "Socket::connectOrDie";
    }
}

void Socket::shutdown(int how)
{
    if (::shutdown(fd_, how) != 0) {
        LOG_SYSERROR << "Socket::shutdown";
    }
}

void Socket::setsockoptOrDie(int level, int optname, bool on)
{
    int optval = on ? 1 : 0;
    if (0 != ::setsockopt(fd_, level, optname, &optval, sizeof(optval))) {
        LOG_SYSFATAL << "Socket::setsockoptOrDie";
    }
}

int Socket::getSocketError()
{
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);
    if (::getsockopt(fd_, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        return errno;
    } else {
        return optval;
    }
}