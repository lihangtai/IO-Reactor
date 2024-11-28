#include "EventLoop.h"
#include "Channel.h"
#include "Epoll.h"
#include <signal.h>
#include "errno.h"


int createEventfd(){

    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtfd < 0){
        perror("EventLoop::createEventfd");
    } 
    return evtfd;
}
EventLoop::EventLoop()
    :threadId_(std::this_thread::get_id())
    ,quit_(false)
    ,callingPendingFunctors_(false)
    ,ep_(std::make_unique<Epoll>())
    ,wakeupFd_(createEventfd())
    ,wakeupChannel_(std::make_unique<Channel>(wakeupFd_))
    {
        wakeupChannel_->SetReadCallback([this]{handleRead();});
        wakeupChannel_->enableReading();
    }

EventLoop::~EventLoop(){
    wakeupChannel_->remove();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
}

void EventLoop::loop()
{
    quit_ = false;
    while(!quit_){
        activeChannels_.clear();
        ep_->Epoll_wait(activeChannels_, 10000);
        for(auto& active : activeChannels_){
            active->handleEvent();
        }
        doPendingFunctors();
    }
}

void EventLoop::updateChannel(Channel* ch){

    ep_->updateChannel(ch);
}

void EventLoop::removeChannel(Channel* ch){
    ep_->del(ch);
}

void EventLoop::quit(){
    quit_ = true;
}

void EventLoop::runInLoop(Functor cb){

    if(isInLoopThread()){
        cb();
    }
    else{
        queueInLoop(std::move(cb));
    }
       
}

void EventLoop::queueInLoop(Functor cb){

    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctors_.push_back(std::move(cb));
    }

    if(!isInLoopThread() || callingPendingFunctors_){
        wakeup();
    }

}

void EventLoop::handleRead(){

    uint64_t one =1;
    auto n = ::read(wakeupFd_, &one, sizeof one);

    if(n !=sizeof(one)){
        printf("EventLoop::handleRead() reads %lu bytes \n", n);
    }
};

void EventLoop::doPendingFunctors(){

    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for(const auto& functor: functors){
        functor();
    }
    callingPendingFunctors_ = false;
}

void EventLoop::wakeup(){

    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof(one));

    if( n!=sizeof(one)){
        printf("EventLoop wakeup write %lu bytes insteadof 8 \n", n);
    }
}