#include"Channel.h"
#include"EventLoop.h"
#include"../src/log/logger.h"
#include <sstream>

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
	else {//���else�����������������ӵģ���Ϊ��ʼ�������ӵ�ʱ��tied_��false,�����ӽ�����ʼͨ��tied_��Ϊtrue
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
	//LOG_TRACE << reventsToString();
	LOG_INFO << reventsToString();

	//���¼�Ϊ����û�пɶ��¼�ʱ
	if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
		if (closeCallback_) {
			LOG_DEBUG << "channel closeCallback";
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





std::string Channel::reventsToString() const
{
	return eventsToString(fd_, revents_);
}

std::string Channel::eventsToString() const
{
	return eventsToString(fd_, events_);
}

std::string Channel::eventsToString(int fd, int ev)
{
	std::ostringstream oss;
	oss << fd << ": ";
	if (ev & EPOLLIN)
		oss << "IN ";
	if (ev & EPOLLPRI)
		oss << "PRI ";
	if (ev & EPOLLOUT)
		oss << "OUT ";
	if (ev & EPOLLHUP)
		oss << "HUP ";
	if (ev & EPOLLRDHUP)
		oss << "RDHUP ";
	if (ev & EPOLLERR)
		oss << "ERR ";

	return oss.str();
}