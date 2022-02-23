#include "Waker.h"
#include "Log.h"

#include <sys/eventfd.h>
#include <unistd.h>

Waker::Waker(EventLoop* loop)
    : loop_(loop)
    , eventfd_(createEventfdOrDie())
    , channel_(eventfd_, loop_)
{
    channel_.setReadCallback(std::bind(&Waker::handleRead, this));
    channel_.enableRead();
}

Waker::~Waker()
{
    channel_.close();
    ::close(eventfd_);
}

void Waker::wakeup()
{
    uint64_t unused = 1;
    ssize_t n = ::write(eventfd_, &unused, sizeof(unused));
    if (n != sizeof(unused)) {
        LOG_FATAL << "Waker::wakeup writes " << n << " bytes instead of "
                  << sizeof(unused);
    }
}

int Waker::createEventfdOrDie() const
{
    int eventfd = ::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if (eventfd == -1) {
        LOG_SYSFATAL << "PendingFuncQueue::createEventfdOrDie";
    }
    return eventfd;
}

void Waker::handleRead()
{
    LOG_TRACE << "Waker::handleRead";
    uint64_t unused;
    ssize_t n = ::read(eventfd_, &unused, sizeof(unused));
    if (n != sizeof(unused)) {
        LOG_SYSFATAL << "Waker::handleRead reads " << n << " bytes instead of "
                     << sizeof(unused);
    }
}