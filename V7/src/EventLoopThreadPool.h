#pragma once
#include "EventLoop.h"
#include "EventLoopThread.h"
#include <vector>
#include <memory>
#include <functional>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool{

    EventLoopThreadPool(EventLoop* baseloop);
    ~EventLoopThreadPool();

    void start();
    void setThreadNum(int numThreads){numThreads_ = numThreads;}

    EventLoop* getNextLoop();

    bool started()const{return started;}


private:
    EventLoop* baseLoop;
    bool started;
    int numThreads;
    std::vector<std::unique_ptr<EventLoopThread>> threads;
    std::vector<EventLoop*> loops;

}