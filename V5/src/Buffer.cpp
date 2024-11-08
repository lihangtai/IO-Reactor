#include "Buffer.h"
#include <errno.h>
#include <unistd.h>
#include <sys/uio.h>

ssize_t Buffer::readFd(int fd, int* saveErrno){
    char extrabuffer[65535];
    struct iovec vec[2];

    auto writable = writeableBytes();
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;

    vec[1].iov_base = extrabuffer;
    vec[1].iov_len = sizeof(extrabuffer);

    auto iovcnt = (writable < sizeof(extrabuffer)) ?2 :1;
    auto n = ::readv(fd,vec, iovcnt);
    if(n < 0){
        *saveErrno = errno;
    }
    else if(static_cast<size_t>(n) <=writable){
       
        writerIndex_ += n;

    }
    else{

        writerIndex_ = buffer_.size();
        append(extrabuffer, n - writable);
    }

    return n;
}

ssize_t Buffer::writeFd(int fd, int* saveErrno){

    auto n  = ::write(fd, peek(), readableBytes());
    if(n < 0){

        *saveErrno = errno;
    }
    return n;
}