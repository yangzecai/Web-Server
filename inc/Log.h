#pragma once

#include <chrono>
#include <condition_variable>
#include <fstream>
#include <memory>
#include <mutex>
#include <ostream>
#include <queue>
#include <sstream>
#include <string>
#include <thread>

#include <assert.h>
#include <ctime>

#define LOG_TRACE                                                              \
    if (log::Manager::getInstance().getLevel() <= log::TRACE)                  \
    log::Logger(__FILE__, __LINE__, log::TRACE).getStream()
#define LOG_DEBUG                                                              \
    if (log::Manager::getInstance().getLevel() <= log::DEBUG)                  \
    log::Logger(__FILE__, __LINE__, log::DEBUG).getStream()
#define LOG_INFO                                                               \
    if (log::Manager::getInstance().getLevel() <= log::INFO)                   \
    log::Logger(__FILE__, __LINE__, log::INFO).getStream()
#define LOG_WARN log::Logger(__FILE__, __LINE__, log::WARN).getStream()
#define LOG_ERROR log::Logger(__FILE__, __LINE__, log::ERROR).getStream()
#define LOG_FATAL log::Logger(__FILE__, __LINE__, log::FATAL).getStream()

namespace log {

using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

enum Level
{
    TRACE = 0,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

struct Event {
    using ptr = std::shared_ptr<Event>;

    std::string file;
    int line;
    Level level;
    TimePoint time;
    std::thread::id treadId;
    std::string message;
    std::string fmtLog;
};

class Logger {
public:
    Logger(const std::string& fileName, int line, Level level);
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::stringstream& getStream() { return ss_; }

private:
    static thread_local std::stringstream ss_;
    Event::ptr event_;
};

class Formater {
public:
    using ptr = std::unique_ptr<Formater>;

    Formater();
    ~Formater() = default;

    Formater(const Formater&) = delete;
    Formater& operator=(const Formater&) = delete;

    void format(Event::ptr event);

private:
    Event::ptr event_;
    std::stringstream ss_;

    auto time();
    auto file();
    auto line();
    auto level();
    auto threadId();
    auto message();
};

class Appender {
public:
    using ptr = std::shared_ptr<Appender>;

    Appender() {}
    virtual ~Appender() {}

    Appender(const Appender&) = delete;
    Appender& operator=(const Appender&) = delete;

    virtual void append(Event::ptr event) = 0;
    virtual void flush() = 0;
};

class Manager {
public:
    static Manager& getInstance()
    {
        static Manager manager;
        return manager;
    }

    Manager(const Manager&) = delete;
    Manager& operator=(const Manager&) = delete;

    void setAppender(Appender::ptr apd) { std::atomic_store<Appender>(&appender_, apd); }
    void setLevel(Level level) { level_ = level; }
    Level getLevel() const { return level_; }

    void start();
    void stop();

    bool isStopped() const { return stop_; }
    bool isInLogThread() const
    {
        return std::this_thread::get_id() == logThread_.get_id();
    }
    void receiveEvent(Event::ptr event);
    void handleEvent();

private:
    std::thread logThread_;
    Formater::ptr formater_;
    Appender::ptr appender_;
    std::queue<Event::ptr> eventQueue_;
    mutable std::mutex mutex_;
    mutable std::condition_variable notEmpty_;
    Level level_;
    bool stop_;
    TimePoint nextFlushTime_;

    Manager();
    ~Manager();
};

class StdoutAppender : public Appender {
public:
    StdoutAppender()
        : Appender()
    {
    }
    ~StdoutAppender() {}

    void append(Event::ptr event) override;
    void flush() override;
};

class FileAppender : public Appender {
public:
    FileAppender(const std::string& fileName);
    ~FileAppender();

    void append(Event::ptr event) override;
    void flush() override;
private:
    std::ofstream fs_;
};

class RollingFileAppender : public Appender {
public:
    RollingFileAppender(const std::string& baseFileName);
    ~RollingFileAppender();

    void append(Event::ptr event) override;
    void flush() override;

private:
    std::ofstream fs_;
    const std::string baseFileName_;
    int writtenBytes_;
    TimePoint nextRollFileTime_;
    static const int kMaxWrittenBytes_ = 10 * 1024 * 1024;

    TimePoint getTomorrowZeroTime();
    std::string getLogFileName() const;
    void rollFile();
};

void setLevel(Level level);
void setAppender(Appender::ptr apd);

} // namespace log