#include "Epoll.h"
#include "util/util.h"
#include <string.h>
#include "Channel.h"

const int SIZE = 1024;

Epoll::Epoll()
: epfd_(epoll_create(1)),
  events_(new epoll_event[SIZE])
{
    perror_if(epfd_ == -1, "epoll_create");
    memset(events_, 0 ,sizeof(SIZE*sizeof(epoll_event)));
}

Epoll::~Epoll(){

    if(epfd_!=-1){
        epfd_ = -1;
    }

    delete [] events_;
}

void Epoll::updateChannel(Channel* ch){

    int fd = ch->Fd();

    struct epoll_event ev;

    memset(&ev, 0, sizeof(ev));

    ev.data.ptr = ch;
    ev.events = ch->Event();

    if(ch->isInEpoll()){
        int ret = epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev);
        perror_if(ret == -1, "epoll_ctl mod");
    }
    else{
        int ret = epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev);
        perror_if(ret == -1, "epoll_ctl add");
        ch->setInEpoll(true);
    }
}

    void Epoll::del(Channel* ch){
        int ret = epoll_ctl(epfd_, EPOLL_CTL_DEL, ch->Fd(), nullptr);
        perror_if(ret == -1, " epoll_ctl DEl");
        ch->setInEpoll(false);
    }


void Epoll::Epoll_wait(vector<Channel*>&active, int timeout){
    int nums = epoll_wait(epfd_,events_, SIZE, timeout);
    perror_if(nums == -1, "epoll_wait");
    for(int i = 0;i<nums;i++){
        Channel* ch = static_cast<Channel*>(events_[i].data.ptr);
        ch->setRevents(events_[i].events);
        active.emplace_back(ch);
    }
}
