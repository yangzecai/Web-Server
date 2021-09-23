#include "Address.h"
#include "AddressImpl.h"

#include <cassert>

Address::Address()
    : impl(nullptr)
{
}

Address::Address(int family, const std::string& ip, uint16_t port)
{
    if (family == AF_INET) {
        impl = std::make_shared<IPv4AddressImpl>(ip, port);
    } else if (family == AF_INET6) {
        impl = std::make_shared<IPv6AddressImpl>(ip, port);
    } else {
        assert(false);
    }
}

Address::Address(int family, uint16_t port, bool loopbackOnly)
{
    if (family == AF_INET) {
        impl = std::make_shared<IPv4AddressImpl>(port, loopbackOnly);
    } else if (family == AF_INET6) {
        impl = std::make_shared<IPv6AddressImpl>(port, loopbackOnly);
    } else {
        assert(false);
    }
}

Address::Address(const sockaddr* addr, socklen_t socklen)
{
    if (socklen == sizeof(sockaddr_in)) {
        impl = std::make_shared<IPv4AddressImpl>(addr, socklen);
    } else if (socklen == sizeof(sockaddr_in6)) {
        impl = std::make_shared<IPv6AddressImpl>(addr, socklen);
    } else {
        assert(false);
    }
}

Address Address::createIPv4Address(const std::string& ip, uint16_t port)
{
    return Address(AF_INET, ip, port);
}
Address Address::createIPv4Address(uint16_t port, bool loopbackOnly)
{
    return Address(AF_INET, port, loopbackOnly);
}
Address Address::createIPv6Address(const std::string& ip, uint16_t port)
{
    return Address(AF_INET6, ip, port);
}
Address Address::createIPv6Address(uint16_t port, bool loopbackOnly)
{
    return Address(AF_INET6, port, loopbackOnly);
}

const sockaddr* Address::getSockAddr() const
{
    return impl->getSockAddr();
}

const socklen_t Address::getSockLen() const
{
    return impl->getSockLen();
}

std::string Address::getAddressStr() const
{
    return impl->getAddressStr();
}