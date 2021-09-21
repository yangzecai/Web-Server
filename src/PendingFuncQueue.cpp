#include "PendingFuncQueue.h"
#include "EventLoop.h"
#include "Log.h"

#include <sys/eventfd.h>
#include <unistd.h>

PendingFuncQueue::PendingFuncQueue(EventLoop* loop)
    : loop_(loop)
    , eventfd_(createEventfdOrDie())
    , channel_(eventfd_, loop)
    , funcQueue_()
    , mutex_()
    , callingPendingFunc_(false)
{
    channel_.setReadCallback(std::bind(&PendingFuncQueue::handleRead, this));
    channel_.enableRead();
}

PendingFuncQueue::~PendingFuncQueue()
{
    channel_.close();
    ::close(eventfd_);
}

void PendingFuncQueue::enqueue(const CallbackFunc& cb)
{
    std::lock_guard<std::mutex> lock(mutex_);
    funcQueue_.push_back(cb);
}

int PendingFuncQueue::createEventfdOrDie() const
{
    int eventfd = ::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if (eventfd == -1) {
        LOG_SYSFATAL << "PendingFuncQueue::createEventfdOrDie";
    }
    return eventfd;
}

void PendingFuncQueue::callPendingFunc()
{
    std::vector<CallbackFunc> processingFuncQueue;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        processingFuncQueue.swap(funcQueue_);
    }
    callingPendingFunc_ = true;
    for (auto& func : processingFuncQueue) {
        func();
    }
    callingPendingFunc_ = false;
}

void PendingFuncQueue::handleRead()
{
    LOG_TRACE << "PendingFuncQueue::handleRead";
    uint64_t unused;
    ssize_t n = ::read(eventfd_, &unused, sizeof(unused));
    if (n != sizeof(unused)) {
        LOG_SYSFATAL << "PendingFuncQueue::handleRead reads " << n
                     << " bytes instead of " << sizeof(unused);
    }
}

void PendingFuncQueue::notify()
{
    uint64_t unused = 1;
    ssize_t n = ::write(eventfd_, &unused, sizeof(unused));
    if (n != sizeof(unused)) {
        LOG_FATAL << "PendingFuncQueue::notify writes " << n
                  << " bytes instead of " << sizeof(unused);
    }
}