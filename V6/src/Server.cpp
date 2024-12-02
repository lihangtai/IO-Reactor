#include "Server.h"
#include <string.h>
#include <unistd.h>
#include <fcntl.h>


Server::Server(const InetAddr& listenAddr, EventLoop* eventloop)
    :loop_(eventloop)
    ,ipPort_(listenAddr.toIpPort())
    ,acceptor_(std::make_unique<Acceptor>(listenAddr, loop_))
    ,loop_threadpool_(std::make_unique<EventLoopThreadPool>(loop_))
    ,compute_threadpool_(std::make_unique<ThreadPool>())
    ,started_(0)
    {
        acceptor_->setNewconnectionCallback([this](int sockfd, const InetAddr& peerAddr){newConnection(sockfd, peerAddr);});
    }


Server::~Server(){
    for(auto& item : connections_){
        ConnectionPtr conn(item.second);
        item.second.reset();
        conn->connectDestroyed();

    }
}

void Server::start(int IOThreadNum,int compute_threadNum){

    if(started_++==0){
        
        loop_threadpool_->setThreadNum(IOThreadNum);
        loop_threadpool_->start();  //这个线程池的构造函数仅初始化对象，start函数才正式开始功能，创建reactor线程对象们，把执行逻辑的线程加入到了线程池的thread容器中，把线程eventloop对象属性放入到线程池的eventloop容器中（用条件变量去通知线程池假如成功）然后开启一个线程开始执行loop函数
        compute_threadpool_->start(compute_threadNum);
        acceptor_->listen();

    }
}

void Server::newConnection(int sockfd, const InetAddr& peerAddr){

    EventLoop* ioLoop = loop_threadpool_->getNextLoop();  //从池子中选择一个Reactor线程
    InetAddr localAddr(sockets::getLocalAddr(sockfd));

    auto conn = std::make_shared<Connection>(ioLoop, sockfd, localAddr, peerAddr); //循环遍历，将新的连接绑定到eventloop中
    connections_[sockfd] = conn;

    conn->setMessageCallback(messageCallback_);
    conn->setCloseCallback([this](const ConnectionPtr& conn){removeConnection(conn);});
    conn->setConnectionCallback(connectionCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);

    ioLoop->runInLoop([conn](){conn->connectEstablished();});

}

void Server::removeConnection(const ConnectionPtr& conn){

    loop_->runInLoop([this, conn](){removeConnectionInLoop(conn);});
}

void Server::removeConnectionInLoop(const ConnectionPtr& conn){
    connections_.erase(conn->fd());

    auto ioLoop = conn->getLoop();
    ioLoop->queueInLoop([conn](){conn->connectDestroyed();});

}