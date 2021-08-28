#include "Log.h"

#include <thread>
#include <vector>
#include <chrono>

void threadFunc()
{
    for (int i = 0; i < 1000000; ++i) {
        LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz";
    }
}

int main()
{
    int threadNum = 4;
    log::setAppender(log::FileAppender::ptr(new log::FileAppender("/tmp/testLog")));

    std::vector<std::thread> threadVec(threadNum);

    for (int i = 0; i < threadNum; ++i) {
        threadVec[i] = std::thread(threadFunc);
    }

    for (int i = 0; i < threadNum; ++i) {
        threadVec[i].join();
    }

    return 0;

}