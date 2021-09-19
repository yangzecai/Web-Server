#include "EventLoop.h"
#include "Log.h"

#include <functional>
#include <iostream>
#include <chrono>

int cnt = 0;
EventLoop* g_loop;

void print(const char* msg)
{
//   printf("msg %s %s\n", , msg);
    std::cout << "msg " << msg << std::endl; 
    if (++cnt == 20)
    {
        g_loop->quit();
    }
}

int main()
{
    log::setLevel(log::TRACE);

    EventLoop loop;
    g_loop = &loop;

    std::cout << "main" << std::endl;

    loop.runAfter(std::chrono::milliseconds(1000), std::bind(print, "once1"));
    loop.runAfter(std::chrono::milliseconds(1500), std::bind(print, "once1.5"));
    loop.runAfter(std::chrono::milliseconds(2500), std::bind(print, "once2.5"));
    loop.runAfter(std::chrono::milliseconds(3500), std::bind(print, "once3.5"));
    loop.runEvery(std::chrono::milliseconds(2000), std::bind(print, "every2"));
    loop.runEvery(std::chrono::milliseconds(3000), std::bind(print, "every3"));

    loop.loop();
    
    std::cout << "main print exits" << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));
}