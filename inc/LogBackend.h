#pragma once

#include "LogAppender.h"
#include "LogBuffer.h"

#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace log {

class LogBackend {
public:
    static LogBuffer* getThreadBuffer();
    static void sync();

private:
    LogBackend();
    ~LogBackend();

    void threadFunc();

    void registerThreadBuffer();
    void deleteThreadBuffer();

    static LogBackend singleton;

    class BufferDestoryer {
    public:
        BufferDestoryer() = default;
        ~BufferDestoryer()
        {
            if (t_buffer_) {
                singleton.deleteThreadBuffer();
            }
        }
        void active() {}
    };

    static thread_local LogBuffer* t_buffer_; // TODO : 测试用智能指针速度
    static thread_local BufferDestoryer bufferDestoryer_;

    LogAppender* file_; // TODO : 测试用智能指针速度
    std::thread logThread_;
    bool running_;
    std::vector<LogBuffer*> buffers_;
    mutable std::mutex bufVecMutex_;
    enum
    {
        SYNC_NONE,
        SYNC_REQUESTED,
        SYNC_HANDLING,
        SYNC_COMPLETED
    } syncStatus_;
    mutable std::mutex condMutex_;
    mutable std::condition_variable syncCompleted_;
    mutable std::condition_variable workAdded_;
};

} // namespace log