#include "Channel.h"
#include "EventLoop.h"


Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop),
      fd_(fd),
      events_(0),
      revents_(0),
      isInEpoll_(0),
      tied_(false)
{}

void Channel::setEvents(int events){
    events_ = events;
}

int Channel::Event() const{
    return events_;
}

void Channel::setRevents(int revents)
{
	revents_ = revents;
}

int Channel::Revent() const {
    return revents_;
}

bool Channel::isInEpoll(){
    return isInEpoll_ == true;
}

void Channel::setInEpoll(bool in) {
    isInEpoll_ = in;
}

int Channel::Fd() const{
    return fd_;
}

void Channel::handleEvent(){
    
        
        if(tied_){
            auto guard = tie_.lock();
            if(guard){
                handleEventWithGuard();

            }
            //只有当对象依然存在的时候，才能安全的访问该对象，对象不存在则不调用
        }
        else{
            handleEventWithGuard();
            //用于建立连接，第一次连接的时候tied才为true
        }
    }


void Channel::tie(const std::shared_ptr<void>& obj){

    tie_ = obj;
    tied_ = true;
}

void Channel::handleEventWithGuard(){
    if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)){
        if(closeCallback_){
            closeCallback_();
        }
    }

    if(revents_ & EPOLLERR){
        if(errorCallback_){
            errorCallback_();
        }
    }

    if(revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)){
        if(readCallback_){
            readCallback_();
        }
    }
    if(revents_ & EPOLLOUT){
        if(writeCallback_){
            writeCallback_();
        }
    }
}

    

void Channel::update(){
    loop_->updateChannel(this);
}