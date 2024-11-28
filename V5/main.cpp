#include "src/Server.h"
#include <stdio.h>
#include <functional>
#include <thread>
#include <memory>
#include <iostream>
#include "src/ThreadPool.h"

using namespace std;


 // 单Reactor + 线程池的模型：
//  epoll会监控提供服务的acceptor fd的事件（新的连接），也会接收到已建立连接的fd的各种event



class ComputerServer{

public:
    ComputerServer(EventLoop* loop, const InetAddr& listenAddr, int numThreads)
    : server_(listenAddr, loop)
    , numThreads_(numThreads)
    
    {
        server_.setConnectionCallback([this](const ConnectionPtr& conn){onConnection(conn);});
        server_.setMessageCallback([this](const ConnectionPtr& conn, Buffer* buf){onMessage(conn, buf);});
    }

    void start(){

        threadPool_.start(numThreads_);  
        server_.start();
    
    }

private:

    void onConnection(const ConnectionPtr& conn){
        
        std::cout << conn->peerAddress().toIpPort() << "->" << conn->localAddress().toIpPort() << " is " << (conn->connected() ? "UP" : "DOWN") << std::endl;       
    }

    void onMessage(const ConnectionPtr& conn, Buffer* buf){

        threadPool_.add([&conn, buf](){
            int nums = stoi(buf->retrieveAllAsString());
            long long sum = 0;
            for(int i=0; i< nums; ++i)
                sum+=i;  //计算 0 到 nums -1 的总和
            cout << "thradPool" << std::this_thread::get_id()<<endl;
            conn->send(to_string(sum));
        });
    }

    Server server_;
    ThreadPool threadPool_;
    int numThreads_;

};
int main(int argc, char* argv[]){
    
    /*
    流程总结：
    Server初始化acceptor对象后，将acceptor的fd加入到了epoll监控的内核事件表中，
    当在epoll的循环中有新的连接时会进行接收accept，将其添加到epoll的感兴趣列表中 
    将接收后连接fd加入Eventloop监听，当有新连接fd可读时，


    循环接收逻辑： Epoll -》 Channel -》 EventLoop  （操作底层对象fd，及其对应的Event和处理事件的回调函数）
    创建Server
    */


    EventLoop loop;
    /*
     Eventloop类初始化了epoll，和 Channel数据容器（用与接收每次epoll_wait的返回列表）

     这个类:
     1. Epoll成员：调用系统调用epoll_create()初始化了一个epoll，用于后续通过IO多路复用监控的interesting fd lists （Epoll类负责封装epoll相关的系统调用，操作的对象类型为Channel，每次操作需要更新到对应的Channel对象中）
     2. 使用了Channel类：用于设置fd相关的事件，设置不同事件会调用的回调函数  
     3. channelList成员：创建了一个Channel类型的vector channelList, 用于存放epoll_wait捕捉到感兴趣fd的指定事件

    */ 
        

    InetAddr listenAddr(10000);
    /*
    创建了一个网络地址对象，用于在10000端口监听，IPV4的任何IP地址（INADDR_ANY）的请求
    */
    

    ComputerServer server(&loop, listenAddr, 4);
    /*
     ComputerServer初始化一个Server（包含acceptor类对象+ connection类的连接容器），一个线程池threadPool_对象
     设置了Server建立新连接和收到消息时调用的回调函数

     最重要：
     目前：acceptor仅处于bind阶段，尚未listen，accept 
     EPOLLIN代表有新的客户想要建立连接，所以EPOLLIN对应回调函数逻辑是
    1. accept建立连接，
    2. 调用server设置的newconnectioncallback函数，创建新的connection类对象并添加入server的容器中，
    在也将在构造函数中设置该channel的各情况回调函数，最终调用connection的connectEstablished函数，注册进入epoll  

     Server：
     1. 创建Acceptor类对象：创建socket连接（以及fd对应的Channel），bind（监听指定的listenAddr（10000+any）），设置Channel的读回调函数为accept系统调用
     2. 设置Acceptor对象建立新连接connection回调函数（fd置为非阻塞，创建新的connection对象，设置对象Channel的收到/写/开启/关闭连接的回调函数）
     3. 创建ThreadPool类对象：start函数循环中创建线程存储到线程池容器(threads_)中，它们同时执行runInThread函数，在循环中通过抢占锁和满足条件变量来获取任务队列Task中的任务，并执行     （move函数把需要运行的函数传入 std::function<void()>中


     其中需要注意的设计是：每次去调用connection类的send函数发送数据时，send会去调用write(fd),如果有部分内容没有发完则把这部分存到output buffer中
     但只看这个函数实现的时候可能会有疑问，什么时候会将buffer中的数据发出呢？
     在connection对象中的channel成员对象中注册对于这个fd的当epoll监听到EPOLLOUT事件的时候，去调度的回调函数处理逻辑
     其实就是把buffer中的数据再次write
    */
    server.start();

    /*
       1. 并行开始：threadpool对象创建线程，多个线程开始抢夺任务队列中的任务（通过互斥量和条件变量），然后执行  （目前暂无任务）
       2. acceptor将自身fd的event设置有数据可读

       其中：Acceptor fd开始listen，并将这个fd 关注的EPOLLIN事件通过Channel注册到Evenloop中，最终注册到epoll中
       Acceptor设置的回调函数是newConnection,当有新的请求时，会接收连接并创建对应的Connection对象，并且设置各种状态下的处理回调函数
       新连接的回调函数在server中进行设置
    */

    loop.loop();
    /*
    循环进行调用epoll_wait，对捕捉到的Channel，调用它对应回调函数进行处理 
    而在V5中每个Channel的中EPOLLIN回调函数触发的时候，会把任务假如到线程池中负责执行
    */

    return 0;
}