#include "Buffer.h"
#include "HttpRequest.h"

#include <iostream>

int main()
{
    HttpRequest request;
    Buffer buffer;
    buffer.append("GET /asd HTTP/1.1\r\n"
                  "Connection: close\r\n"
                  "Keep-Alive: timeout=10\r\n"
                  "\r\n"
                  "asd");
    if (false == request.parse(buffer)) {
        std::cout << "fail to parse" << std::endl;
    } else {
        std::cout << request.toString() << std::endl;
    }
}