#include "Log.h"

#include <chrono>
#include <iostream>

float testSpeed(int num)
{
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num; ++i) {
        LOG_INFO << "Hello 0123456789"
                 << " abcdefghijklmnopqrstuvwxyz";
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto microsec =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start)
            .count();
    return float(num) / microsec * 1000 * 1000;
}

int main()
{
    int testNum = 10000000;
    // auto stdoutSpeed = testSpeed(testNum);
    log::setAppender(log::FileAppender::ptr(new log::FileAppender("/tmp/testLog")));
    auto nullSpeed = testSpeed(testNum);
    // log::setAppender(log::FileAppender::ptr(new log::FileAppender("/tmp/testLog")));
    // auto testlogSpeed = (testSpeed(testNum));

    // std::cout << "stdout : " << stdoutSpeed << " log line per sec" << std::endl;
    std::cout << "null : " << nullSpeed << " log line per sec" << std::endl;
    // std::cout << "testlog : " << testlogSpeed << " log line per sec"
    //           << std::endl;
}