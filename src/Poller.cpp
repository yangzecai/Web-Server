#include "Poller.h"

#include <sys/epoll.h>

Poller::Poller()
    : epollfd_(::epoll_create(1))
{

}