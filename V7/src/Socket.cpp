#include "Socket.h"
#include "util/util.h"
#include "InetAddr.h"
#include <fcntl.h>
#include <unistd.h>


Socket::Socket()
    :fd_(::socket(AF_INET,SOCK_STREAM,0))
    {
        perror_if(fd_==-1,"socket error");
    }

Socket::Socket(int sockfd)
    :fd_(sockfd)
    {
        perror_if(fd_==-1,"socket error");
    }

Socket::~Socket(){
    
    if (fd_ != -1) {
		close(fd_);
		fd_= -1;
	}
}

void Socket::bind(const InetAddr& serv_addr){
    
    int ret = ::bind(fd_, (struct sockaddr*)serv_addr.getSockAddr(), sizeof(struct sockaddr) );
    perror_if(ret==-1,"bind error");

}

int Socket::accept(InetAddr* addr){

    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    int cfd = ::accept4(fd_, (struct sockaddr*)&cliaddr, &clilen, SOCK_NONBLOCK | SOCK_CLOEXEC);
    perror_if(cfd==-1,"accept error");
    addr->setAddr(cliaddr);

    return cfd;

}

void Socket::listen()
{
	int ret = ::listen(fd_, 128);
	perror_if(ret == -1, "listen");
}

void Socket::setNonblock()
{
	int flag = fcntl(fd_, F_GETFL);
	flag |= O_NONBLOCK;
	fcntl(fd_, F_SETFL, flag);
}