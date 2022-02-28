#include "HttpResponce.h"

std::string HttpResponse::serialize() const
{
    std::string ret;
    if (version == Version::INVALID || statusCode == StatusCode::INVALID) {
        return ret;
    }
    switch (version) {
    case Version::HTTP1_1:
        ret += "HTTP/1.1";
        break;
    }
    ret += " ";
    switch (statusCode) {
    case StatusCode::OK:
        ret += "200";
        break;
    case StatusCode::NOTFOUND:
        ret += "404";
        break;
    }
    ret += " ";
    ret += statusMessage;
    ret += body;
    return ret;
}