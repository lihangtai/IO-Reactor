#include "Socket.h"
#include "util/util.h"
#include "InetAddr.h"
#include <fcntl.h>
#include <unistd.h>

Socket::Socket()
    :sockfd_(socket(AF_INET, SOCK_STREAM, 0));
{     
}

Socket::Socket(int fd)
    :sockfd_(fd)
{
    perror_if(sockfd_ == -1; "socket");
}