#pragma once

#include <algorithm>
#include <cassert>
#include <string>
#include <vector>

class Buffer {
public:
    Buffer();
    ~Buffer() {}

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    size_t getReadableBytes() const { return writePos_ - readPos_; }
    size_t getWritableBytes() const { return buffer_.size() - writePos_; }
    size_t getPrependableBytes() const { return readPos_; }

    const char* beginOfReadableBytes() const { return begin() + readPos_; }
    char* beginOfWritableBytes() { return begin() + writePos_; }
    const char* beginOfWritableBytes() const { return begin() + writePos_; }

    void retrieve(size_t len);
    void retrieveAll();

    void append(const std::string& str) { append(str.data(), str.length()); }
    void append(const char* data, size_t len);
    void append(const void* data, size_t len);
    void prepend(const void* data, size_t len);

    void allocWritableBytes(size_t len);
    void finishAlloc(size_t len) { writePos_ += len; }

    void shrink(size_t reserve);

    ssize_t writeFromFd(int fd);

private:
    void swap(Buffer& rhs);

    char* begin() { return buffer_.data(); }
    const char* begin() const { return buffer_.data(); }

    void makeSpace(size_t len);

    static const size_t kCheapPrepend_ = 8;
    static const size_t kInitialSize_ = 1024;

    std::vector<char> buffer_;
    size_t readPos_;
    size_t writePos_;
};

inline Buffer::Buffer()
    : buffer_(kCheapPrepend_ + kInitialSize_)
    , readPos_(kCheapPrepend_)
    , writePos_(kCheapPrepend_)
{
    assert(getReadableBytes() == 0);
    assert(getWritableBytes() == kInitialSize_);
    assert(getPrependableBytes() == kCheapPrepend_);
}

inline void Buffer::retrieve(size_t len)
{
    assert(len <= getReadableBytes());
    if (len < getReadableBytes()) {
        readPos_ += len;
    } else {
        retrieveAll();
    }
}

inline void Buffer::retrieveAll()
{
    readPos_ = kCheapPrepend_;
    writePos_ = kCheapPrepend_;
}

inline void Buffer::append(const char* data, size_t len)
{
    allocWritableBytes(len);
    std::copy(data, data + len, beginOfWritableBytes());
    finishAlloc(len);
}

inline void Buffer::append(const void* data, size_t len)
{
    append(static_cast<const char*>(data), len);
}

inline void Buffer::prepend(const void* data, size_t len)
{
    assert(len <= getPrependableBytes());
    readPos_ -= len;
    const char* d = static_cast<const char*>(data);
    std::copy(d, d + len, begin() + readPos_);
}

inline void Buffer::allocWritableBytes(size_t len)
{
    if (getWritableBytes() < len) {
        makeSpace(len);
    }
    assert(getWritableBytes() >= len);
}

inline void Buffer::swap(Buffer& rhs)
{
    buffer_.swap(rhs.buffer_);
    std::swap(readPos_, rhs.readPos_);
    std::swap(writePos_, rhs.writePos_);
}

inline void Buffer::shrink(size_t reserve)
{
    std::vector<char> buf(kCheapPrepend_ + getReadableBytes() + reserve);
    std::copy(beginOfReadableBytes(),
              beginOfReadableBytes() + getReadableBytes(),
              buf.begin() + kCheapPrepend_);
    buf.swap(buffer_);
}

inline void Buffer::makeSpace(size_t len)
{
    if (getWritableBytes() + getPrependableBytes() < len + kCheapPrepend_) {
        buffer_.resize(writePos_ + len);
    } else {
        // move readable data to the front, make space inside buffer
        assert(kCheapPrepend_ < readPos_);
        size_t readable = getReadableBytes();
        std::copy(begin() + readPos_, begin() + writePos_,
                  begin() + kCheapPrepend_);
        readPos_ = kCheapPrepend_;
        writePos_ = readPos_ + readable;
        assert(readable == getReadableBytes());
    }
}