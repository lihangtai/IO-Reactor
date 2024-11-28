#include "Socket.h"
#include "util/util.h"
#include "InetAddr.h"
#include <unistd.h>
#include <fcntl.h>

Socket::Socket()
    :sockfd_(::socket(AF_INET, SOCK_STREAM, 0))
    {
        perror_if(sockfd_ == -1, "socket");
    }

Socket::Socket(int sockfd){
    sockfd_ = sockfd;
}

void Socket::bind(const InetAddr& serv_addr){

    int ret = ::bind(sockfd_,(struct sockaddr*)serv_addr.getSockAddr(), sizeof(sockaddr_in)); 
    perror_if(ret == -1, "bind");
}

int Socket::accept(InetAddr* addr){

    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);

    int cfd = ::accept4(sockfd_, (sockaddr*)&cliaddr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    perror_if(cfd == -1 , "accept");
    addr->setAddr(cliaddr);

    return cfd;
}


void Socket::listen(){

    int ret = ::listen(sockfd_, 128);
    perror_if(ret == -1, "listen");
}

void Socket::setNonblock(){

    int flags = :: fcntl(sockfd_, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(sockfd_, F_SETFL, flags);
}