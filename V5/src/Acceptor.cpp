#include "Acceptor.h"

Acceptor::Acceptor(const InetAddr& listenaddr, EventLoop* eventloop)
    : loop_(eventloop)
    , acceptSocket_(Socket())
    , acceptChannel_(loop_, acceptSocket_.fd())
    , listen_(false)
    {

        acceptSocket_.bind(&listenaddr);
        
        auto cb = [this](){ handleRead();};

        acceptChannel_.SetReadCallback(cb);

    }

    Acceptor::~Acceptor(){
        acceptChannel_.remove();
    }

    void Acceptor::handleRead(){
        InetAddr peerAddr;
        int connfd = acceptSocket_.accept(&peerAddr);
        
    }