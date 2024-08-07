#pragma once


class InetAddr;


class Socket{

    private:
        int sockfd_;
    public:
        Socket();
        Socket(int fd);
        ~Socket();
        void bind(InetAddr* serv_addr);
        void listen();
        int accept(InetAddr* addr);
        void setNonblock();
        int fd()const { return sockfd_; }
};