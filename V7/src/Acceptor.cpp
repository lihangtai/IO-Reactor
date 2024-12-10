#include "Acceptor.h"
#include "EventLoop.h"


Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
    : loop_(loop)
    , acceptSocket_(Socket())
    , acceptChannel_(loop_, acceptSocket_.fd())
    , listen(false)
{   
    sockets::setReuseAddr(acceptSocket_.fd(), true);
    acceptSocket_.bind(listenAddr);
    acceptChannel_.SetReadCallback([this]() {handleRead(); });
}

Acceptor::~Acceptor()
{
	acceptChannel_.disableAll();
	acceptChannel_.remove();
}

void Acceptor::listen()
{
	acceptSocket_.listen();
	acceptChannel_.enableReading();
	listen_ = true;
}

void Acceptor::handleRead()
{
	InetAddr peerAddr;
	int connfd = acceptSocket_.accept(&peerAddr);
	if (connfd >= 0) {
		if (newConnectionCallback_) {
			newConnectionCallback_(connfd,peerAddr);
		}
	}
	else {
		LOG_ERROR << "accpet error";
	}
}