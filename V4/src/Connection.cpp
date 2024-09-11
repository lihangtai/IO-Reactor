#include "Connection.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Socket.h"

Connection::Connection(EventLoop* loop, int fd)
    :loop_(loop),
     state_(StateE::KDisconnected),
     socket_(std::make_unique<Socket>(fd)),
     channel_(std::make_unique<Channel>(loop, fd))
{
    channel_->setReadCallback([this](){handleRead();});
    channel_->setWriteCallback([this](){handleWrite();});

}

void Connection::send(Buffer* message){

}
void Connection::send(const void* message, int len){

}

/*对于一个connection的用户态输出缓冲区，只有当内核缓冲区可以写入时(EPOLLOUT触发时)才可以写入，否则存储在用户态缓冲区中
但send函数为了不影响connection中的用户态缓冲区的读出，使用的是一种尽力而为的写法。即使EPOLLOUT未触发，且用户态缓冲区为空时
也调用write，写不进入就存入用户态缓冲区，当EPOLLOUT触发时，再调用write将用户态缓冲区中的数据写入内核缓冲区
*/
void Connection::send(const std::string& message){

    if(state_ == StateE::KDisconnected)
    {
        return ;
    }
    bool faultError = false;
    ssize_t nwrote = 0;
    size_t remaining = message.size();
    if(!channel_->isWrite() && outputBuffer_.readableBytes() == 0){
        nwrote = ::write(fd(), message.data(), message.size());

        if(nwrote >= 0){
            remaining = message.size() - nwrote;
        }
        else{
            nwrote = 0;
            if(errno !=EWOULDBLOCK){    //在非阻塞IO模式下，如果操作不能立刻完成（内存缓冲区没有准备好），则返回这个错误
                if(errno == EPIPE || errno == ECONNRESET){
                    faultError = true;
                }          // EPIPE：套接字的另一边已经关闭    ECONNRESET：远程主机关闭了连接
            }
        }
    }
    assert(remaining <= message.size());

    if(!faultError && remaining >0){
        outputBuffer_.append(message.data() + nwrote, remaining);
        if(!channel_->isWrite()){
            channel_->enableWriting();
        }
    }

}


void Connection::connectEstablished(){
    assert(state_ == StateE::KConnecting);
    setState(StateE::KConnected);
    channel_->enableReading();
}

void Connection::connectDestroyed(){

    if(state_ == StateE::KConnected){
        setState(StateE::KDisconnected);
        channel_->disableAll();
    }
}

//读缓冲区
void Connection::handleRead(){

    int saveErrno = 0;
    auto n = inputBuffer_.readFd(fd(), &saveErrno);
    if(n>0){
        messageCallback_(shared_from_this(), &inputBuffer_); 
        
         //这是一个回调函数，在主程序中设置 ：server_.setMessageCallback([this](const ConnectionPtr& conn, Buffer* buf) {onMessage(conn,buf); });
        //成功读入，服务端就执行指定的函数
    }
    else if(n == 0){
        handleClose();
    }
    else{
        printf("readFd error\n");
    }
}

//写缓冲区

void Connection::handleWrite(){

    if(!channel_->isWrite()){
        printf("Connection fd = %d is down, no more writing\n", channel_->Fd());
    }

    auto n = ::write(fd(), outputBuffer_.peek(), outputBuffer_.readableBytes());

    if(n > 0){

        outputBuffer_.retrieve(n);  //更新buffer下标

        if(outputBuffer_.readableBytes() == 0){
            channel_->disableWriting();
        }
        else{
            printf("not all data is writen\n");
        }
    }
    else{
        printf("handleWrite error\n");
    }



}

void Connection::handleClose(){
    assert(state_ == StateE::KConnected || state_ == StateE::KDisconnecting);

    setState(StateE::KDisconnected);
    channel_->disableAll();
    
    ConnectionPtr guardThis(shared_from_this());
    printf("关闭连接后调用 查看智能指针的引用计数 user_count = %ld\n", guardThis.use_count());
    closeCallback_(guardThis);
}