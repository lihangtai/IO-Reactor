#pragma once

#include <functional>
#include "InetAddr.h"

#include "Socket.h"
#include "Channel.h"


class EventLoop;

class Acceptor
{
    public:
        using NewConnectionCallback = std::function<void(int sockfd)>;

    public:
        Acceptor(const InetAddr& listenAddr, EventLoop* loop);
        ~Acceptor();

        void setNewconnectionCallback(const NewConnectionCallback& cb){newConnectionCallback_ = cb;}

        void listen();


    private:
        EventLoop* loop_;
        Socket acceptSocket_;
        Channel acceptChannel_;

        NewConnectionCallback newConnectionCallback_;
        bool listen_;

        void handleRead();

};
