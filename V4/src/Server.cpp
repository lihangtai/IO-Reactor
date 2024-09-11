#include "Server.h"
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

void setNonblock(int sockfd){

    int flag = fcntl(sockfd, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(sockfd , F_SETFL, flag);

}

Server::Server(const InetAddr& listenAddr, EventLoop* eventloop)
    :loop_(eventloop)
    ,acceptor_(std::make_unique<Acceptor>(listenAddr, loop_))
{

    auto cb = [this](int sockfd){newConnection(sockfd);};
    acceptor_->setNewconnectionCallback(cb);
    //设置了acceptor的回调函数了

}

Server::~Server(){

    for(auto& item: connections_){
        ConnectionPtr conn(item.second);
        item.second.reset();
        conn->connectDestroyed();
    }

   

}

void Server::newConnection(int sockfd){

    setNonblock(sockfd);

    auto conn = std::make_shared<Connection>(loop_, sockfd);
    printf("new connection created: user_count = %ld\n", conn.use_count());
    connections_[sockfd] = conn; //加入server的 map中
    conn->setMessageCallback(messagCallback_);
    conn->setCloseCallback([this](const ConnectionPtr& conn){removeConeection(conn);});

}

void Server::removeConeection(const ConnectionPtr& conn){

    auto n = connections_.erase(conn->fd());

    assert(n == 1);
    conn->connectDestroyed();
}