#include "src/Server.h"
#include <stdio.h>
#include <functional>
#include <thread>
#include <memory>
#include <iostream>
#include "src/ThreadPool.h"
using namespace std;

int main(){

    InetAddr servAddr(10000);
    EventLoop loop;
    Server server(servAddr, &loop); //包含一个Acceptor（base EventLoop) 和 Reactor线程池和 运算线程池
    server.setConnectionCallback([](const ConnectionPtr& conn){
        if(conn->connected()){
            printf("Connection connected ip:port: %s connected../n", conn->peerAddress().toIpPort().c_str());
        }else{
            printf("Connection disconnected/n");
        }

    });

    server.setMessageCallback([](const ConnectionPtr& conn, Buffer* buf){
        std::string msg(buf->retrieveAllAsString());
        printf("onMessage() %ld bytes received:%s\n", msg.size(), msg.c_str());
        conn->send(msg);
    });

    server.start(2); //开启多个reactor线程池 + 计算线程池 （但涉及文件描述符的IO操作依然由reactor负责处理，避免产生冲突）
    loop.loop();

    return 0;
    
}