/*

增加应用层IO buffer    server中有两种对象  1. TCP connections: 有状态  2. acceptor负责接收连接的

主体类并没有发生改变，但增加了实现的细节 （例如buffer，区分连接类型）

*/ 

#include "src/Server.h"
#include <stdio.h>
#include <functional>
#include <thread>

void onMessage(const ConnectionPtr& conn, Buffer* buf){

    std::string msg(buf->retrieveAllAsString());
    printf("onMEssage() %ld bytes received:%s\n", msg.size(), msg.c_str());

    conn->send(msg);

}
//这是一个回调函数，读取连接中缓冲区的数据，然后又重新发送出去


int main(){

    InetAddr serverAddr(8888);
    EventLoop loop;    
    //EventLoop -> Epoll 创建了一个epoll_event数组，用于记录收到的IO请求

    Server server(serverAddr, &loop);
    //创建负责接收连接的acceptor和记录连接信息(Connection类）的map容器
    //loop户会透传给每个类
    //Socket -》acceptor -》Channel（用于设置epoll的event)  
    //server 启动了acceptor， 设置了接收到新连接的回调函数（开启一个新的connection）
    server.setMessageCallback([=](const ConnectionPtr& conn, Buffer* buf){onMessage(conn, buf);});
    //设置了server中 acceptor中新连接回调函数内的收到消息的回调函数

    loop.loop();

    //要注意loop中收到IO时间之后，才会主动去调用连接的处理函数
    //而这个实现是在channel中的属性  明天需要搞懂是如何调用的那个回调函数
}


/*
重点：如何设置的回调函数？  本程序设置了收到新连接的回调函数，以及收到消息的回调函数

example
std::function<int> xx  -> std::function<int> = std::move(xx)
std::function:一个用于存储函数，lambda，函数绑定对象的模板类  



*/