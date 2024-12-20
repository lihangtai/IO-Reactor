#pragma once
#include <string>
#include <arpa/inet.h>


class InetAddr{

    public:
        InetAddr();
        InetAddr(unsigned short port, const char* ip = nullptr);
        InetAddr(const struct sockaddr_in& addr):addr_(addr){}
        ~InetAddr();
        const struct sockaddr_in* getSockAddr() const { return &addr_; }
        void setAddr(const struct sockaddr_in& addr){addr_ = addr;}
        std::string toIp()const;
        std::string toIpPort()const;
        unsigned short toPort()const;

    private:
        struct sockaddr_in addr_;

};