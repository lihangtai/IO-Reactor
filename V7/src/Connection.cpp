#include"Connection.h"
#include"EventLoop.h"
#include"Channel.h"
#include"Socket.h"
#include"util/util.h"
#include<thread>
#include<iostream>
#include"../src/log/logger.h"

Connection::Connection(EventLoop* loop, int sockfd, const InetAddr& loaclAddr, const InetAddr& peerAddr)
	:loop_(loop)
	, state_(StateE::kConnecting)
	, socket_(std::make_unique<Socket>(sockfd))
	, channel_(std::make_unique<Channel>(loop, sockfd))
	,localAddr_(loaclAddr)
	,peerAddr_(peerAddr)
{
	//���ú� ����д���ر�����,������ �Ļص�����
	channel_->SetReadCallback([this]() {handleRead(); });
	channel_->setWriteCallback([this]() {handleWrite(); });
	channel_->setCloseCallback([this]() {handleClose(); });
	channel_->setErrorCallback([this]() {handleError(); });
}

Connection::~Connection()
{
	LOG_DEBUG << "Connection::dtor at  fd= " << channel_->Fd() << "%d  state= " << static_cast<int>(state_);
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
		//��ǰ��ֱ�ӵ��õģ�����ʹ��runInLoop()����ȥ����
		loop_->runInLoop([this]() { shutdownInLoop(); });
	}
}
void Connection::shutdownInLoop()
{
	if (!channel_->isWrite())  // ˵����ǰoutputBuffer�е������Ѿ�ȫ���������
	{
		
		sockets::shutdownWrite(fd());   // �ر�д�� ,�ܴ���EPOLLHUP,Ҳ�ᴥ��EPOLLIN
	}
}
void Connection::forceClose()
{
	if (state_ == StateE::kConnected || state_ == StateE::kDisconnecting)
	{
		setState(StateE::kDisconnecting);
		//handleClose();
		//ʹ��queueInLoop��������һ���Ƿ�����������У�������EventLoop::doPendingFunctors()��ִ��forceCloseInLoop()����Ҫʹ��shared_from_this()
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
	connectionCallback_(shared_from_this());	//�����û����õ����ӳɹ���Ͽ��Ļص�����
}

// ��������(�ر����ӵ����һ��)
void Connection::connectDestroyed()
{
	if (state_ == StateE::kConnected) {	//һ�㲻��������if
		setState(StateE::kDisconnected);
		channel_->disableAll();

		connectionCallback_(shared_from_this());//�����û����õ����ӳɹ���Ͽ��Ļص�����
	}
	
	channel_->remove();
	//Conntion��������������������вŻ����(����û�������TcpConnection����)
}


void Connection::handleRead()
{
	int savedErrno = 0;
	auto n = inputBuffer_.readFd(fd(), &savedErrno);
	if (n > 0) {
		//������û����úõĺ���
		messageCallback_(shared_from_this(), &inputBuffer_);
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
		LOG_INFO << "Connection fd = " << channel_->Fd() << "  is down, no more writing";
		return;
	}

	auto n = ::write(fd(), outputBuffer_.peek(), outputBuffer_.readableBytes());
	if (n > 0) {
		//����readerIndex
		outputBuffer_.retrieve(n);
		if (outputBuffer_.readableBytes() == 0) {//����Ҫ���͵�������ȫ��������ϣ�����ȡ��д�¼�
			channel_->disableWriting();

		/*	if (state_ == StateE::kDisconnecting) {
				shutdownInLoop();
			}*/
		}
		else {
			LOG_INFO << "read to write more data";
		}
	}
	else {
		LOG_ERROR << "handleWrite error";
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
	LOG_DEBUG << "Connection::handleError() error=" << err;
}

void Connection::sendInLoop(const char* message, size_t len)
{
	if (state_ == StateE::kDisconnected) {
		LOG_DEBUG << "disconnected, give up writing";
		return;
	}

	bool faultError = false;
	ssize_t nwrote = 0;
	size_t reamining = len;
	//�����ǰchannelû��д�¼����������ҷ��ͻ������޴����͵����ݣ��ǾͿ���ֱ�ӷ���
	if (!channel_->isWrite() && outputBuffer_.readableBytes() == 0) {
		nwrote = ::write(fd(), message, len);
		if (nwrote >= 0) {
			reamining = len - nwrote;
			if (reamining == 0) {
				//��ʾ��������ȫ���ͳ�ȥ��֪ͨ�û�д�����
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