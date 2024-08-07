#pragma once
#include<string>
#include<arpa/inet.h>
#include<stdio.h>

class InetAddr
{
    public:
        InetAddr();
        InetAddr(int port , const char * ip = "127.0.0.1");
        const sockaddr_in* getAddr()const {return &addr_;}
        void setAddr(const struct sockaddr_in &addr){addr_= addr;}
        std::string toIp()const;
        std::string toIpPort()const;
        int toPort()const;


    
    private:
        struct sockaddr_in addr_;
};
