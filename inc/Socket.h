#pragma once

#include "Address.h"

#include <sys/socket.h>
#include <sys/types.h>

class Socket {
public:
    Socket(int sockfd);
    Socket(int domain = AF_INET, int type = SOCK_STREAM);
    ~Socket();

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    int getFd() const { return fd_; }

    void bind(const Address& addr) { bindOrDie(addr); }
    void listen() { listenOrDie(); }
    int accept(Address& clientAddr) { return acceptOrDie(clientAddr); }
    void connect(const Address& addr) { connectOrDie(addr); }
    void close();
    void shutdownWrite() { shutdownOrDie(SHUT_WR); }

    void setNonblock(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setTcpNoDelay(bool on);
    void setKeepAlive(bool on);

    bool isValid() { return fd_ != -1; }

private:
    int socketOrDie(int domain, int type);
    void bindOrDie(const Address& addr);
    void listenOrDie();
    int acceptOrDie(Address& clientAddr);
    void connectOrDie(const Address& addr);
    void closeOrDie();
    void shutdownOrDie(int how);

    int fcntlOrDie(int cmd, ...);
    void setsockoptOrDie(int level, int optname, bool on);

    int fd_;
};
