#pragma once

#include <string>

#include <netinet/in.h>

class AddressImpl {
public:
    AddressImpl() {}
    virtual ~AddressImpl() {}
    virtual const sockaddr* getSockAddr() const = 0;
    int getFamily() const { return getSockAddr()->sa_family; }
    virtual const socklen_t getSockLen() const = 0;
    virtual std::string getAddressStr() const = 0;
};

class IPv4AddressImpl : public AddressImpl {
public:
    IPv4AddressImpl() {}
    IPv4AddressImpl(const std::string& ip, uint16_t port);
    IPv4AddressImpl(uint16_t port, bool loopbackOnly);
    IPv4AddressImpl(const sockaddr* addr, socklen_t socklen);

    const sockaddr* getSockAddr() const override
    {
        return reinterpret_cast<const sockaddr*>(&addr_);
    }
    const socklen_t getSockLen() const override { return sizeof(addr_); }
    std::string getAddressStr() const;

private:
    sockaddr_in addr_;
};

class IPv6AddressImpl : public AddressImpl {
public:
    IPv6AddressImpl(const std::string& ip, uint16_t port);
    IPv6AddressImpl(uint16_t port, bool loopbackOnly);
    IPv6AddressImpl(const sockaddr* addr, socklen_t socklen);

    const sockaddr* getSockAddr() const override
    {
        return reinterpret_cast<const sockaddr*>(&addr_);
    }
    const socklen_t getSockLen() const override { return sizeof(addr_); }
    std::string getAddressStr() const;  // FIXME: 未实现

private:
    sockaddr_in6 addr_;
};