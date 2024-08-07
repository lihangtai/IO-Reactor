#include "Channel.h"
#include "EventLoop.h"

Channel::Channel(EventLoop* loop, int fd)
    :loop_(loop)
    ,fd_(fd)
    ,events_(0)
    ,revents_(0)
    ,isInEpoll_(0)
    {

}


void Channel::setEvents(int events){
    
    events_ = events;
}

int Channel::Event()const
{
	return events_;
}
void Channel::setRevents(int revents)
{
	revents_ = revents;
}
int Channel::Revent()const
{
	return revents_;
}

bool Channel::isInEpoll()
{
	return isInEpoll_ == true;
}

void Channel::setInEpoll(bool in)
{
	isInEpoll_ = in;
}

int Channel::Fd()const
{
	return fd_;
}

void Channel::handleEvent(){

    if(events_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)){

        if(readCallback_){
            readCallback_();
        }  //  std::function 类定义中由一个operator bool 布尔转换符
    }
    if(revents_ & EPOLLOUT){
        if(writeCallback_){

            writeCallback_();
        }
    }
}

void Channel::remove()
{
	loop_->removeChannel(this);
}

void Channel::update()
{
	loop_->updateChannel(this);
}