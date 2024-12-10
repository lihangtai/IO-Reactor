#include "EventLoopThread.h"


EventLoopThread::EventLoopThread()
    :_loop(nullptr)

{
    
}

EventLoopThread::~EventLoopThread(){
   if (loop_) {
		loop_->quit();
		if (thread_.joinable())
			thread_.join();
	}
    
}

EventLoop* EventLoopThread::startLoop(){
    thread_ = std::move(std::thread([this]() {threadFunc(); }));
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (loop_ == nullptr) {
            cond_.wait(lock);
        }
    }
    return loop_;
}

void EventLoopThread::threadFunc(){
    
    EventLoop loop;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond.notify();
    }
    loop.loop();
    std::lock_guard<std::mutex> lock(mutex_);
    loop_ = nullptr;
}