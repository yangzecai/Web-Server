#include "Log.h"

#include <thread>
#include <vector>
#include <chrono>

void threadFunc()
{
    for (int i = 0; i < 1000; ++i) {
        LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz";
    }
}

int main()
{
    log::setAppender(log::FileAppender::ptr(new log::FileAppender("/dev/null")));

    std::vector<std::thread> threadVec(16);

    for (int i = 0; i < 16; ++i) {
        threadVec[i] = std::thread(threadFunc);
    }

    for (int i = 0; i < 16; ++i) {
        threadVec[i].join();
    }

    return 0;

}