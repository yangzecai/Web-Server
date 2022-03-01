#include "LogBackend.h"
#include "Config.h"

#include <algorithm>
#include <cassert>

namespace log {

LogBackend LogBackend::singleton;
thread_local LogBuffer* LogBackend::t_buffer_ = nullptr;
thread_local LogBackend::BufferDestoryer LogBackend::bufferDestoryer_;

LogBackend::LogBackend()
    : file_(new LogAppender(LOG_BASE_NAME, LOG_ROLL_SIZE))
    , logThread_()
    , running_(true)
    , buffers_()
    , bufVecMutex_()
    , syncStatus_(SYNC_NONE)
    , condMutex_()
    , syncCompleted_()
    , workAdded_()
{
    logThread_ = std::thread(std::bind(&LogBackend::threadFunc, this));
}

LogBackend::~LogBackend()
{
    sync();
    {
        std::lock_guard<std::mutex> lock(condMutex_);
        running_ = false;
        workAdded_.notify_all();
    }
    assert(buffers_.empty());
    logThread_.join();

    delete file_;
    file_ = nullptr;
}

void LogBackend::sync()
{
    std::unique_lock<std::mutex> lock(singleton.condMutex_);
    singleton.syncStatus_ = SYNC_REQUESTED;
    singleton.workAdded_.notify_all();
    singleton.syncCompleted_.wait(lock);
}

LogBuffer* LogBackend::getThreadBuffer()
{
    if (!t_buffer_) {
        singleton.registerThreadBuffer();
    }
    return t_buffer_;
}

void LogBackend::registerThreadBuffer()
{
    using namespace std::placeholders;
    t_buffer_ = new LogBuffer(LOG_BUF_CAP, LOG_BUF_TAIL_CAP);
    t_buffer_->setReadCallback(
        std::bind(&LogAppender::append, this->file_, _1, _2));
    std::lock_guard<std::mutex> lock(bufVecMutex_);
    buffers_.push_back(t_buffer_);
    bufferDestoryer_.active();
}

void LogBackend::deleteThreadBuffer()
{
    sync();
    {
        std::lock_guard<std::mutex> lock(bufVecMutex_);
        buffers_.erase(std::find(buffers_.begin(), buffers_.end(), t_buffer_));
    }
    delete t_buffer_;
    t_buffer_ = nullptr;
}

void LogBackend::threadFunc()
{
    assert(running_ == true);

    while (running_) {
        uint32_t logBytes = 0;
        {
            std::lock_guard<std::mutex> lock(bufVecMutex_);
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
            std::unique_lock<std::mutex> lock(condMutex_);
            if (syncStatus_ == SYNC_REQUESTED) {
                syncStatus_ = SYNC_HANDLING;
                continue;
            }
            if (syncStatus_ == SYNC_HANDLING) {
                syncStatus_ = SYNC_COMPLETED;
            }
            if (syncStatus_ == SYNC_COMPLETED) {
                syncStatus_ = SYNC_NONE;
                syncCompleted_.notify_all();
            }
            if (syncStatus_ == SYNC_NONE) {
                workAdded_.wait_for(lock, LOG_REST_INTERVAL);
            }
        }
    }
}

} // namespace log