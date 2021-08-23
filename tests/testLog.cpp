#include "Log.h"

#include <thread>
#include <vector>

void threadFunc()
{
    for (int i = 0; i < 1000; ++i) {
        LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz";
        LOG_WARN << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz";
        LOG_ERROR << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz";
        LOG_FATAL << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz";
    }
}

int main()
{
    log::manager.start();
    std::vector<std::thread> threadVec(16);

    for (int i = 0; i < 16; ++i) {
        threadVec[i] = std::thread(threadFunc);
    }

    for (int i = 0; i < 16; ++i) {
        threadVec[i].join();
    }

    return 0;

}