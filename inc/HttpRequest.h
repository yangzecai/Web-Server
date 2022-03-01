#pragma once

#include <map>

class Buffer;

struct HttpRequest {
public:
    enum Method
    {
        M_INVALID,
        M_GET,
        M_POST,
        M_HEAD,
        M_PUT,
        M_DELETE
    };

    enum Version
    {
        V_INVALID,
        V_HTTP1_1
    };

    HttpRequest();
    ~HttpRequest();

    HttpRequest(const HttpRequest&) = delete;
    HttpRequest& operator=(const HttpRequest&) = delete;

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