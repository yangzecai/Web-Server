#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

#include <assert.h>

template <typename T>
class BlockingQueue {
public:
    using ptr = std::shared_ptr<BlockingQueue>;

    BlockingQueue()
        : mutex_()
        , notEmpty_()
        , queue_()
    {
    }

    BlockingQueue(const BlockingQueue&) = delete;
    BlockingQueue& operator=(const BlockingQueue&) = delete;

    void put(T&& t)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(t));
        notEmpty_.notify_one();
    }

    void put(const T& t)
    {
        T tmp(t);
        BlockingQueue<T>::put(std::move(tmp));
    }

    T take()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (queue_.empty()) {
            notEmpty_.wait(lock);
        }
        T ret(std::move(queue_.front()));
        queue_.pop();
        return ret;
    }

    

private:
    mutable std::mutex mutex_;
    std::condition_variable notEmpty_;
    std::queue<T> queue_;
};