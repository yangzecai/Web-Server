#include "Log.h"

#include <cstdio>
#include <cstring>
#include <thread>

#include <sys/syscall.h>
#include <sys/time.h>
#include <unistd.h>

namespace log {

thread_local char t_errBuf[64];
const char* strerror(int saveErrno)
{
    assert(saveErrno);
    return strerror_r(saveErrno, t_errBuf, sizeof(t_errBuf));
}

LogLevel g_logLevel = LogLevel::INFO;
LogLevel getLevel() { return g_logLevel; }
void setLevel(LogLevel level) { g_logLevel = level; }

// helper class for known string length at compile time
class T {
public:
    T(const char* str, unsigned len)
        : str_(str)
        , len_(len)
    {
        assert(strlen(str) == len_);
    }

    const char* str_;
    const unsigned len_;
};
inline LogStream& operator<<(LogStream& s, T v)
{
    s.append(v.str_, v.len_);
    return s;
}

auto LogLine::time()
{
    static thread_local time_t t_lastTime = 0;
    static thread_local char t_timeStr[32];
    if (t_lastTime != time_.tv_sec) {
        std::strftime(t_timeStr, sizeof(t_timeStr), "%Y-%m-%d %T.",
                      std::localtime(&time_.tv_sec));
        t_lastTime = time_.tv_sec;
    }
    std::snprintf(t_timeStr + 20, 7, "%06lu", time_.tv_usec);
    return T(t_timeStr, 26);
}

auto LogLine::file()
{
    file_ = std::strrchr(file_, '/') + 1;
    return file_;
}

auto LogLine::level()
{
    static const char* LevelName[6] = {"TRACE", "DEBUG", "INFO ",
                                       "WARN ", "ERROR", "FATAL"};
    return T(LevelName[level_], 5);
}

auto LogLine::line() { return line_; }

auto LogLine::threadId()
{
    static thread_local char t_threadIdStr[16];
    static thread_local int t_tidStrLen = 0;
    if (__builtin_expect(t_tidStrLen == 0, 0)) {
        t_tidStrLen = std::snprintf(t_threadIdStr, sizeof(t_threadIdStr), "%d",
                                    static_cast<int>(::syscall(SYS_gettid)));
    }
    return T(t_threadIdStr, t_tidStrLen);
}

LogLine::LogLine(const char* file, int line, LogLevel level)
    : time_()
    , stream_(new LogStream())
    , level_(level)
    , line_(line)
    , file_(file)
{
    ::gettimeofday(&time_, nullptr);

    getStream() << time() << ' ' << threadId() << ' ' << this->level() << ' ';
}

LogLine::~LogLine()
{
    getStream() << " - " << file() << ':' << line() << '\n';
    delete stream_;
    stream_ = nullptr;

    if (level_ == FATAL) {
        LogBackend::sync();
        abort();
    }
}

} // namespace log