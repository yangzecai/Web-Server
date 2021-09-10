#pragma once

#include "LogAppender.h"
#include "RingBuffer.h"

#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace log {

class LogBackend {
public:
    static RingBuffer* getThreadBuffer();

private:
    LogBackend();
    ~LogBackend();

    void threadFunc();

    static LogBackend singleton;
    static thread_local RingBuffer* t_buffer_; // FIXME : using shared ptr

    static void registerThreadBuffer();

    LogAppender* file_; // FIXME : using smart ptr
    std::thread logThread_;
    std::vector<RingBuffer*> buffers_;
    mutable std::mutex mutex_;
    bool running_;
};

} // namespace log