#include "AddressImpl.h"
#include "Log.h"

#include <cstring>
#include <iostream>

#include <arpa/inet.h>

IPv4AddressImpl::IPv4AddressImpl(const std::string& ip, uint16_t port)
{
    std::memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = ::htons(port);
    if (::inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr) <= 0) {
        LOG_SYSFATAL << "IPv4AddressImpl::IPv4AddressImpl";
    }
}

IPv4AddressImpl::IPv4AddressImpl(uint16_t port, bool loopbackOnly)
{
    std::memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = ::htons(port);
    addr_.sin_addr.s_addr =
        ::htonl(loopbackOnly ? INADDR_LOOPBACK : INADDR_ANY);
}

IPv4AddressImpl::IPv4AddressImpl(const sockaddr* addr, socklen_t socklen)
{
    assert(socklen == sizeof(sockaddr_in) && addr->sa_family == AF_INET);
    std::memcpy(&addr_, addr, socklen);
}

std::string IPv4AddressImpl::getAddressStr() const
{
    char buf[32];
    inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    size_t hostLen = std::strlen(buf);
    std::snprintf(buf + hostLen, sizeof(buf) - hostLen, ":%u",
                  ::ntohs(addr_.sin_port));
    return buf;
}

IPv6AddressImpl::IPv6AddressImpl(const std::string& ip, uint16_t port)
{
    std::memset(&addr_, 0, sizeof(addr_));
    addr_.sin6_family = AF_INET6;
    addr_.sin6_port = ::htons(port);
    if (::inet_pton(AF_INET6, ip.c_str(), &addr_.sin6_addr) <= 0) {
        LOG_SYSFATAL << "IPv6AddressImpl::IPv6AddressImpl";
    }
}

// FIXME: in6addr_loopback, in6addr_any 未进行网络字节序转化。
IPv6AddressImpl::IPv6AddressImpl(uint16_t port, bool loopbackOnly)
{
    std::memset(&addr_, 0, sizeof(addr_));
    addr_.sin6_family = AF_INET6;
    addr_.sin6_port = ::htons(port);
    addr_.sin6_addr = loopbackOnly ? in6addr_loopback : in6addr_any;
}

IPv6AddressImpl::IPv6AddressImpl(const sockaddr* addr, socklen_t socklen)
{
    assert(socklen == sizeof(sockaddr_in6) && addr->sa_family == AF_INET);
    std::memcpy(&addr_, addr, socklen);
}

// FIXME: 未实现
std::string IPv6AddressImpl::getAddressStr() const
{
    return std::string();
}