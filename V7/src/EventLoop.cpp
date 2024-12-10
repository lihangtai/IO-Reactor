#include "EventLoop.h"
#include "Channel.h"
#include "Epoll.h"
#include <signal>

int createEventfd(){
    int eventfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(eventfd < 0){
        perror("eventfd");
    }
    return eventfd;
}

class IgnoreSigPipe
{
public:
	IgnoreSigPipe()
	{
		::signal(SIGPIPE, SIG_IGN);
	}
};

IgnoreSigPipe initObj;

EventLoop::EventLoop()
    : threadId_(std::this_thread::get_id()),
    , quit(false)
    , callingPendingFunctors_(false)
    , ep_(std::make_unique<Epoll>())
    , WakeupFd_(createEventfd())
    , wakeupChannel_(std::make_unique<Channel>(WakeupFd_))
    , currentActiveChannel_(nullptr)
{
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    wakeupChannel_->enableReading();
}
EventLoop::~EventLoop()
{
	wakeupChannel_->disableAll();
	wakeupChannel_->remove();
	::close(wakeupFd_);
}

void EventLoop::loop()
{
	quit_ = false;
	while (!quit_) {
		activeChannels_.clear();
		ep_->Epoll_wait(activeChannels_,10000);
		for (auto& active : activeChannels_) {
			active->handleEvent();
		}

		//执行当前EventLoop事件循环需要处理的回调任务操作
		doPendingFunctors();	
	}
}

void EventLoop::updateChannel(Channel* channel)
{
	ep_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
	ep_->del(channel);
}

void EventLoop::assertInLoopThread()
{
	if (!isInLoopThread()) {
		LOG_ERROR << "not in this loopThread";
		//会出错，现在还没有处理
	}
}

void EventLoop::runInLoop(Functor cb)
{
	if (isInLoopThread()) {
		cb();
	}
	else {
		queueInLoop(std::move(cb));
	}
}

void EventLoop::queueInLoop(Functor cb)
{
	{
		std::lock_guard<std::mutex> lock(mutex_);
		pendingFunctors_.push_back(std::move(cb));
	}

	if (!isInLoopThread() || callingPendingFunctors_) {
		wakeup();
	}
}

void EventLoop::wakeup()
{
	uint64_t one = 1;
	ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
	if (n != sizeof(one)){
		LOG_DEBUG<< "EventLoop wakeup write "<< n <<" bytes instead of 8";
	}
}

void EventLoop::handleRead()
{
	uint64_t one = 1;
	auto n = ::read(wakeupFd_, &one, sizeof(one));
	if (n != sizeof(one)){
		LOG_INFO << "EventLoop::handleRead() reads " << n << " bytes";
	}
}

void EventLoop::doPendingFunctors()	
{
	std::vector<Functor> functors;
	callingPendingFunctors_ = true;
	// 把functors转移到局部的functors，这样在执行回调时不用加锁。不影响mainloop注册回调
	{
		std::lock_guard<std::mutex> lock(mutex_);
		functors.swap(pendingFunctors_);
	}

	for (const auto& functor : functors) {
		functor();	//执行当前loop需要执行的回调操作
	}

	callingPendingFunctors_ = false;
}
//在给定的时间执行定时器
int64_t EventLoop::runAt(Timestamp time, TimerCallback cb)
{
	return timer_queue_->addTimer(std::move(cb), time, 0.0);
}
//在给定的时间间隔后执行定时器
int64_t EventLoop::runAfter(double delay_seconds, TimerCallback cb)
{
	Timestamp time(addTime(Timestamp::now(), delay_seconds));
	return runAt(time, std::move(cb));
}
//每个一个时间间隔就执行一次定时器
int64_t EventLoop::runEvery(double interval_seconds, TimerCallback cb)
{
	Timestamp time(addTime(Timestamp::now(), interval_seconds));
	return timer_queue_->addTimer(std::move(cb), time, interval_seconds);
}

void EventLoop::cancel(int64_t timerId)
{
	return timer_queue_->cancel(timerId);
}