#include  "EventLoop.h"
#include "Channel.h"
#include "Epoll.h"


EventLoop::EventLoop(){

    ep_ = std::make_unique<Epoll>();  // unique_ptr 与 make_unique 配合使用
}

EventLoop::~EventLoop(){

}

void EventLoop::loop(){

    quit_ = false;
    while(!quit_){

        activechannels_.clear();
        ep_-> Epoll_wait(activechannels_);
  
        for(auto& active: activechannels_){      //C++11 标准引入了范围-based for 循环，使得对集合（如数组、容器等）的遍历更加简便
            active->handleEvent();
        }

    }
}

void EventLoop::updateChannel(Channel* channel)
{
	ep_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
	ep_->del(channel);
}

void EventLoop::quit() {
	quit_ = true;
}