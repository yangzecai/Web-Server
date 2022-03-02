#pragma once

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "TcpServer.h"

#include <list>
#include <memory>

class HttpRequest;
class HttpResponse;

class HttpServer {
public:
    using RequestCallback =
        std::function<void(const HttpRequest&, HttpResponse&)>;

    HttpServer(EventLoop* loop, const Address& addr);
    ~HttpServer();

    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;

    void setRequestCallback(const RequestCallback& cb)
    {
        requestCallback_ = cb;
    }
    void setThreadNum(int num) { server_.setThreadNum(num); }

    void start();

private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer& buffer);
    void onClose(const TcpConnectionPtr& conn);

    TcpServer server_;
    RequestCallback requestCallback_;
    size_t timeout_;

    class TimingWheel {
    public:
        class Entry {
        public:
            Entry(const TcpConnectionPtr& conn);
            ~Entry();

        private:
            std::weak_ptr<TcpConnection> conn_;
        };

        using Bucket = std::list<std::shared_ptr<Entry>>;

        TimingWheel(EventLoop* loop, size_t timeout);
        ~TimingWheel();
        TimingWheel(const TimingWheel&) = delete;
        TimingWheel& operator=(const TimingWheel&) = delete;

        void enqueue(const std::weak_ptr<Entry>& entryWeakPtr);

    private:
        void tick();

        EventLoop* loop_;
        std::vector<Bucket> wheel_;
        size_t tail_;
        size_t timeout_;
        static const size_t kBucketNum_ = 32;
    };

    HttpServer::TimingWheel* getTimingWheel(EventLoop* loop) const;
};