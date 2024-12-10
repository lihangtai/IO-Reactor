#pragma once

#include<functional>
#include"InetAddr.h"

#include"Socket.h"
#include"Channel.h"


class Acceptor
{

pubic:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddr&)>;

    Acceptor(const InetAddr& listenAddr, EventLoop* eventloop);
    ~Acceptor();

    void listen();

    void setNewconnectionCallback(const NewConnectionCallback& cb)
    {
        newConnectionCallback = cb;
    }

private:

    void handleRead();

    Socket acceptSocket_
    EventLoop* loop_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback;
    bool listen_;
}