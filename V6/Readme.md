# 相对于V5（单Reactor + 线程池模型）的一些优化和更新

1. 跨线程调度能力
在V5中EventLoop对象中循环使用epoll_wait监听感兴趣fd列表的指定事件，监听到事件后将处理函数分发给到线程池进行处理。 这样可能出现一个线程在使用epoll_wait监听fd，而另一个线程池中的线程在读写fd

这样容易出现竞态条件，造成处理逻辑错误。
正确的设计应该为：复杂运算由线程池进行处理，但是对于一个IO的监听和读写应该集中在同一个线程中
所以线程池中的线程执行完运算后，要具有跨线程调度的能力，去主线程填入处理逻辑，主线程通过注册一个**eventfd**进入epoll，从而在循环中唤醒epoll_wait完成IO读写操作。

> 可以使用互斥锁或者eventfd等机制
eventfd:用于跨线程间的轻量化同步通信 （内核管理的一个uint64整数）


``` 
Eventloop中加入一个pendingFunctor函数容器，每次循环中查看时候有需要执行的IO操作 （connection触发回调之后最后的IO事件加入到这个容器中进行执行）

void EventLoop::loop()
{
	quit_ = false;
	while (!quit_) {
		activeChannels_.clear();
		ep_->Epoll_wait(activeChannels_);
		for (auto& active : activeChannels_) {
			active->handleEvent();
		}
        
		//就是添加了这一句，执行当前EventLoop事件循环需要处理的回调任务操作
		doPendingFunctors();
	}
}



2. 主从Reactor + threadpool模型

![](https://i-blog.csdnimg.cn/blog_migrate/3466668d7967bb8bdd9999915629e685.png)

> 单Reactor模型：即要管理新的连接，也要管理对已经建立连接的对端进行读写操作，如果IO压力比较大可能会处理不及时。

升级为：主从Reactor + threadpool模型
主Readctor负责与客户端建立连接，其他的Reactor负责与客户端IO通信,threadpool负责进行计算任务