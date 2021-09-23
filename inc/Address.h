#pragma once

#include <memory>
#include <string>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

class AddressImpl;

class Address {
public:
    Address();
    Address(int family, const std::string& ip, uint16_t port);
    Address(int family, uint16_t port, bool loopbackOnly = false);
    Address(const sockaddr* addr, socklen_t socklen);
    ~Address() {}

    static Address createIPv4Address(const std::string& ip, uint16_t port);
    static Address createIPv4Address(uint16_t port, bool loopbackOnly = true);
    static Address createIPv6Address(const std::string& ip, uint16_t port);
    static Address createIPv6Address(uint16_t port, bool loopbackOnly = true);

    const sockaddr* getSockAddr() const;
    int getFamily() const { return getSockAddr()->sa_family; }
    const socklen_t getSockLen() const;
    
    std::string getAddressStr() const;

private: 
    std::shared_ptr<AddressImpl> impl;
};
