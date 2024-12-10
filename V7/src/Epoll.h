#pragma once
#include <sys/epoll.h>
#include "EventLoop.h"
#include "Channel.h"


class Epoll
{

public:

    Epoll();

    ~Epoll();

    void updateChannel(Channel* ch);
    void del(Channel* ch);
    int GetEpollfd()const { return epfd_; }

    void epoll_wait(std::vector<Channel*>& active, int timeout = 10);
    

private:
    int epfd_;
    struct epoll_event* events;

};