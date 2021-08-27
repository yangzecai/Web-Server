#pragma once

#include <memory>

class Poller {
public:
    using ptr = std::unique_ptr<Poller>;

    Poller();
    ~Poller() = default;

    Poller(const Poller&) = delete;
    Poller& operator=(const Poller&) = delete;

private:
    int epollfd_;
};