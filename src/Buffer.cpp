#include "Buffer.h"
#include "Log.h"

#include <sys/uio.h>

ssize_t Buffer::writeFromFd(int fd)
{
    thread_local char extrabuf[65536];
    struct iovec vec[2];
    const size_t writableLen = getWritableBytes();
    vec[0].iov_base = beginOfWritableBytes();
    vec[0].iov_len = writableLen;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;
    const ssize_t n = readv(fd, vec, 2);
    if (n < 0) {
        LOG_SYSERROR << "Buffer::writeFromFd";
    } else if (static_cast<size_t>(n) <= writableLen) {
        writePos_ += n;
    } else {
        writePos_ = buffer_.size();
        append(extrabuf, n - writableLen);
    }
    return n;
}
