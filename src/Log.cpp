#include "Log.h"

#include <chrono>
#include <iostream>
#include <memory>

namespace log {

Manager manager;
thread_local std::stringstream Logger::ss_;

std::uint64_t getTimestamp()
{
    using namespace std::chrono;
    return duration_cast<microseconds>(
               high_resolution_clock::now().time_since_epoch())
        .count();
}

Logger::Logger(const std::string& fileName, int line, Level level)
    : event_(std::make_shared<Event>(
          Event{fileName, line, level, getTimestamp(),
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
    static const int kMicroPerSec = 1000000;
    static char timeStr[32];
    time_t seconds = event_->time / kMicroPerSec;
    uint64_t micros = event_->time % kMicroPerSec;
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

void StdoutAppender::append(const std::string& fmtLog) { std::cout << fmtLog; }

Manager::Manager()
    : logThread_()
    , formater_(std::make_unique<Formater>())
    , appender_(std::make_unique<StdoutAppender>())
    , eventQueue_()
    , level_(INFO)
    , stop_(false)
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
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (eventQueue_.empty()) {
            notEmpty_.wait(lock);
        }
    }
    auto event = std::move(eventQueue_.front());
    eventQueue_.pop();

    if (event == nullptr) { // 正常退出
        stop_ = true;
        return;
    }

    appender_->append(formater_->format(event));

    if (event->level == FATAL) { // 异常退出
        appender_->flush();
        abort();
    }
}

} // namespace log