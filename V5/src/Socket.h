#pragma once 
#include "InetAddr.h"
#include <fcntl.h>
#include <unistd.h>

#include <sys/socket.h>

class InetAddr;

class Socket{

    private:
        int sockfd_;

    public:
        Socket();
        Socket(int fd);
        ~Socket();
        void bind(const InetAddr *serv_addr);
        int accept(InetAddr* client_addr);
        void listen();
        void setNonblock();
        int fd(){return sockfd_;}
};

