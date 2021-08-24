#include "Log.h"

#include <thread>
#include <vector>
#include <chrono>

void threadFunc()
{
    for (int i = 0; i < 1000; ++i) {
        LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz";
        // LOG_FATAL << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz";
    }
}

int main()
{
    log::manager.start();
    log::manager.setAppender(new log::FileAppender("/tmp/testLog"));

    std::vector<std::thread> threadVec(16);

    for (int i = 0; i < 16; ++i) {
        threadVec[i] = std::thread(threadFunc);
    }

    for (int i = 0; i < 16; ++i) {
        threadVec[i].join();
    }

    return 0;

}