#pragma once
#include "InetAddr.h"


class Socket
{

public:
    Socket();
    Socket(int fd);
    ~Socket();

    void bind(const InetAddr &server_addr);

    void listen();

    int accept(InetAddr* addr);
    
    void setNonblock();

    int fd()const{return fd_;}

private:

    int fd_;
   
};