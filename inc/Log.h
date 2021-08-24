#pragma once

#include <condition_variable>
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
    if (log::manager.getLevel() <= log::TRACE)                                 \
    log::Logger(__FILE__, __LINE__, log::TRACE).getStream()
#define LOG_DEBUG                                                              \
    if (log::manager.getLevel() <= log::DEBUG)                                 \
    log::Logger(__FILE__, __LINE__, log::DEBUG).getStream()
#define LOG_INFO                                                               \
    if (log::manager.getLevel() <= log::INFO)                                  \
    log::Logger(__FILE__, __LINE__, log::INFO).getStream()
#define LOG_WARN log::Logger(__FILE__, __LINE__, log::WARN).getStream()
#define LOG_ERROR log::Logger(__FILE__, __LINE__, log::ERROR).getStream()
#define LOG_FATAL log::Logger(__FILE__, __LINE__, log::FATAL).getStream()

namespace log {

class Manager;
extern Manager manager;

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
    uint64_t time;
    std::thread::id treadId;
    std::string message;
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

    std::string format(Event::ptr event);

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
    using ptr = std::unique_ptr<Appender>;

    Appender() {}
    virtual ~Appender() {}

    Appender(const Appender&) = delete;
    Appender& operator=(const Appender&) = delete;

    virtual void append(const std::string& fmtLog) = 0;
    virtual void flush() = 0;
};

class StdoutAppender : public Appender {
public:
    StdoutAppender()
        : Appender()
    {
    }
    ~StdoutAppender() {}

    void append(const std::string& fmtLog) override;
    void flush() override {}
};

class Manager {
public:
    Manager();
    ~Manager();

    Manager(const Manager&) = delete;
    Manager& operator=(const Manager&) = delete;

    void setAppender(Appender::ptr apd) { appender_.reset(apd.release()); }
    void setLevel(Level level) { level_ = level; }
    Level getLevel() const { return level_; }

    void start();
    void stop();

    bool isStopped() const { return stop_; }
    bool isInLogThread() const
    {
        std::this_thread::get_id() == logThread_.get_id();
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
};

} // namespace log