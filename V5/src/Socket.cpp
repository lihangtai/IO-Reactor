#include "Socket.h"
#include "util/util.h"

Socket::Socket()
    :sockfd_(socket(AF_INET, SOCK_STREAM, 0))
{     
}

Socket::Socket(int fd)
    :sockfd_(fd)
{
    perror_if(sockfd_ == -1, "socket");
}

Socket::~Socket(){

    close(sockfd_);
}

void Socket::bind(const InetAddr *serv_addr){

    int ret = ::bind(sockfd_, (struct sockaddr*)serv_addr->getSockAddr(), sizeof(struct sockaddr_in));
    
        perror_if(ret == -1, "bind");
    
}


void Socket::listen(){
    int ret = ::listen(sockfd_, 5);
    perror_if(ret == -1, "listen");
}

int Socket::accept(InetAddr* client_addr){

    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);
    int connfd = ::accept(sockfd_, (struct sockaddr*)&cliaddr, &len);
    perror_if(connfd == -1, "accept");
    client_addr->setAddr(cliaddr);
    return connfd;
}


void Socket::setNonblock()
{
    int flag = fcntl(sockfd_, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(sockfd_, F_SETFL, flag);

}