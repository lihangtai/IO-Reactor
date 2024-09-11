/*
 一个Channel 封装了一个多路复用的IO 文件描述符，并添加了各种动作与属性 （完成封装）
 设置了这个channel面对读/写/关闭时候需要调用的回调函数
*/
#pragma once
#include <functional>
#include <unistd.h>
#include <fcntl.h>
#include "Epoll.h"

class EventLoop;

class Channel{

    public:
        
        using EventCallback = std::function<void()>;

    public:
        Channel(EventLoop *loop, int fd);

        void setEvents(int events);
        int Event()const;
        void setRevents(int events);
        int Revent()const;

        bool isInEpoll();
        void setInEpoll(bool in);
        int Fd()const;

        void setReadCallback(EventCallback cb){readCallback_ = std::move(cb);}
        void setWriteCallback(EventCallback cb){ writeCallback_ = std::move(cb);}
        void setCloseCallback(EventCallback cb){ closeCallback_ = std::move(cb);}

        void enableReading(){ events_ |= (EPOLLIN | EPOLLPRI), update();}
        void disableReading(){ events_ &= ~(EPOLLIN | EPOLLPRI); update();}
        void enableWriting() { events_ |= EPOLLOUT; update(); }			//注册可写事件
	    void disableWriting() { events_ &= ~EPOLLOUT; update(); }		//注销可写事件
	    void disableAll() {events_ = 0; update();}	//注销所有事件


        bool isNoneEvent()const { return events_ == 0; }
	    bool isWrite()const {return events_ & EPOLLOUT; }
	    bool isRead()const {return events_& (EPOLLIN | EPOLLPRI); }

	    void handleEvent();
	    void remove();
    

    private:
        void update();
    
    private:
        EventLoop* loop_;
        int fd_;
        int events_;
        int revents_;
        bool isInEpoll_;

        EventCallback readCallback_;
        EventCallback writeCallback_;
        EventCallback closeCallback_;  // 
};