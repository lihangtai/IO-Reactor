#include"Epoll.h"
#include"util/util.h"
#include<string.h>
#include"Channel.h"
const int Ksize = 1024;

Epoll::Epoll()
    : epfd_(::epoll_create(1))
    , events(new struct epoll_event[1024])
{
    perror_if(epfd_ == -1, "epoll_create");
    memset(events, 0, sizeof(struct epoll_event)*1024);
}

Epoll::~Epoll(){
    ::close(epfd_);
}


void Epoll::updateChannel(Channel* ch){

    int fd = ch->Fd();

    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.data.fd = fd;
    ev.data.ptr = ch;
    ev.events = ch->Event();

    if(ch->isInEpoll()){
        if (ch->isNoneEvent()) {	//������channel�Ѳ������κ��¼����Ϳ��Խ���EPOLL_CTL_DEL
			del(ch);
		}
        else
        {
            int ret = ::epoll_ctl(fd, EPOLL_CTL_MOD, fd, &ev);
            perror_if(ret < 0, "epoll_ctl");
        }
    }
    else{
            int ret = ::epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev);
            perror_if(ret < 0, "epoll_ctl");
            ch->setInEpoll(true);
    }

    
}

void Epoll::del(Channel* ch)
{
    if(ch->isInEpoll()){
        int ret = ::epoll_ctl(epfd_, EPOLL_CTL_DEL, ch->Fd(), nullptr);
        perror_if(ret < 0, "epoll_ctl");
        ch->setInEpoll(false);
    }
}

Epoll::~Epoll()
{
    ::close(epfd_);
    delete [] events;
}

void Epoll::epoll_wait(std::vector<Channel*>& active, int timeout)
{

    int nfds = ::epoll_wait(epfd_, events, Ksize, timeout);

    if(nfds < 0){
        perror_if(true, "epoll_wait");
    }
    else{
        for(int i=0; i<nfds;i++){
            Channel* ch = static_cast<Channel*>(events[i].data.ptr);
            ch->setRevents(events[i].events);
            active.emplace_back(ch);
        }
    }
    
}