#include "Util.h"

#include <unistd.h>
#include <sys/syscall.h>

__thread int t_tid;

int getCurThreadId()
{
    if (__builtin_expect(t_tid == 0, 0)) {
        t_tid = static_cast<int>(::syscall(SYS_gettid));
    }
    return t_tid;
}