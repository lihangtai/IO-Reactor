#include"Channel.h"
#include"EventLoop.h"


Channel::Channel(EventLoop* loop, int fd)
	:loop_(loop)
	, fd_(fd)
	, events_(0)
	, revents_(0)
	, isInEpoll_(false)
	,tied_(false)
{
}

void Channel::setEvents(int events)
{
	events_ = events;
}
int Channel::Event()const
{
	return events_;
}
void Channel::setRevents(int revents)
{
	revents_ = revents;
}
int Channel::Revent()const
{
	return revents_;
}

bool Channel::isInEpoll()
{
	return isInEpoll_ == true;
}

void Channel::setInEpoll(bool in)
{
	isInEpoll_ = in;
}

int Channel::Fd()const
{
	return fd_;
}


void Channel::handleEvent()
{
	if (tied_) {
		std::shared_ptr<void> guard = tie_.lock();
		if (guard) {
			handleEventWithGuard();
		}
	}
	else {	//���else�����������������ӵģ���Ϊ��ʼ�������ӵ�ʱ��tied_��false,�����ӽ�����ʼͨ��tied_��Ϊtrue
		handleEventWithGuard();
	}
}

void Channel::remove()
{
	loop_->removeChannel(this);
}

void Channel::update()
{
	loop_->updateChannel(this);
}

void Channel::tie(const std::shared_ptr<void>& obj)
{
	tie_ = obj;
	tied_ = true;
}

void Channel::handleEventWithGuard()
{
	if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {//���¼�Ϊ����û�пɶ��¼�ʱ
		if (closeCallback_) {
			printf("channel closeCallback\n");
			closeCallback_();
		}
	}
	if (revents_ & EPOLLERR) {
		if (errorCallback_)
			errorCallback_();
	}
	if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) { //���ڶ����¼� 
		if (readCallback_)
			readCallback_();
	}
	if (revents_ & EPOLLOUT) {   //����д���¼�
		if (writeCallback_)
			writeCallback_();
	}
}