#pragma once

#include <memory>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

class Address {
public:
    Address() {}
    virtual ~Address() {}
    virtual const sockaddr* getSockAddr() const = 0;
    int getFamily() const { return getSockAddr()->sa_family; }
    virtual const socklen_t getSockLen() const = 0;
};

class IPv4Address : public Address {
public:
    IPv4Address() {}
    IPv4Address(const char* ip, uint16_t port);
    IPv4Address(bool loopbackOnly, uint16_t port);  // FIXME : bug
    IPv4Address(const sockaddr* addr, socklen_t socklen);

    const sockaddr* getSockAddr() const override
    {
        return reinterpret_cast<const sockaddr*>(&addr_);
    }
    const socklen_t getSockLen() const override { return sizeof(addr_); }

private:
    sockaddr_in addr_;
};

class IPv6Address : public Address {
public:
    IPv6Address(const char* ip, uint16_t port);
    IPv6Address(bool loopbackOnly, uint16_t port);
    IPv6Address(const sockaddr* addr, socklen_t socklen);

    const sockaddr* getSockAddr() const override
    {
        return reinterpret_cast<const sockaddr*>(&addr_);
    }
    const socklen_t getSockLen() const override { return sizeof(addr_); }

private:
    sockaddr_in6 addr_;
};