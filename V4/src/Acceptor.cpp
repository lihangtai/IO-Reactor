#include "Acceptor.h"

Acceptor::Acceptor(const InetAddr& listenAddr, EventLoop* eventloop)
    :loop_(eventloop),
    acceptSocket_(Socket()),
    acceptChannel_(loop_, acceptSocket_.fd()),
    listen_(false)
{

    acceptSocket_.bind(listenAddr);

    auto cb = [this](){handleRead();};

    acceptChannel_.setReadCallback(cb);

    this->listen();



}

Acceptor::~Acceptor(){

    acceptChannel_.remove();
}

void Acceptor::listen(){
    
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

//仅在目前目前函数实现中会发生疑惑：newConnection并没有具体的函数实现，传递一个fd给它是什么意思呢
// solution：在main函数中实例化之后会调用setNewconnectionCallback，设置这个对象的回调函数
//而输入的参数是作为回调函数操作的fd
void Acceptor::handleRead(){

    InetAddr peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);
    if(connfd >= 0){

        if(newConnectionCallback_){
            newConnectionCallback_(connfd);  //调用的是Server::newConnection(int sockfd)函数
        }
    }

}

