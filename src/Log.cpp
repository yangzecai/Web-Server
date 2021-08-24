#include "Log.h"

#include <chrono>
#include <iostream>

namespace log {

Manager manager;
thread_local std::stringstream Logger::ss_;

Logger::Logger(const std::string& fileName, int line, Level level)
    : event_(std::make_shared<Event>(Event{
          fileName, line, level, std::chrono::high_resolution_clock::now(),
          std::this_thread::get_id(), std::string()}))
{
}

Logger::~Logger()
{
    event_->message = ss_.str();
    ss_.str("");
    manager.receiveEvent(event_);
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
    uint64_t time =
        duration_cast<microseconds>(event_->time.time_since_epoch()).count();
    time_t seconds = time / kMicroPerSec;
    uint64_t micros = time % kMicroPerSec;
    std::strftime(timeStr, 32, "%Y-%m-%d %T.", std::localtime(&seconds));
    std::snprintf(timeStr + 20, 7, "%06lu", micros);
    return timeStr;
}
auto Formater::file()
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
inline auto Formater::threadId() { return event_->treadId; }
inline auto Formater::message() { return event_->message; }

std::string Formater::format(Event::ptr event)
{
    event_ = event;
    ss_.str("");

    ss_ << time() << ' ' << level() << ' ' << threadId() << ' ' << file() << ':'
        << line() << " " << message() << "\n";
    return ss_.str();
}

Manager::Manager()
    : logThread_()
    , formater_(std::make_unique<Formater>())
    , appender_(std::make_unique<StdoutAppender>())
    , eventQueue_()
    , level_(INFO)
    , stop_(false)
    , nextFlushTime_(std::chrono::high_resolution_clock::now() +
                     std::chrono::seconds(3))
{
}

Manager::~Manager()
{
    if (!stop_) {
        stop();
    }
}

void Manager::start()
{
    assert(!isStopped());
    logThread_ = std::thread([this]() {
        while (!this->isStopped()) {
            this->handleEvent();
        }
    });
}

void Manager::stop()
{
    assert(!isInLogThread());
    receiveEvent(nullptr);
    logThread_.join();
    appender_->flush();
}

void Manager::receiveEvent(Event::ptr event)
{
    assert(!isInLogThread());
    std::lock_guard<std::mutex> lock(mutex_);
    eventQueue_.push(std::move(event));
    notEmpty_.notify_one();
}

void Manager::handleEvent()
{
    assert(isInLogThread());
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (eventQueue_.empty()) {
            auto status = notEmpty_.wait_until(lock, nextFlushTime_);
            if (status == std::cv_status::timeout) {
                appender_->flush();
                nextFlushTime_ += std::chrono::seconds(3);
                return;
            }
        }
    }
    auto event = std::move(eventQueue_.front());
    eventQueue_.pop();

    if (event == nullptr) { // 正常退出
        stop_ = true;
        return;
    }

    appender_->append(formater_->format(event));
    if (event->level == FATAL) {
        appender_->flush();
        abort();
    }
    if (event->time > nextFlushTime_) {
        appender_->flush();
        nextFlushTime_ += std::chrono::seconds(3);
    }
}

inline void StdoutAppender::append(const std::string& log) { std::cout << log; }
inline void StdoutAppender::flush() { std::cout << std::flush; }

FileAppender::FileAppender()
    : Appender()
    , fs_("/home/yangzecai/Projects/Web-Server/tmp")
{
    std::cout << fs_.is_open() << std::endl;
}

void FileAppender::append(const std::string& log) { fs_ << log; }
inline void FileAppender::flush() { fs_ << std::flush; }

} // namespace log