#pragma once
#include<functional>
#include<unistd.h>
#include<fcntl.h>
#include<memory>
#include"Epoll.h"
class EventLoop;

class Channel
{
public:
	using EventCallback = std::function<void()>;

public:
	Channel(EventLoop* loop, int fd);
	~Channel() = default;
	void setEvents(int events);
	int Event()const;
	void setRevents(int events);
	int Revent()const;

	bool isInEpoll();
	void setInEpoll(bool in);
	int Fd()const;

	void SetReadCallback(EventCallback cb) { readCallback_ = std::move(cb); }
	void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
	void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
	void setErrorCallback(EventCallback cb){ errorCallback_ = std::move(cb); }

	void enableReading() { events_ |= (EPOLLIN | EPOLLPRI); update(); }	//ע��ɶ��¼�
	void disableReading() { events_ &= ~(EPOLLIN | EPOLLPRI); update(); }	//ע���ɶ��¼�
	void enableWriting() { events_ |= EPOLLOUT; update(); }			//ע���д�¼�
	void disableWriting() { events_ &= ~EPOLLOUT; update(); }		//ע����д�¼�
	void disableAll() {events_ = 0; update();}	//ע�������¼�

	//����fd��ǰ���¼�״̬
	bool isNoneEvent()const { return events_ == 0; }
	bool isWrite()const {return events_ & EPOLLOUT; }
	bool isRead()const {return events_& (EPOLLIN | EPOLLPRI); }

	void handleEvent();
	void remove();

	void tie(const std::shared_ptr<void>&);

	//for debug
	std::string reventsToString() const;
	std::string eventsToString() const;
private:
	void update();
	void handleEventWithGuard();

	static std::string eventsToString(int fd, int ev);	//��Ҫ��static,��Ϊ�Ǹ�reventsToString() constʹ��

private:
	EventLoop* loop_;

	int fd_;
	int events_;
	int revents_;
	bool isInEpoll_;

	std::weak_ptr<void> tie_;
	bool tied_;
	
	EventCallback readCallback_;
	EventCallback writeCallback_;
	EventCallback closeCallback_;
	EventCallback errorCallback_;
};