#include "LogBackend.h"
#include "Config.h"

#include <cassert>

namespace log {

LogBackend LogBackend::singleton;
thread_local RingBuffer* LogBackend::t_buffer_ = nullptr;

LogBackend::LogBackend()
    : file_(new LogAppender(LOG_BASE_NAME, LOG_ROLL_SIZE))
    , logThread_()
    , buffers_()
    , mutex_()
    , running_(true)
{
    logThread_ = std::thread(std::bind(&LogBackend::threadFunc, &singleton));
}

LogBackend::~LogBackend()
{
    running_ = false;
    logThread_.join();

    for (auto buffer : buffers_) {
        if (buffer->getReadableBytes()) {
            buffer->readAll();
        }
        delete buffer;
    }

    delete file_;
}

RingBuffer* LogBackend::getThreadBuffer()
{
    if (!t_buffer_) {
        registerThreadBuffer();
    }
    return t_buffer_;
}

void LogBackend::registerThreadBuffer()
{
    using namespace std::placeholders;
    t_buffer_ = new RingBuffer(LOG_BUF_CAP, LOG_BUF_TAIL_CAP);
    t_buffer_->setReadCallback(
        std::bind(&LogAppender::append, singleton.file_, _1, _2));
    std::lock_guard<std::mutex> lock(singleton.mutex_);
    singleton.buffers_.push_back(t_buffer_);
}

void LogBackend::threadFunc()
{
    assert(running_ == true);

    while (running_) {
        uint32_t logBytes = 0;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (uint32_t i = 0; i < buffers_.size(); ++i) {
                if (buffers_[i]->getReadableBytes()) {
                    logBytes += buffers_[i]->readAll();
                }
            }
        }
        if (logBytes) {
            file_->flush();
            continue;
        } else {
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
        // FIXME : 未实时 delete 退出线程的 buffer
    }
}

} // namespace log