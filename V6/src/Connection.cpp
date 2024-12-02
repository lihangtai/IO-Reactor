#include"Connection.h"
#include"EventLoop.h"
#include"Channel.h"
#include"Socket.h"
#include"util/util.h"
#include<thread>
#include<iostream>
Connection::Connection(EventLoop* loop, int sockfd, const InetAddr& loaclAddr, const InetAddr& peerAddr)
	:loop_(loop)
	, state_(StateE::kConnecting)
	, socket_(std::make_unique<Socket>(sockfd))
	, channel_(std::make_unique<Channel>(loop, sockfd))
	,localAddr_(loaclAddr)
	,peerAddr_(peerAddr)
{
	
	channel_->SetReadCallback([this]() {handleRead(); });
	channel_->SetWriteCallback([this]() {handleWrite(); });
	channel_->setCloseCallback([this]() {handleClose(); });
	channel_->setErrorCallback([this]() {handleError(); });
}

Connection::~Connection()
{
	printf("Connection::dtor at  fd=%d  state=%d\n", channel_->Fd(), static_cast<int>(state_));
}
void Connection::send(Buffer* message)
{
	send(message->peek(), message->readableBytes());
	message->retrieveAll();
}
void Connection::send(const char* message, size_t len)
{
	if (state_ == StateE::kConnected) {
		if (loop_->isInLoopThread()) {
			sendInLoop(message, len);
		}
		else {
			loop_->runInLoop([this,message,len]() {sendInLoop(message, len); });
		}
	}
}
void Connection::send(const std::string& message)
{
	send(message.data(), message.size());
}

void Connection::shutdown()
{
	if (state_ == StateE::kConnected) {
		setState(StateE::kDisconnecting);
		
		loop_->runInLoop([this]() { shutdownInLoop(); });
	}
}
void Connection::shutdownInLoop()
{
	if (!channel_->isWrite())  
	{
		
		sockets::shutdownWrite(fd());   
	}
}
void Connection::forceClose()
{
	if (state_ == StateE::kConnected || state_ == StateE::kDisconnecting)
	{
		setState(StateE::kDisconnecting);
		//handleClose();
		
		loop_->queueInLoop([this]() { shared_from_this()->forceCloseInLoop(); });
	}
}

void Connection::forceCloseInLoop()
{
	loop_->assertInLoopThread();
	if (state_ == StateE::kConnected || state_ == StateE::kDisconnecting)
	{
		setState(StateE::kDisconnecting);
		handleClose();
	}
}
void Connection::connectEstablished()
{
	assert(state_ == StateE::kConnecting);
	setState(StateE::kConnected);
	channel_->tie(shared_from_this());
	channel_->enableReading();
	connectionCallback_(shared_from_this());	
}


void Connection::connectDestroyed()
{
	if (state_ == StateE::kConnected) {	
		setState(StateE::kDisconnected);
		channel_->disableAll();

		connectionCallback_(shared_from_this());
	}
	
	channel_->remove();

}


void Connection::handleRead()
{
	int savedErrno = 0;
	auto n = inputBuffer_.readFd(fd(), &savedErrno);
	if (n > 0) {
		messageCallback_(shared_from_this(), &inputBuffer_);	//������û����úõĺ���
	}
	else if (n == 0) {
		//��ʾ�ͻ��˹ر�������
		handleClose();
	}
	else {
		handleError();
	}
}
void Connection::handleWrite()
{
	if (!channel_->isWrite()) {
		printf("Connection fd = % d is down, no more writing\n", channel_->Fd());
		return;
	}

	auto n = ::write(fd(), outputBuffer_.peek(), outputBuffer_.readableBytes());
	if (n > 0) {
		//����readerIndex
		outputBuffer_.retrieve(n);
		if (outputBuffer_.readableBytes() == 0) {
			channel_->disableWriting();

		/*	if (state_ == StateE::kDisconnecting) {
				shutdownInLoop();
			}*/
		}
		else {
			printf("read to write more data\n");
		}
	}
	else {
		printf("handleWrite error\n");
	}
}
void Connection::handleClose()
{
	assert(state_ == StateE::kConnected || state_ == StateE::kDisconnecting);
	setState(StateE::kDisconnected);
	channel_->disableAll();

	ConnectionPtr guardThis(shared_from_this());
	connectionCallback_(guardThis);		//�����û������connectionCallback_����

	closeCallback_(guardThis);	//closeCallback_����Server::removeConnection()����
}

void Connection::handleError()
{
	int err = sockets::getSocketError(channel_->Fd());
	printf("Connection::handleError() error=%d\n", err);
}

void Connection::sendInLoop(const char* message, size_t len)
{
	if (state_ == StateE::kDisconnected) {
		printf("disconnected, give up writing\n");
		return;
	}

	bool faultError = false;
	ssize_t nwrote = 0;
	size_t reamining = len;
	
	if (!channel_->isWrite() && outputBuffer_.readableBytes() == 0) {
		nwrote = ::write(fd(), message, len);
		if (nwrote >= 0) {
			reamining = len - nwrote;
			if (reamining == 0) {
				
				if (writeCompleteCallback_) {
					writeCompleteCallback_(shared_from_this());
				}
			}
		}
		else {	//nwrote<0
			nwrote = 0;
			if (errno != EWOULDBLOCK) {
				if (errno == EPIPE || errno == ECONNRESET) {
					faultError = true;
				}
			}
		}
	}

	if (!faultError && reamining > 0) {
		outputBuffer_.append(static_cast<const char*>(message) + nwrote, reamining);
		if (!channel_->isWrite()) {
			channel_->enableWriting();
		}
	}
}