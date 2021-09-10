#pragma once

#include <memory>

#include <cstdio>
#include <ctime>
#include <string>

class LogAppender {
public:
    using ptr = std::unique_ptr<LogAppender>;

    static LogAppender::ptr createLogAppender(const std::string& baseName,
                                              uint32_t rollSize);

    LogAppender(const std::string& baseName, uint32_t rollSize);
    ~LogAppender();

    void append(const char* data, uint32_t bytes);
    void flush();

private:
    bool rollFile();
    std::string getLogFileName(std::time_t now);

    const std::string baseName_;
    uint32_t rollSize_;
    std::time_t lastRoll_;
    FILE* fp_;
    char buffer_[64 * 1024];
    uint32_t writtenBytes_;
};