#pragma once 
// 该类同一调度 eventloop和 socket创建 ，以及对应的channel 和对epoll的操作 
// 一个connection 对应着一个channel（一个socket），以及connection加入到事件循环Eventloop
// 顺便操作epoll对就绪的IO进行响应

/*对于输出用户态缓冲区：采用尽力而为的写操作。   当内核缓冲区可以写入（EPOLLOUT 事件触发）时，直接将数据写入文件描述符。
如果没有 EPOLLOUT 事件且用户态缓冲区为空，尝试写入数据，如果失败（并且不是因为 EWOULDBLOCK），处理错误。
如果有数据没有成功写入，将其保存到用户态缓冲区 outputBuffer_ 中，并确保在下次可以写入时会触发 EPOLLOUT 事件。*/

//对于输入的的用户态缓冲区：
#include <memory>
#include "Callbacks.h"
#include "Buffer.h"
#include "Socket.h"

class EventLoop;
class Channel;

/*这个继承使得可以使用share_from_this()方法，返回一个指向当前对象的指针
this指针不会计数 （容易导致未定义行为）
shared_from_this() 返回一个指向当前对象的智能指针，会计数，只有所有与该对象相关的共享指针都被释放时，对象才会被销毁。
*/
class Connection: public std::enable_shared_from_this<Connection>{

    public:       
        enum class StateE{ KDisconnected, KConnecting, KConnected, KDisconnecting};
    
    public:
        
        Connection(EventLoop* loop, int sockfd);
        EventLoop* getLoop()const {return loop_;};
        void setMessageCallback(const MessageCallback& cb){
            messageCallback_ = cb;
        };

        void setCloseCallback(const CloseCallback& cb){
            closeCallback_ = cb;
        }

        bool connected()const {return state_ == StateE::KConnected;}
        bool disconnected()const { return state_ == StateE::KDisconnected;}
        void setState(StateE state){state_ = state;}
        void send(Buffer* message);
        void send(const void* message, int len);
        void send(const std::string& message);

        void connectEstablished();

        void connectDestroyed();

        Buffer* inputBuffer(){ return &inputBuffer_;}
        Buffer* outputBuffer(){ return &outputBuffer_;}

        int fd()const{return socket_->fd();}
        
        void Connection::shutdown();

    private:
        void handleRead();
        void handleWrite();
        void handleClose();


    private:

        EventLoop* loop_;
        StateE state_;
        std::unique_ptr<Socket> socket_;
        std::unique_ptr<Channel> channel_;

        MessageCallback messageCallback_;    //回调函数入口
        CloseCallback closeCallback_;

        Buffer inputBuffer_;
        Buffer outputBuffer_;
};
