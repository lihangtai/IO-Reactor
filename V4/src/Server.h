#pragma once

#include "Channel.h"
#include "Epoll.h"
#include "Socket.h"
#include "util/util.h"
#include "EventLoop.h"
#include "InetAddr.h"
#include <memory>

/* 

下列类为 V4 实现的新类模块  

简单逻辑：服务端负责接收客户端连接的主体是acceptor，连接以后每个fd对应一个connection，同时EventLoop对象管理着整个IO多路复用的执行

acceptor 负责监听端口，负责接受连接  -》channel -》 socket 

connection负责读写数据，读数据以后会调用回调函数 而且有读写缓冲区 -》channel -》 socket   buffer

EventLoop负责控制epoll 来处理 IO多路复用

*/

#include "Acceptor.h"
#include "Buffer.h"
#include "Connection.h"
#include <map>

// Server 包含一个 Acceptor 和 一个 记录所有connection连接的map 

class Server{

    public:
        using connectionMap = std::map<int, ConnectionPtr>;

    public:
        Server(const InetAddr& serverAddr, EventLoop* eventloop);
        ~Server();

        void setMessageCallback(const MessageCallback &cb){

            messagCallback_ = cb;
        }
                
    
    private:

        void newConnection(int sockfd);
        void removeConeection(const ConnectionPtr& conn);

    private:

        EventLoop *loop_;
        std::unique_ptr<Acceptor> acceptor_;  //这里使用智能指针是为了防止重复调用对象
        connectionMap connections_;
        
        MessageCallback messagCallback_;


};