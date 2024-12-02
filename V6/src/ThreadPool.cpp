#include "ThreadPool.h"
#include <assert.h>

ThreadPool::ThreadPool()
    :running_(false)
{

}

ThreadPool::~ThreadPool(){
    if(running_){
        stop();
    }
}

void ThreadPool::start(int num){

    assert(num > 0);
    running_ = true;
    threads_.resize(num);

    for(int i =0;i<num;i++){
        threads_.push_back(std::make_unique<std::thread>([this](){

            runInThread();
        })
        );
    }
}

void ThreadPool::runInThread(){

    while(running_){
        
        Task task;
        {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this](){return !task_.empty() || !running_;});

        if(!task_.empty()){
            task = std::move(task_.front());
            task_.pop();
        }

        }
        if(task){
            task();
        }
           
    }
}

void ThreadPool::add(Task task){
    
    if(threads_.empty()){
        task();
    }

   else{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        task_.push(std::move(task));
    }
   cond_.notify_one();
   }

}

void ThreadPool::stop(){

    {
        std::unique_lock<std::mutex>lock(mutex_);
        running_ = false;
        cond_.notify_all();
    }

    for(auto& thr: threads_){
        thr->join();
    }

}