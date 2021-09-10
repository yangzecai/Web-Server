#include "LogAppender.h"

#include <cassert>
#include <ctime>

namespace log {

LogAppender::ptr LogAppender::createLogAppender(const std::string& baseName,
                                                uint32_t rollSize)
{
    return std::make_unique<LogAppender>(baseName, rollSize);
}

LogAppender::LogAppender(const std::string& baseName, uint32_t rollSize)
    : baseName_(baseName)
    , rollSize_(rollSize)
    , fp_(nullptr)
    , buffer_()
    , writtenBytes_(0)
{
    assert(baseName.find('/') == std::string::npos);
    rollFile();
}

LogAppender::~LogAppender()
{
    if (fp_ != nullptr) {
        flush();
        std::fclose(fp_);
    }
}

void LogAppender::append(const char* data, uint32_t bytes)
{
    uint32_t ret = ::fwrite_unlocked(data, sizeof(char), bytes, fp_);
    assert(ret == bytes);
    (void)ret;

    writtenBytes_ += bytes;
    if (writtenBytes_ > rollSize_) {
        rollFile();
    }
}

void LogAppender::flush() { std::fflush(fp_); }

bool LogAppender::rollFile()
{
    std::time_t now;
    std::time(&now);

    if (now != lastRoll_) {
        std::string fileName = getLogFileName(now);
        lastRoll_ = now;
        if (fp_) {
            std::fclose(fp_);
        }
        fp_ = std::fopen(fileName.c_str(), "a");
        assert(fp_);
        ::setbuffer(fp_, buffer_, sizeof(buffer_));
        writtenBytes_ = 0;
        return true;
    }
    return false;
}

std::string LogAppender::getLogFileName(std::time_t now)
{
    std::string fileName;
    fileName.reserve(baseName_.size() + 64);
    fileName = baseName_;

    char timeBuf[32];
    std::tm tm;
    ::localtime_r(&now, &tm);
    std::strftime(timeBuf, sizeof(timeBuf), ".%Y%m%d-%H%M%S", &tm);

    fileName += timeBuf;
    fileName += ".log";

    return fileName;
}

} // namespace log