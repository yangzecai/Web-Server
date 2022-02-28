#pragma once

#include <map>

class Buffer;

struct HttpRequest {
public:
    HttpRequest(const HttpRequest&) = delete;
    HttpRequest& operator=(const HttpRequest&) = delete;

    enum Method
    {
        INVALID,
        GET,
        POST,
        HEAD,
        PUT,
        DELETE
    };

    enum Version
    {
        INVALID,
        HTTP1_1
    };

    Method method;
    Version version;
    std::string path;
    std::map<std::string, std::string> headers;
    std::string body;

    bool parse(Buffer& buffer);
    std::string toString() const;

private:
    bool parseRequestLine(Buffer& buffer);
    bool parseHeaders(Buffer& buffer);
    bool parseBody(Buffer& buffer);
    bool parseMethod(Buffer& buffer);
    bool parsePath(Buffer& buffer);
    bool parseVersion(Buffer& buffer);
    bool parseHeader(Buffer& Buffer);
};