#pragma once

#include "EventLoop.h"
#include <mutex>
#include <thread>
#include <condition_variable>

class EventLoopThread
{
public:

    EventLoopThread();
    ~EventLoopThread();
    EventLoop* startLoop();

private:
    EventLoop *_loop;
    std::mutex mtx_;
    std::conditon_variable cond_;
    std::thread thread_;

    void threadFunc();


};
