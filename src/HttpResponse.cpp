#include "HttpResponse.h"

HttpResponse::HttpResponse()
    : version(Version::V_INVALID)
    , statusCode(StatusCode::S_INVALID)
    , statusMessage()
    , headers()
    , body()
{
}

HttpResponse::~HttpResponse() {}

std::string HttpResponse::serialize() const
{
    std::string ret;
    if (version == Version::V_INVALID || statusCode == StatusCode::S_INVALID) {
        return ret;
    }
    switch (version) {
    case Version::V_HTTP1_1:
        ret += "HTTP/1.1";
        break;
    default:
        break;
    }
    ret += " ";
    switch (statusCode) {
    case StatusCode::S_OK:
        ret += "200";
        break;
    case StatusCode::S_BADREQUEST:
        ret += "400";
        break;
    case StatusCode::S_NOTFOUND:
        ret += "404";
        break;
    default:
        break;
    }
    ret += " ";
    ret += statusMessage;
    ret += "\r\n";
    for (const auto& header : headers) {
        ret += header.first;
        ret += ": ";
        ret += header.second;
        ret += "\r\n";
    }
    ret += "\r\n";
    ret += body;
    return ret;
}