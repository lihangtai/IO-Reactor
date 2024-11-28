#pragma once
#include <vector>
#include <memory>
#include <functional>


class EventLoop;
class EventLoopThread;

class EventLoopThreadPool
{

    public:
        EventLoopThreadPool(EventLoop* base);
        ~EventLoopThreadPool();

        void setThreadNum(int numThreads){numThreads_ = numThreads;}

        void start();

        EventLoop* getNextLoop();

        bool started()const { return started_;}

    private:
        EventLoop* baseLoop_;
        bool started_;
        int numThreads_;
        int next_;
        std::vector<std::unique_ptr<EventLoopThread>> threads_;
        std::vector<EventLoop*> loops_;
};