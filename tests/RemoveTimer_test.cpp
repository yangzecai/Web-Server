#include "EventLoop.h"
#include "Log.h"

#include <chrono>
#include <functional>
#include <iostream>

int cnt = 0;
EventLoop* g_loop;

void print(const char* msg)
{
    std::cout
        << "msg "
        << std::chrono::high_resolution_clock::now().time_since_epoch().count()
        << ' ' << msg << std::endl;
    if (++cnt == 20) {
        g_loop->quit();
    }
}

int main()
{
    log::setLevel(log::TRACE);

    EventLoop loop;
    g_loop = &loop;

    print("main");

    TimerId once1_timerid = loop.runAfter(std::chrono::milliseconds(1000),
                                       std::bind(print, "once1"));
    TimerId once1_5_timerid = loop.runAfter(std::chrono::milliseconds(1500), std::bind(print, "once1.5"));
    loop.runAfter(std::chrono::milliseconds(2500), std::bind(print, "once2.5"));
    loop.runAfter(std::chrono::milliseconds(3500), std::bind(print, "once3.5"));
    TimerId every2_timerid = loop.runEvery(std::chrono::milliseconds(2000),
                                        std::bind(print, "every2"));
    loop.runEvery(std::chrono::milliseconds(3000), std::bind(print, "every3"));
    loop.runAt(std::chrono::high_resolution_clock::now() +
                   std::chrono::seconds(4),
               [&every2_timerid, &loop]() {
                   loop.cancel(every2_timerid);
                   print("cancel timer every2");
               });
    loop.runAt(std::chrono::high_resolution_clock::now() +
                   std::chrono::seconds(1),
               [&once1_timerid, &loop]() {
                   loop.cancel(once1_timerid);
                   print("cancel timer once1");
               });
    loop.runAt(std::chrono::high_resolution_clock::now() +
                   std::chrono::seconds(1),
               [&once1_5_timerid, &loop]() {
                   loop.cancel(once1_5_timerid);
                   print("cancel timer once1.5");
               });

    loop.loop();

    std::cout << "main print exits" << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));
}