#pragma once
#include<vector>
#include<memory>
#include<atomic>
#include<thread>
#include<mutex>
#include <sys/eventfd.h>
#include<functional>
#include"timestamp.h"
#include"Callbacks.h"

class TimerQueue;
class Channel;
class Epoll;

class EventLoop
{
public:
	using Functor = std::function<void()>;
	using channelList=std::vector<Channel*>;
public:
	EventLoop();
	~EventLoop();

	void loop();
	void updateChannel(Channel* channel);
	void removeChannel(Channel* channel);

	bool isInLoopThread()const { return threadId_ == std::this_thread::get_id(); }
	void runInLoop(Functor cb);
	void queueInLoop(Functor cb);
		// ��������loop�����߳�  ��wakeupfd_дһ�����ݣ�wakeupChannel�ͷ������¼�����ǰloop�߳̾ͻᱻ����
	void wakeup();

	std::thread::id getThreadId(){ return threadId_; }

	void quit();	//�˳��¼�ѭ��

	void assertInLoopThread();

	int64_t runAt(Timestamp time, TimerCallback cb);
	int64_t runAfter(double delay_seconds, TimerCallback cb);
	int64_t runEvery(double interval_seconds, TimerCallback cb);
	void cancel(int64_t timerId);

private:
	void doPendingFunctors();	//ִ������ص�����
	void handleRead();	//����wake up



	std::thread::id threadId_;
	std::atomic_bool quit_;	//��־�˳�loopѭ��

	std::atomic_bool callingPendingFunctors_;   //��ʶ��ǰloop�Ƿ�����Ҫִ�еĻص�����

	std::unique_ptr<Epoll> ep_;
	channelList activeChannels_;	//���浱ǰ��Ծ�¼���Channel�б�

	int wakeupFd_;
	std::unique_ptr<Channel> wakeupChannel_;
	std::unique_ptr<TimerQueue> timer_queue_;
	std::vector<Functor> pendingFunctors_;  //�洢loop��Ҫִ�е����лص�����
	std::mutex mutex_;  // ��������������������vector�������̰߳�ȫ����
};