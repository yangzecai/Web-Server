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

class Manager;

class Event {
public:
    using ptr = std::shared_ptr<Event>;

    Event() {}
    virtual ~Event() {}

    Event(const Event&) = delete;
    Event& operator=(const Event&) = delete;

    virtual void handle(Manager* manager) = 0;
};

class MessageEvent : public Event,
                     public std::enable_shared_from_this<MessageEvent> {
public:
    using ptr = std::shared_ptr<MessageEvent>;

    MessageEvent(const std::string& fileName, int line, Level level);
    ~MessageEvent() {}

    void handle(Manager* manager) override;

    std::string file;
    int line;
    Level level;
    TimePoint time;
    std::thread::id threadId;
    std::string content;
    std::string fmtLog;
};

class Logger {
public:
    Logger(const std::string& fileName, int line, Level level);
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::ostringstream& getStream() { return ss_; }

private:
    static thread_local std::ostringstream ss_;
    MessageEvent::ptr event_;
};

class Formater {
public:
    using ptr = std::unique_ptr<Formater>;

    Formater();
    ~Formater() = default;

    Formater(const Formater&) = delete;
    Formater& operator=(const Formater&) = delete;

    void format(MessageEvent::ptr event);

private:
    MessageEvent::ptr event_;
    std::ostringstream ss_;

    auto time();
    auto file();
    auto line();
    auto level();
    auto threadId();
    auto content();
};

class Appender {
public:
    using ptr = std::shared_ptr<Appender>;

    Appender() {}
    virtual ~Appender() {}

    Appender(const Appender&) = delete;
    Appender& operator=(const Appender&) = delete;

    virtual void append(MessageEvent::ptr event) = 0;
    virtual void flush() = 0;
};

class Manager {
public:
    friend class StopEvent;
    
    static Manager& getInstance()
    {
        static Manager manager;
        return manager;
    }

    Manager(const Manager&) = delete;
    Manager& operator=(const Manager&) = delete;

    void setAppender(Appender::ptr apd) { appender_ = apd; }
    Appender::ptr getAppender(void) { return appender_; }
    void setLevel(Level level) { level_ = level; }
    Level getLevel() const { return level_; }
    Formater::ptr& getFormat() { return formater_; }

    void start();
    void stop();

    bool isStopped() const { return stop_; }
    bool isInLogThread() const
    {
        return std::this_thread::get_id() == logThread_.get_id();
    }
    void sendEvent(Event::ptr event);
    void handleEvent();

private:
    std::thread logThread_;
    Formater::ptr formater_;
    Appender::ptr appender_;
    std::queue<Event::ptr> eventQueue_;
    size_t queueCapacity_;
    mutable std::mutex mutex_;
    mutable std::condition_variable notEmpty_;
    mutable std::condition_variable notFill_;
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

    void append(MessageEvent::ptr event) override;
    void flush() override;
};

class FileAppender : public Appender {
public:
    FileAppender(const std::string& fileName);
    ~FileAppender();

    void append(MessageEvent::ptr event) override;
    void flush() override;

private:
    std::ofstream fs_;
};

class RollingFileAppender : public Appender {
public:
    RollingFileAppender(const std::string& baseFileName);
    ~RollingFileAppender();

    void append(MessageEvent::ptr event) override;
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

class SetAppenderEvent : public Event {
public:
    using ptr = std::shared_ptr<SetAppenderEvent>;

    SetAppenderEvent(Appender::ptr appender)
        : Event()
        , appender_(appender)
    {
    }
    ~SetAppenderEvent() {}

    void handle(Manager* manager) override { manager->setAppender(appender_); }

private:
    Appender::ptr appender_;
};

void setLevel(Level level);
void setAppender(Appender::ptr appender);

} // namespace log