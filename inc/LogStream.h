#pragma once

#include "Config.h"
#include "LogBackend.h"
#include "LogBuffer.h"

#include <cassert>
#include <cstring>
#include <string>

namespace log {

class StreamBuffer {
public:
    StreamBuffer()
        : buffer_(LogBackend::getThreadBuffer())
        , cap_(LOG_STREAM_BUF_CAP)
        , data_(buffer_->allocWritableSpace(cap_))
        , cur_(data_)
    {
    }

    ~StreamBuffer() { buffer_->finishAlloc(length()); }

    StreamBuffer(const StreamBuffer&) = delete;
    StreamBuffer& operator=(const StreamBuffer&) = delete;

    void append(const char* buf, uint32_t len)
    {
        if (avail() > len) {
            memcpy(cur_, buf, len);
            cur_ += len;
        }
    }

    const char* data() const { return data_; }
    uint32_t length() const { return cur_ - data_; }

    // write to data_ directly
    char* current() { return cur_; }
    uint32_t avail() const { return end() - cur_; }
    void add(uint32_t len) { cur_ += len; }

    void reset() { cur_ = data_; }

private:
    const char* end() const { return data_ + cap_; }

    LogBuffer* buffer_;
    uint32_t cap_;
    char* data_;
    char* cur_;
};

class LogStream {
    using self = LogStream;

public:
    using Buffer = StreamBuffer;

    LogStream() = default;
    ~LogStream() = default;

    LogStream(const LogStream&) = delete;
    LogStream& operator=(const LogStream&) = delete;

    self& operator<<(bool v)
    {
        buffer_.append(v ? "1" : "0", 1);
        return *this;
    }
    self& operator<<(short);
    self& operator<<(unsigned short);
    self& operator<<(int);
    self& operator<<(unsigned int);
    self& operator<<(long);
    self& operator<<(unsigned long);
    self& operator<<(long long);
    self& operator<<(unsigned long long);
    self& operator<<(const void*);
    self& operator<<(float v)
    {
        *this << static_cast<double>(v);
        return *this;
    }
    self& operator<<(double);
    self& operator<<(char v)
    {
        buffer_.append(&v, 1);
        return *this;
    }
    self& operator<<(const char* str)
    {
        if (str) {
            buffer_.append(str, strlen(str));
        } else {
            buffer_.append("(null)", 6);
        }
        return *this;
    }
    self& operator<<(const unsigned char* str)
    {
        return operator<<(reinterpret_cast<const char*>(str));
    }
    self& operator<<(const std::string& v)
    {
        buffer_.append(v.c_str(), v.size());
        return *this;
    }

    void append(const char* data, int len) { buffer_.append(data, len); }
    const Buffer& buffer() const { return buffer_; }
    void resetBuffer() { buffer_.reset(); }

private:
    template <typename T>
    void formatInteger(T);

    Buffer buffer_;

    static const int kMaxNumericSize = 32;
};

} // namespace log