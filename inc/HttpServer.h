#pragma once

#include "TcpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

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
    void onMessage(const TcpConnectionPtr& conn, Buffer& buffer);

    TcpServer server_;
    RequestCallback requestCallback_;
};