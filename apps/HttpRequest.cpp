#include "HttpRequest.h"
#include "../inc/Buffer.h"

#include <regex>

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
    case Method::INVALID:
        str += "INVALID";
        break;
    case Method::GET:
        str += "GET";
        break;
    case Method::POST:
        str += "POST";
        break;
    case Method::HEAD:
        str += "HEAD";
        break;
    case Method::PUT:
        str += "PUT";
        break;
    case Method::DELETE:
        str += "DELETE";
        break;
    }
    str += "\n";

    str += "path : " + path + "\n";

    str += "version : ";
    switch (version) {
    case Version::INVALID:
        str += "INVALID";
        break;
    case Version::HTTP1_1:
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
    const char* end = buffer.find("\r\n\r\n") + 2;
    while (buffer.beginOfReadableBytes() < end) {
        if (parseHeader(buffer) == false) {
            return false;
        }
    }
    buffer.retrieve(2);
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
        method = Method::GET;
    } else if (0 == ::strncmp(str, "POST", len)) {
        method = Method::POST;
    } else if (0 == ::strncmp(str, "HEAD", len)) {
        method = Method::HEAD;
    } else if (0 == ::strncmp(str, "PUT", len)) {
        method = Method::PUT;
    } else if (0 == ::strncmp(str, "DELETE", len)) {
        method = Method::DELETE;
    } else {
        method = Method::INVALID;
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
        version = Version::HTTP1_1;
    } else {
        version = INVALID;
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
    headers[std::string(begin, colon)] = std::string(colon + 1, crlf);
    buffer.retrieve(crlf - begin + 2);
    return true;
}