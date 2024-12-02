#pragma once

#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>
#include <thread>

class ThreadPool
{
    public:
        using Task = std::function<void()>;
               
    public:
        explicit ThreadPool();
        ~ThreadPool();

        void start(int num);
        void stop();
        void runInThread();
        void add(Task task);

    private:
        std::vector<std::unique_ptr<std::thread>> threads_;
        std::queue<Task> task_;
        std::mutex mutex_;
        std::condition_variable cond_;
        bool running_;
};