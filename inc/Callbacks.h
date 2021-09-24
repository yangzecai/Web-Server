#pragma once

#include <memory>
#include <functional>

class TcpConnection;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using MessageCallback =
    std::function<void(const TcpConnectionPtr&, const char* buf, size_t len)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>; 