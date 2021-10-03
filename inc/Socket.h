#pragma once

#include <sys/socket.h>
#include <sys/types.h>

class Address;

class Socket {
public:
    Socket(int sockfd);
    Socket(int domain, int type);
    ~Socket();

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    Socket(Socket&& rhs);
    Socket& operator=(Socket&& rhs);

    int getFd() const { return fd_; }

    void bind(const Address& addr) { bindOrDie(addr); }
    void listen() { listenOrDie(); }
    int accept(Address* clientAddr) { return acceptOrDie(clientAddr); }
    int connect(const Address& addr);
    void close();
    void shutdownWrite() { shutdown(SHUT_WR); }

    void setNonblock(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setTcpNoDelay(bool on);
    void setKeepAlive(bool on);

    bool isValid() { return fd_ != -1; }

    int getSocketError();

    int release();

private:
    void swap(Socket& rhs) noexcept;

    int socketOrDie(int domain, int type);
    void bindOrDie(const Address& addr);
    void listenOrDie();
    int acceptOrDie(Address* clientAddr);
    void closeOrDie();
    void shutdown(int how);
    void setsockoptOrDie(int level, int optname, bool on);

    int fd_;
};
