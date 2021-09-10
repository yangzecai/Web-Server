#include "RingBuffer.h"

#include <algorithm>
#include <thread>

#include <cassert>
#include <cstring>

RingBuffer::RingBuffer(uint32_t cap, uint32_t tailCap)
    : data_(new char[cap + tailCap])
    , cap_(cap)
    , tailCap_(tailCap)
    , writePos_(0)
    , readPos_(0)
    , readCallback_()
{
    assert(cap && !(cap & (cap - 1)));
}

uint32_t RingBuffer::read(uint32_t bytes)
{
    bytes = std::min(bytes, getReadableBytes());
    uint32_t bytesToEnd = cap_ - getReadIndex();

    if (bytes < bytesToEnd) {
        readCallback_(data_ + getReadIndex(), bytes);
    } else {
        readCallback_(data_ + getReadIndex(), bytesToEnd);
        readCallback_(data_, bytes - bytesToEnd);
    }
    readPos_ += bytes;
    return bytes;
}

char* RingBuffer::allocWritableSpace_unblock(uint32_t space)
{
    assert(space <= tailCap_);
    return space > getWritableBytes() ? nullptr : data_ + getWriteIndex();
}

char* RingBuffer::allocWritableSpace(uint32_t space)
{
    char* ret;
    while ((ret = allocWritableSpace_unblock(space)) == nullptr) {
        std::this_thread::yield();
    }
    return ret;
}

void RingBuffer::finishAlloc(uint32_t bytes)
{
    assert(bytes <= tailCap_);
    if (getWriteIndex() + bytes > cap_) {
        std::memcpy(data_, data_ + cap_, getWriteIndex() + bytes - cap_);
    }
    writePos_ += bytes;
}
