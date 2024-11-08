#pragma once 
#include <functional>
#include "InetAddr.h"

#include "Socket.h"
#include "Channel.h"

class EventLoop;


class Acceptor
{

public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddr&)>;

public:
    Acceptor(const InetAddr& listenAddr, EventLoop* eventloop);
    ~Acceptor();

    void setNewconnectionCallback(const NewConnectionCallback& cb){newconnectioncallback_ = cb;}

    void listen();


private:
    void handleRead();

    EventLoop* loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;

    NewConnectionCallback newconnectioncallback_;
    bool listen_;
};