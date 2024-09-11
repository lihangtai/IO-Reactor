#pragma once

#include <arpa/inet.h>


namespace sockets{
    
    void setNonblock(int sockfd);

    void shutdownWrite(int sockfd);

    int getSocketError(int sockfd);

    struct sockaddr_in getLocalAddr(int sockfd);
    struct sockaddr_in getPeerAddr(int sockfd);
}

void perror_if(bool condition, const char* errorMessage);