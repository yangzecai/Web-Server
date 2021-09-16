#include "Address.h"
#include "Log.h"

#include <cstring>

#include <arpa/inet.h>

IPv4Address::IPv4Address(const char* ip, uint16_t port)
{
    std::memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = ::htons(port);
    if (::inet_pton(AF_INET, ip, &addr_.sin_addr) <= 0) {
        LOG_SYSFATAL << "IPv4Address::IPv4Address";
    }
}

IPv4Address::IPv4Address(bool loopbackOnly, uint16_t port)
{
    std::memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = ::htons(port);
    addr_.sin_addr.s_addr = loopbackOnly ? INADDR_LOOPBACK : INADDR_ANY;
}

IPv4Address::IPv4Address(const sockaddr* addr, socklen_t socklen)
{
    assert(socklen == sizeof(sockaddr_in) && addr->sa_family == AF_INET);
    std::memcpy(&addr_, addr, socklen);
}

IPv6Address::IPv6Address(const char* ip, uint16_t port)
{
    std::memset(&addr_, 0, sizeof(addr_));
    addr_.sin6_family = AF_INET6;
    addr_.sin6_port = ::htons(port);
    if (::inet_pton(AF_INET6, ip, &addr_.sin6_addr) <= 0) {
        LOG_SYSFATAL << "IPv6Address::IPv6Address";
    }
}

IPv6Address::IPv6Address(bool loopbackOnly, uint16_t port)
{
    std::memset(&addr_, 0, sizeof(addr_));
    addr_.sin6_family = AF_INET6;
    addr_.sin6_port = ::htons(port);
    addr_.sin6_addr = loopbackOnly ? in6addr_loopback : in6addr_any;
}

IPv6Address::IPv6Address(const sockaddr* addr, socklen_t socklen)
{
    assert(socklen == sizeof(sockaddr_in6) && addr->sa_family == AF_INET);
    std::memcpy(&addr_, addr, socklen);
}