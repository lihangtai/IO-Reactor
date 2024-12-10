#include "EventLoopThreadPool.h"

#include "EventLoopThread.h"


EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop)
    :baseLoop_(baseLoop),
    started_(false),
    numThreads_(0),
    next_(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
    
}

void EventLoopThreadPool::start(){

    started_ = true;
    for(int i=0; i < numThreads_; ++i){
        EventLoopThread* t = new EventLoopThread();
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        
        loops_.push_back(t->startLoop());
    }
}