#include "EventLoop.h"
#include "Channel.h"
#include "Log.h"

#include <sys/timerfd.h>
#include <strings.h>
#include <unistd.h>

EventLoop* g_loop;

void timeout()
{
    printf("Timeout!\n");
    g_loop->quit();
}

int main()
{
    log::setLevel(log::TRACE);

    EventLoop loop;
    g_loop = &loop;

    int timefd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    Channel channel(timefd, &loop);
    channel.setReadCallback(timeout);
    channel.enableRead();
    
    struct itimerspec howlong;
    bzero(&howlong, sizeof(howlong));
    howlong.it_value.tv_sec = 5;
    ::timerfd_settime(timefd, 0, &howlong, NULL);

    loop.loop();
    ::close(timefd);
}