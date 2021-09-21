#include "EventLoop.h"
#include "EventLoopThread.h"
#include "Util.h"
#include "Log.h"

#include <iostream>
#include <chrono>

#include <unistd.h>

void runInThread()
{
  printf("runInThread(): pid = %d, tid = %d\n",
         getpid(), getCurThreadId());
}

int main()
{
  log::setLevel(log::TRACE);
  printf("main(): pid = %d, tid = %d\n",
         getpid(), getCurThreadId());

  EventLoopThread loopThread;
  EventLoop* loop = loopThread.startLoop();
  loop->runInLoop(runInThread);
  sleep(1);
  loop->runAfter(std::chrono::seconds(2), runInThread);
  sleep(3);
  loop->quit();

  printf("exit main().\n");
}
