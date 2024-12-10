#include"Buffer.h"
#include<sys/uio.h>
#include<unistd.h>
#include<errno.h>

ssize_t Buffer::readFd(int fd, int* saveErrno)
{
	char extrabuffer[65535];
	struct iovec vec[2];

	auto writable = writeableBytes();

	vec[0].iov_base = begin() + writerIndex_;	//��һ�黺����
	vec[0].iov_len = writable;	//iov_base��������д�Ŀռ��С

	vec[1].iov_base = extrabuffer;	//�ڶ��컺����
	vec[1].iov_len = sizeof(extrabuffer);

	//��Buffer��65535�ֽڵĿռ�ռ䣬�Ͳ�����ջ�ϵĻ�����
	auto iovcnt = (writable < sizeof(extrabuffer)) ? 2 : 1;
	auto n = ::readv(fd, vec, iovcnt);
	if (n < 0) {
		*saveErrno = errno;
	}
	else if (static_cast<size_t>(n) <= writable) {
		writerIndex_ += n;
	}
	else {
		//Buffer�ײ�Ŀ�д�ռ䲻�����n�ֽ����ݣ�
		writerIndex_ = buffer_.size();	//����writerIndex_Ϊĩβ����ʹ��append
		append(extrabuffer, n - writable);
	}

	return n;
}
ssize_t Buffer::writeFd(int fd, int* saveErrno)
{
	auto n = ::write(fd, peek(), readableBytes());
	if (n < 0) {
		*saveErrno = errno;
	}
	return n;
}

