#pragma once

#include <functional>
#include <unistd.h>
#include <fcntl.h>
#include <memory>
#include "Epoll.h"
class EventLoop;

class Channel
{  
    public:
        using EventsCallback = std::function<void()>; 

    public:
        Channel(EventLoop* loop, int fd);
        ~Channel();
        void setEvents(int events);
        int Event()const;
        void setRevents(int events);
        int Revent()const;

        bool isInEpoll()const;
        void setInEpoll(bool in);
        int Fd()const;

        void SetReadCallback(EventsCallback cb){readCallback_ = std::move(cb);}
        void SetWriteCallback(EventsCallback cb){writeCallback_ = std::move(cb);}

        void setCloseCallback(EventsCallback cb){closeCallback_ = std::move(cb);}

        void setCloseCallback(EventsCallback cb){closeCallback_ = std::move(cb);}
        void setErrorCallback(EventsCallback cb){errorCallback_ = std::move(cb);}

        void enableReading(){events_ |= (EPOLLIN | EPOLLPRI); update(); }
        void enableWriting(){events_ |= EPOLLOUT ; update();}
        void disableReading(){ events_ &= ~(EPOLLIN | EPOLLPRI); update();}
        
        void disableWriting(){ events_ &= ~EPOLLOUT; update();}

        void disableAll(){events_ = 0; update();}

        bool isNoneEvent() const{ return events_ == 0; }
        bool isWrite() const {return events_ & EPOLLOUT;}

        bool isRead() const {return events_ &(EPOLLIN | EPOLLPRI);}

        void handleEvent();

        void remove();

        void tie(const std::shared_ptr<void>&);

        

    private:
        void handleEventsWithGuard();
        void update();
    
    private:
        EventsCallback readCallback_;
        EventsCallback writeCallback_;
        EventsCallback closeCallback_;
        EventsCallback errorCallback_;
        int fd_;
        int events_;
        int revents_;
        bool isInEpoll_;
        EventLoop* loop_;
        std::weak_ptr<void> tie_;
        bool tied_;

};