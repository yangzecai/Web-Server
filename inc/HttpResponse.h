#pragma once

#include <string>
#include <map>

struct HttpResponse {
    enum Version
    {
        V_INVALID,
        V_HTTP1_1
    };

    enum StatusCode
    {
        S_INVALID,
        S_OK = 200,
        S_BADREQUEST = 400,
        S_NOTFOUND = 404
    };

    HttpResponse();
    ~HttpResponse();

    HttpResponse(const HttpResponse&) = delete;
    HttpResponse& operator=(const HttpResponse&) = delete;

    Version version;
    StatusCode statusCode;
    std::string statusMessage;
    std::map<std::string, std::string> headers;
    std::string body;

    std::string serialize() const;
};
