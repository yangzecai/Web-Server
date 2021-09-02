#include "Log.h"

#include <chrono>
#include <iostream>

namespace log {

thread_local std::ostringstream Logger::ss_;
thread_local SPSCQueue threadBuffer_;

void setLevel(Level level) { Manager::getInstance().setLevel(level); }
void setAppender(Appender::ptr appender)
{
    Manager::getInstance().sendEvent(
        std::make_shared<SetAppenderEvent>(appender));
}

MessageEvent::MessageEvent(const std::string& file, int line, Level level)
    : Event()
    , file(file)
    , line(line)
    , level(level)
    , time(std::chrono::high_resolution_clock::now())
    , threadId(std::this_thread::get_id())
    , content()
    , fmtLog()
{
}

void MessageEvent::handle(Manager* manager)
{
    manager->getFormat()->format(shared_from_this());
    manager->getAppender()->append(shared_from_this());
    if (level == FATAL) {
        manager->getAppender()->flush();
        abort();
    }
}

Logger::Logger(const std::string& fileName, int line, Level level)
    : event_(std::make_shared<MessageEvent>(fileName, line, level))
{
}

Logger::~Logger()
{
    event_->content = ss_.str();
    ss_.str("");
    Manager::getInstance().sendEvent(std::move(event_));
}

Formater::Formater()
    : event_(nullptr)
    , ss_()
{
}

auto Formater::time()
{
    using namespace std::chrono;
    static const int kMicroPerSec = 1000000;
    static char timeStr[32];
    static time_t lastSeconds = 0;
    uint64_t time =
        duration_cast<microseconds>(event_->time.time_since_epoch()).count();
    time_t seconds = time / kMicroPerSec;
    uint64_t micros = time % kMicroPerSec;
    if (seconds != lastSeconds) {
        std::strftime(timeStr, 32, "%Y-%m-%d %T.", std::localtime(&seconds));
        lastSeconds = seconds;
    }
    std::snprintf(timeStr + 20, 7, "%06lu", micros);
    return timeStr;
}
inline auto Formater::file()
{
    return event_->file.substr(event_->file.rfind('/') + 1);
}
inline auto Formater::line() { return event_->line; }
inline auto Formater::level()
{
    static const char* levelStr[] = {"TRACE", "DEBUG", "INFO ",
                                     "WARN ", "ERROR", "FATAL"};
    return levelStr[event_->level];
}
inline auto Formater::threadId() { return event_->threadId; }
inline auto Formater::content() { return event_->content; }

void Formater::format(std::shared_ptr<MessageEvent> event)
{
    event_ = event;
    ss_.str("");

    ss_ << time() << ' ' << level() << ' ' << threadId() << ' ' << file() << ':'
        << line() << " " << content() << "\n";
    event->fmtLog = std::move(ss_.str());
}

class StopEvent : public Event {
public:
    StopEvent()
        : Event()
    {
    }
    ~StopEvent() {}
    void handle(Manager* manager) { manager->stop_ = true; }
};

Manager::Manager()
    : logThread_()
    , formater_(std::make_unique<Formater>())
    , appender_(std::make_shared<StdoutAppender>())
    , eventQueue_()
    , queueCapacity_(1000000)
    , mutex_()
    , notEmpty_()
    , notFill_()
    , level_(INFO)
    , stop_(false)
    , nextFlushTime_(std::chrono::high_resolution_clock::now() +
                     std::chrono::seconds(3))
{
    logThread_ = std::thread([this]() {
        while (!this->isStopped()) {
            this->handleEvent();
        }
    });
}

Manager::~Manager()
{
    sendEvent(std::make_shared<StopEvent>());
    logThread_.join();
    appender_->flush();
}

void Manager::sendEvent(Event::ptr event)
{
    assert(!isInLogThread());
    std::unique_lock<std::mutex> lock(mutex_);
    while (eventQueue_.size() >= queueCapacity_) {
        notFill_.wait(lock);
    }
    eventQueue_.push(std::move(event));
    notEmpty_.notify_one();
}

void Manager::handleEvent()
{
    Event::ptr event;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (eventQueue_.empty()) {
            auto status = notEmpty_.wait_until(lock, nextFlushTime_);
            if (status == std::cv_status::timeout) {
                appender_->flush();
                nextFlushTime_ += std::chrono::seconds(3);
            }
        }
        event = std::move(eventQueue_.front());
        eventQueue_.pop();
        notFill_.notify_one();
    }

    event->handle(this);
}

inline void StdoutAppender::append(MessageEvent::ptr event)
{
    assert(!event->fmtLog.empty());
    std::cout << event->fmtLog;
}

inline void StdoutAppender::flush() {}

FileAppender::FileAppender(const std::string& fileName)
    : Appender()
    , fs_(fileName)
{
}

FileAppender::~FileAppender() { fs_.close(); }

void FileAppender::append(MessageEvent::ptr event) { fs_ << event->fmtLog; }

void FileAppender::flush() { fs_ << std::flush; }

RollingFileAppender::RollingFileAppender(const std::string& baseFileName)
    : Appender()
    , fs_()
    , baseFileName_(baseFileName)
    , writtenBytes_(0)
    , nextRollFileTime_(getTomorrowZeroTime())
{
    fs_.open(getLogFileName());
}

RollingFileAppender::~RollingFileAppender() { fs_.close(); }

void RollingFileAppender::append(MessageEvent::ptr event)
{
    assert(!event->fmtLog.empty());
    fs_ << event->fmtLog;
    writtenBytes_ += event->fmtLog.size();
    if (event->time > nextRollFileTime_) {
        rollFile();
        nextRollFileTime_ += std::chrono::hours(24);
    }
    if (writtenBytes_ >= kMaxWrittenBytes_) {
        rollFile();
    }
}

inline void RollingFileAppender::flush() { fs_ << std::flush; }

TimePoint RollingFileAppender::getTomorrowZeroTime()
{
    using namespace std::chrono;
    static const int kSecPerDay = 24 * 60 * 60;
    std::time_t now = std::time(NULL);
    now = now / kSecPerDay * kSecPerDay + kSecPerDay;
    return high_resolution_clock::from_time_t(now);
}

std::string RollingFileAppender::getLogFileName() const
{
    using namespace std::chrono;
    static const int kMicroPerSec = 1000000;
    uint64_t time = duration_cast<microseconds>(
                        high_resolution_clock::now().time_since_epoch())
                        .count();
    time_t seconds = time / kMicroPerSec;
    char timeStr[32];
    std::strftime(timeStr, 32, "-%Y%m%d-%H%M%S", std::localtime(&seconds));

    std::string fileName;
    fileName.reserve(64);
    fileName += baseFileName_;
    fileName += timeStr;
    fileName += ".log";
    return fileName;
}

void RollingFileAppender::rollFile()
{
    fs_.close();
    fs_.open(getLogFileName());
    writtenBytes_ = 0;
}

} // namespace log