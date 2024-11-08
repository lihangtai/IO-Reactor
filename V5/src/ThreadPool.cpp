#include "ThreadPool.h"
#include <assert.h>

ThreadPool::ThreadPool()
        : running_(false){

        }

ThreadPool::~ThreadPool(){
    
    if(running_ ){
        running_ = false;
    }
}

void ThreadPool::start(int num){
    
    running_ = true;
    threads_.reserve(num);

    for(int i = 0; i < num; i++){
        threads_.emplace_back(std::make_unique<std::thread>([this](){
            printf("thread start\n");
            runInThread();
        })
        );
    }
}
//多个线程阻塞等待add()函数增加新任务，并用条件变量通知线程池

void ThreadPool::stop(){

    {
    std::unique_lock<std::mutex> lock(mutex_);
    running_ = false;
    cond_.notify_all();
    printf("stop running_ is false\n");
    }
    for(auto& thr: threads_)
        thr->join();
}

void ThreadPool::add(Task task){
    if(threads_.empty()){
        task();
    }
    else{

        {
            std::unique_lock<std::mutex> lock(mutex_);

            if(!running_){
                return;
            }
            tasks_.push(std::move(task));
        }

        cond_.notify_one();
    }

}

void ThreadPool::runInThread(){
    printf("runningthread start\n");

    while(running_){
        Task task;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cond_.wait(lock, [this](){return !running_ || !tasks_.empty();});

            if(!tasks_.empty()){
                task = std::move(tasks_.front());
                tasks_.pop();
            }
        
        }
        if(task){
            task();
        }
        
    }
    printf("thread exit\n");
}