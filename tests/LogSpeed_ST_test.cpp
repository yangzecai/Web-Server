#include "Log.h"

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

void bench(bool longLog)
{

    int cnt = 0;
    const int kBatch = 1000;
    std::string empty = " ";
    std::string longStr(3000, 'X');
    longStr += " ";

    for (int t = 0; t < 30; ++t) {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < kBatch; ++i) {
            LOG_INFO << "Hello 0123456789"
                     << " abcdefghijklmnopqrstuvwxyz "
                     << (longLog ? longStr : empty) << cnt;
            ++cnt;
        }
        auto end = std::chrono::high_resolution_clock::now();
        printf("%f\n",
               (float)std::chrono::duration_cast<std::chrono::nanoseconds>(
                   end - start)
                       .count() /
                   1000 / kBatch);
        // struct timespec ts = {0, 500 * 1000 * 1000};
        // nanosleep(&ts, NULL);
    }
}

int main(int argc, char* argv[])
{
    bool longLog = argc > 1;

    bench(longLog);
}