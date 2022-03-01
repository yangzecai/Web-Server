#include "HttpRequest.h"
#include "Buffer.h"

HttpRequest::HttpRequest()
    : method(Method::M_INVALID)
    , version(Version::V_INVALID)
    , path()
    , headers()
    , body()
{
}

HttpRequest::~HttpRequest() {}

bool HttpRequest::parse(Buffer& buffer)
{
    return parseRequestLine(buffer) && parseHeaders(buffer) &&
           parseBody(buffer);
}

std::string HttpRequest::toString() const
{
    std::string str = "";

    str += "[request line]\n";

    str += "method : ";
    switch (method) {
    case Method::M_INVALID:
        str += "INVALID";
        break;
    case Method::M_GET:
        str += "GET";
        break;
    case Method::M_POST:
        str += "POST";
        break;
    case Method::M_HEAD:
        str += "HEAD";
        break;
    case Method::M_PUT:
        str += "PUT";
        break;
    case Method::M_DELETE:
        str += "DELETE";
        break;
    }
    str += "\n";

    str += "path : " + path + "\n";

    str += "version : ";
    switch (version) {
    case Version::V_INVALID:
        str += "INVALID";
        break;
    case Version::V_HTTP1_1:
        str += "HTTP/1.1";
        break;
    }
    str += "\n";

    str += "[headers]\n";
    for (const auto& header : headers) {
        str += header.first + " : " + header.second + "\n";
    }

    str += "[body]\n";
    str += body + "\n";

    return str;
}

bool HttpRequest::parseRequestLine(Buffer& buffer)
{
    return parseMethod(buffer) && parsePath(buffer) && parseVersion(buffer);
}

bool HttpRequest::parseHeaders(Buffer& buffer)
{
    while (true) {
        const char* end = buffer.find("\r\n");
        if (end == nullptr) {
            return false;
        } else if (end == buffer.beginOfReadableBytes()) {
            buffer.retrieve(2);
            break;
        } else {
            if (false == parseHeader(buffer)) {
                return false;
            }
        }
    }
    return true;
}

bool HttpRequest::parseBody(Buffer& buffer)
{
    body =
        std::string(buffer.beginOfReadableBytes(), buffer.getReadableBytes());
    return true;
}

bool HttpRequest::parseMethod(Buffer& buffer)
{
    const char* space = buffer.find(" ");
    const char* str = buffer.beginOfReadableBytes();
    if (space == nullptr || space == str) {
        return false;
    }
    size_t len = space - str;
    if (0 == ::strncmp(str, "GET", len)) {
        method = Method::M_GET;
    } else if (0 == ::strncmp(str, "POST", len)) {
        method = Method::M_POST;
    } else if (0 == ::strncmp(str, "HEAD", len)) {
        method = Method::M_HEAD;
    } else if (0 == ::strncmp(str, "PUT", len)) {
        method = Method::M_PUT;
    } else if (0 == ::strncmp(str, "DELETE", len)) {
        method = Method::M_DELETE;
    } else {
        method = Method::M_INVALID;
        return false;
    }
    buffer.retrieve(len + 1);
    return true;
}

bool HttpRequest::parsePath(Buffer& buffer)
{
    const char* space = buffer.find(" ");
    const char* str = buffer.beginOfReadableBytes();
    if (space == nullptr || space == str) {
        return false;
    }
    size_t len = space - str;
    path = std::string(str, len);
    buffer.retrieve(len + 1);
    return true;
}

bool HttpRequest::parseVersion(Buffer& buffer)
{
    const char* crlf = buffer.find("\r\n");
    const char* str = buffer.beginOfReadableBytes();
    if (crlf == nullptr || crlf == str) {
        return false;
    }
    size_t len = crlf - str;
    if (0 == ::strncmp(str, "HTTP/1.1", len)) {
        version = Version::V_HTTP1_1;
    } else {
        version = V_INVALID;
        return false;
    }
    buffer.retrieve(len + 2);
    return true;
}

bool HttpRequest::parseHeader(Buffer& buffer)
{
    const char* crlf = buffer.find("\r\n");
    const char* colon = buffer.find(":");
    const char* begin = buffer.beginOfReadableBytes();
    if (crlf == nullptr || colon == nullptr || begin == crlf ||
        begin == colon || crlf <= colon) {
        return false;
    }
    const char* keyEnd = colon;
    while (*(keyEnd - 1) == ' ') {
        if (--keyEnd == begin) {
            return false;
        }
    }
    const char* valueBegin = colon + 1;
    while (*valueBegin == ' ') {
        if (++valueBegin == crlf) {
            return false;
        }
    }
    headers[std::string(begin, keyEnd)] = std::string(valueBegin, crlf);
    buffer.retrieve(crlf - begin + 2);
    return true;
}