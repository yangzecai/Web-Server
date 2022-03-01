#pragma once

#include <cstdint>
#include <functional>

class LogBuffer {
public:
    using ReadCallback = std::function<void(const char*, uint32_t)>;

    LogBuffer(uint32_t cap, uint32_t tailCap);
    ~LogBuffer() { delete[] data_; }

    LogBuffer(const LogBuffer&) = delete;
    LogBuffer& operator=(const LogBuffer&) = delete;

    char* allocWritableSpace_unblock(uint32_t space);
    char* allocWritableSpace(uint32_t space);
    void finishAlloc(uint32_t bytes);

    uint32_t read(uint32_t bytes);
    uint32_t readAll() { return read(getReadableBytes()); }

    uint32_t getReadableBytes() const { return writePos_ - readPos_; }
    uint32_t getWritableBytes() const { return cap_ - getReadableBytes(); }
    void setReadCallback(ReadCallback rcb) { readCallback_ = rcb; }

private:
    char* data_;
    const uint32_t cap_;
    const uint32_t tailCap_;
    uint32_t writePos_;
    uint32_t readPos_;
    ReadCallback readCallback_;

    uint32_t getWriteIndex() const { return writePos_ & (cap_ - 1); }
    uint32_t getReadIndex() const { return readPos_ & (cap_ - 1); }
};