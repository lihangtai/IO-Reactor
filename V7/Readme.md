> 在多reactor + 负责任务线程池的基础上再添加：定时器功能，异步日志

## 定时器功能

需要的elements：时间timestamp（对真实时间的量化）， 定时器timer（间隔时间超时回调函数，是否重复），定时器容器（用一个fd高效增减管理多个timer） 


### 对于时间戳使用的是：std::chrono::steady_clock (操作系统时间)
```
std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
```


### 对于定时器
不能让定时器阻塞Reactor的执行, 触发后调用fd对应的回调函数

1. 定时函数的选择：有的函数基于信号，有的会阻塞整个线程，为了让整个功能能够保持响应
使用timefd_ 因为把时间当作了一个文件描述符，超时的一瞬间文件变得可读，可以更好的match epoll

timerfd_create ：把时间变成了一个文件描述符，该“文件”在定时器超时的那一刻变得可读，这样就能很方便地融入到 select/poll 框架中，用统一的方式来处理 IO 事件和超时事件，这也正是 Reactor 模式的长处。

timerfd_settime ：设置定时器，设置定时器超时后触发的回调函数，设置定时器是否重复触发. 相对于当前时间或者指定绝对时间，可以设置重复

> timerfd_create 创建的文件描述符是用于管理单个定时器的，最后一次调用的设置会覆盖之前的设置
定时器触发 epoll_wait之后需要read这个文件描述符，否则持续会持续发送EPOLLIN


### timer容器

***重点是容器的数据结构选择***
容器里存放的是timer是：确定的时间点（不是时间间隔），是否需要重复触发，以及对应的EPOLLIN回调函数


总结-》该高效的增删管理的timer容器对象的设计思想是：1.使用了timerfd 加入到了epoll的循环处理中，并且由于timerfd_setter每次只能执行一个定时器功能，该类维护了时间容器，每次定时器expire都会进行比较选出最近的时间作为下一个定时器开启 
2.使用三个容器维护了功能：timer容器，active_timer容器（高效查），和unactive_timer容器
3. 每个容器里的对象在注册的时候就声明了EPOLLIN触发的时候会执行的函数逻辑

使用非常方便:直接声明时间+需要执行的逻辑
``` 
	loop.runAfter(1, []() {myprint("once1"); });
	loop.runAfter(1.5, []() {myprint("once1.5"); });
	loop.runAfter(2.5, []() {myprint("once2.5"); });
	loop.runAfter(4.8, []() {myprint("once4.8"); });
	loop.runEvery(2, []() {myprint("every2"); }); 
```

## 异步LOG系统 

设计与实现：实现一个线程安全，不能让LOG阻塞Reactor的执行
前台（应用）：像使用stdout一样轻松地打印日志
后台线程负责在日志信息积累到一定量/到达时间间隔，持久化到磁盘
> stdout 并不是线程安全的


NOTE: 由于***设计比较复杂***，分为3个阶段设计进行理解


阶段一：

Log << "hello world"  (调用宏创建类对象)  -》 LOG用户态缓冲区 （填满/定时刷新） -》 对象生命周期结束**析构时，输出到stdout中**

![](https://i-blog.csdnimg.cn/blog_migrate/f1368e15eada8a7cbd5d0f16eb13528a.png)


阶段二：
日志需要持久化到文件中，定期刷新/文件满了需要新开文件
前台日志输出与后端log线程通过条件变量进行同步通信

前台与后台都使用双缓冲技术：
 前台：拥有 currentbuffer 和 nextbuffer 和 buffers（属于类成员属性）  
 有空间存进去 | 没空间把currentbuffer存到容器，新建buffer或者把nextbuffer move给currentbuffer，然后继续存  （存完cond通知后台线程，目前容器和currentbuffer中有数据）
 
 后台线程：拥有两个buffer和一个btw容器（类成员函数内部局部变量）  等待cond或一定周期后继续 
 将currentbuffer存入buffers容器，为currentbuffer，nextbufffer替换为局部buffer，交换空容器btw和buffers，将btw容器中的数据持久化到磁盘 （再将两个buffer置空）

![](https://i-blog.csdnimg.cn/blog_migrate/721aef4d82017f169371edbfee5693bc.png)

问题：对于使用者，每次调用宏写入日志都涉及到多个对象的创建和销毁，开销不大吗
> 如果使用了异步日志（AsyncLogger），那么每次日志记录都会启动新线程，线程的创建和销毁本身就带来额外的性能开销，尤其是在高并发环境下，频繁的线程创建和销毁会导致 CPU 上下文切换、资源竞争等问题。

！是的，就是那么蠢 （false）
LOG就是一直持续创建和析构， 但在第一次析构的时候创建了静态变量对象（异步IO对象+一个线程，生命周期为整个程序，后续不会创建std::call_once函数），每次析构的时候都会把buffer中的数据传给静态变量对象

阶段三：
日志文件滚动功能 （防止某个日志文件过大，不同的日志文件使用不同标志的文件名）

多次调用宏创建LOG类对象  -》 操作符重载(<<)持续将数据写入到FIXBUFFER缓存中 -》 LOG对象析构造ing -》 （用std::once_flag + std::call_once限制只创建1次）创建异步IO类对象asynclogger  -》 start成员函数开启后端文件IO线程 (死循环，按照预设条件做持久化)   -》 LOG将FIXBUFFER缓冲写入到asynclogger  -》 满足条件触发后端IO线程的flush函数，将asynclogger中的数据持久化到磁盘 （此处使用双缓冲技术） -》 程序结束

![](https://i-blog.csdnimg.cn/blog_migrate/307e9c2a9f9915dfb31d17fabb540ee5.png)


使用非常方便:第一次使用宏创建对象的时候就会初始化异步IO对象（静态变量），后面使用宏，对象在析构的时候都会把数据写入到异步IO对象中，当满足条件时，异步IO对象会触发后端IO线程的flush函数，将数据持久化到磁盘
```
LOG_INFO << "1111woshisdfsd " << 23 << 34 << "buox";
```