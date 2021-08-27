#include "EventLoop.h"
#include "Log.h"

#include <iostream>
#include <thread>

EventLoop loop;

void func(void)
{
    std::cout << "thread " << std::this_thread::get_id() << ":" << std::endl;
    loop.loop();
}

int main()
{

    EventLoop loop;
    std::thread t(func);
    func();

    t.join();

    return 0;
}

// void threadFunc(void) {
//     std::cout << "thread " << std::this_thread::get_id() << ":" << std::endl;
//     EventLoop loop;
//     loop.loop();
// }

// int main() {
//     std::thread t(threadFunc);
//     threadFunc();

//     t.join();
// }