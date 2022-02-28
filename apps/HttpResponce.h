#pragma once

#include <string>

struct HttpResponse {
    enum Version
    {
        INVALID,
        HTTP1_1
    };

    enum StatusCode
    {
        INVALID,
        OK = 200,
        NOTFOUND = 404
    };

    Version version;
    StatusCode statusCode;
    std::string statusMessage;
    std::string body;

    std::string serialize() const;
};
