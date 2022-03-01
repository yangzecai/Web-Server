#include "HttpServer.h"
#include "Log.h"
#include "TcpConnection.h"

std::string kNotFoundHtml =
    "<html>"
    "<head><title>404 Not Found</title></head>"
    "<body>"
    "<h1>Not Found</h1>"
    "<p>The requested URL was not found on this server.</p>"
    "</body>"
    "</html>";

HttpServer::HttpServer(EventLoop* loop, const Address& addr)
    : server_(loop, addr)
    , requestCallback_()
{
    using std::placeholders::_1, std::placeholders::_2;
    server_.setMessageCallback(std::bind(&HttpServer::onMessage, this, _1, _2));
}

HttpServer::~HttpServer() {}

void HttpServer::start()
{
    server_.start();
}

void HttpServer::onMessage(const TcpConnectionPtr& conn, Buffer& buffer)
{
    HttpRequest request;
    bool ok = request.parse(buffer);
    buffer.retrieveAll();
    if (requestCallback_ == nullptr || false == ok) {
        HttpResponse response;
        response.version = HttpResponse::Version::V_HTTP1_1;
        response.statusCode = HttpResponse::StatusCode::S_NOTFOUND;
        response.statusMessage = std::string("Not Found");
        response.headers["Connection"] = "close";
        response.headers["Content-Type"] = "text/html";
        response.body = kNotFoundHtml;
        conn->send(response.serialize());
        conn->shutdown();
        return;
    }

    HttpResponse response;
    requestCallback_(request, response);
    conn->send(response.serialize());
    
    if (response.headers.find("Connection") != response.headers.end() &&
        response.headers["Connection"] == "close") {
        conn->shutdown();
    }
}