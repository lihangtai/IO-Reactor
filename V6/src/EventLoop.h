#pragma once
#include <vector>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <sys/eventfd.h>
#include <functional>
class Channel;
class Epoll;


class EventLoop{

    public:
        using Functor = std::function<void()>;
        using channelList = std::vector<Channel*>;

    public:
        EventLoop();
        ~EventLoop();

        void loop();
        void updateChannel(Channel* channel);
        void removeChannel(Channel* channel);

        bool isInLoopThread()const{return threadId_ == std::this_thread::get_id();}

        void runInLoop(Functor cb);
        void queueInLoop(Functor cb);
        

        void wakeup();

        std::thread::id getThreadId(){return threadId_;}

        void quit();

        void assertInLoopThread(){
            if(!isInLoopThread()){
                printf("not in this loopThread\n");
            }
        }


    private:
        void doPendingFunctors();
        void handleRead();

        std::thread::id threadId_;
        std::atomic_bool quit_;
        std::atomic_bool callingPendingFunctors_;

        std::unique_ptr<Epoll> ep_;
        channelList activeChannels_;

        int wakeupFd_;
        std::unique_ptr<Channel> wakeupChannel_;

        std::vector<Functor> pendingFunctors_;
        
        std::mutex mutex_;
};
         











































































































































































































































