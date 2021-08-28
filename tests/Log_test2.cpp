#include "Log.h"

int main()
{
    for (int i = 0; i < 10000000; ++i) {
        LOG_INFO << "Hello 0123456789"
                 << " abcdefghijklmnopqrstuvwxyz";
        log::setAppender(log::FileAppender::ptr(new log::StdoutAppender()));
    }
}