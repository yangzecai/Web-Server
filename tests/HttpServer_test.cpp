#include "Address.h"
#include "EventLoop.h"
#include "HttpRequest.h"
#include "HttpServer.h"
#include "Log.h"

#include <iostream>

void onRequest(const HttpRequest& request, HttpResponse& response)
{
    bool longConn = true;
    if (request.headers.find("Connection") != request.headers.end() &&
        request.headers.at("Connection") == "close") {
        longConn = false;
    }

    if (request.path == "/") {
        response.version = HttpResponse::Version::V_HTTP1_1;
        response.statusCode = HttpResponse::StatusCode::S_OK;
        response.statusMessage = "OK";
        response.headers["Content-Type"] = "text/html";
        if (longConn) {
            response.headers["Connection"] = "keep-alive";
            response.headers["Keep-Alive"] = "timeout=30";
        } else {
            response.headers["Connection"] = "close";
        }
        response.body = "<html>"
                        "<head><title>This is title</title></head>"
                        "<body><h1>Hello World!</h1></body>"
                        "</html>";
    } else {
        response.version = HttpResponse::Version::V_HTTP1_1;
        response.statusCode = HttpResponse::StatusCode::S_OK;
        response.statusMessage = "OK";
        response.headers["Content-Type"] = "text/html";
        response.headers["Connection"] = "close";
        response.body = "<html>"
                        "<head><title>404 Not Found</title></head>"
                        "<body><h1>Not Found</h1</body>"
                        "</html>";
    }
}

int main()
{
    log::setLevel(log::TRACE);
    EventLoop loop;
    HttpServer server(&loop, Address::createIPv4Address(9981));
    server.setThreadNum(2);
    server.setRequestCallback(onRequest);
    server.start();
    loop.loop();
}
