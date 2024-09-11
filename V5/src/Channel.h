#pragma once
#include <functional>
#include <unistd.h>
#include <fcntl.h>
#include <memory>
#include "Epoll.h"

class EventLoop;


/*
因为epoll需要处理fd，所以抽象了这个类，提供用于epoll的每个fd的操作函数，以及当事件到达的时候，会调用的回调函数（读，写，结束）
同时在这个类中还设定了一个布尔量
*/
class Channel{

public:
    using ReadEventCallback = std::function<void()>;
    using EventCallback = std::function<void()>;

public:
    Channel(EventLoop* loop, int fd);

    void setEvents(int events);

    void setRevents(int events);

    int Revent()const;
    
    int Event()const;

    bool isInEpoll();
    void setInEpoll(bool in);
    int Fd()const;

    void SetReadCallback(EventCallback cb){ readCallback_ = std::move(cb);}
    void setWriteCallback(EventCallback cb){writeCallback_ = std::move(cb);}
    void setCloseCallback(EventCallback cb){closeCallback_ = std::move(cb);}
    void setErrorCallback(EventCallback cb){errorCallback_ = std::move(cb);}

// EPOLLPRI 是文件描述符上有高优先级的数据可读
    void enableReading(){ events_ |= (EPOLLIN | EPOLLPRI); update();}

    void disableReading(){events_ &= ~(EPOLLIN | EPOLLPRI); update();}

    void enableWriting(){ events_ &= ~EPOLLOUT, update();}

    void disableWriting(){events_ &= ~EPOLLOUT; update();}

    void disableAll(){ events_ = 0; update();}

    bool isNoneEvent()const {return events_ == 0;}

    bool isWrite()const{return events_ & EPOLLOUT;}

    bool isRead()const {return events_ & EPOLLOUT;}

    void handleEvent();

    void remove();

    void tie(const std::shared_ptr<void>&);
private:

void update();
void handleEventWithGuard();


private:
    EventLoop* loop_;

    int fd_;
    int events_;
    int revents_;
    bool isInEpoll_;


//用来判定当前channel是否与其他某个对象进行了绑定，谁拥有了当前的channel，使用weak指针来接收是担心循环引用导致的问题
    std::weak_ptr<void> tie_;
    bool tied_;

    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};