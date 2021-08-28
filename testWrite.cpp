#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/time.h>
#include <thread>
#include <cstdio>

auto getTime()
{
    using namespace std::chrono;
    static const int kMicroPerSec = 1000000;
    static char timeStr[32];
    // uint64_t time = duration_cast<microseconds>(
    //                     steady_clock::now().time_since_epoch())
    //                     .count();
    // time_t seconds = time / kMicroPerSec;
    // uint64_t micros = time % kMicroPerSec;

    ::timeval tv;
    ::gettimeofday(&tv, NULL);
    // ::time_t seconds = tv.tv_usec;
    // ::suseconds_t micros = tv.tv_sec;

    auto tm_time = std::localtime(&tv.tv_sec);

    // std::strftime(timeStr, 32, "%Y-%m-%d %T.", std::localtime(&seconds));
    std::snprintf(timeStr, sizeof(timeStr), "%4d%02d%02d %02d:%02d:%02d",
        tm_time->tm_year + 1900, tm_time->tm_mon + 1, tm_time->tm_mday,
        tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec);
    // std::snprintf(timeStr + 20, 7, "%06lu", micros);

    return timeStr;
}

int main()
{
    // long long total = 10LL * 1024 * 1024 * 1024;

    // int strSize = 10;
    // std::string testStr = std::string(strSize, 'a');
    // int iterNum = total / strSize;

    int iterNum = 10000000;

    std::ofstream fs;
    // fs.open("/tmp/testLog", std::ios::out | std::ios::trunc);
    auto file = fopen("/tmp/testLog", "w");
    auto start = std::time(NULL);
    for (int i = 0; i < iterNum; ++i) {
        // fs << getTime();
        fwrite(getTime(), 1, 20, file);
    }
    fflush(file);
    auto end = std::time(NULL);
    fclose(file);
    std::cout << end - start << std::endl;
}
