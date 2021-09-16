#pragma once

#include "LogStream.h"

#define LOG_TRACE                                                              \
    if (log::getLevel() <= log::TRACE)                                         \
    log::LogLine(__FILE__, __LINE__, log::TRACE).getStream()
#define LOG_DEBUG                                                              \
    if (log::getLevel() <= log::DEBUG)                                         \
    log::LogLine(__FILE__, __LINE__, log::DEBUG).getStream()
#define LOG_INFO                                                               \
    if (log::getLevel() <= log::INFO)                                          \
    log::LogLine(__FILE__, __LINE__, log::INFO).getStream()
#define LOG_WARN log::LogLine(__FILE__, __LINE__, log::WARN).getStream()
#define LOG_ERROR log::LogLine(__FILE__, __LINE__, log::ERROR).getStream()
#define LOG_FATAL log::LogLine(__FILE__, __LINE__, log::FATAL).getStream()
#define LOG_SYSERROR                                                           \
    log::LogLine(__FILE__, __LINE__, log::ERROR).getStream()                   \
        << log::strerror() << ' '
#define LOG_SYSFATAL                                                           \
    log::LogLine(__FILE__, __LINE__, log::FATAL).getStream()                   \
        << log::strerror() << ' '

class timeval;

namespace log {

enum LogLevel
{
    TRACE = 0,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};
const char* strerror();
LogLevel getLevel();
void setLevel(LogLevel);

class LogLine {
public:
    LogLine(const char* file, int line, LogLevel level);
    ~LogLine();
    LogStream& getStream() { return *stream_; }

private:
    timeval time_;
    LogStream* stream_; // TODO : 测试用智能指针速度
    LogLevel level_;
    int line_;
    const char* file_;

    auto time();
    auto file();
    auto level();
    auto line();
    auto threadId();
    auto error();
};

} // namespace log