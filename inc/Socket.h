#pragma once

class Socket {
public:
    Socket();
    ~Socket();
private:
    int createSocketFd();

    int fd_;
};
