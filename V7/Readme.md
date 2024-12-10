> 在多reactor + 负责任务线程池的基础上再添加：定时器功能，异步日志

## 定时器功能

1. 定时函数的选择：有的函数基于信号，有的会阻塞整个线程，为了让整个功能能够保持响应
使用timefd_ 因为把时间当作了一个文件描述符，超时的一瞬间文件变得可读，可以更好的match epoll

sleep / alarm / usleep：在实现时有可能用了信号 SIGALRM，在多线程程序中处理信号是个相当麻烦的事情，应当尽量避免。

nanosleep 和 clock_nanosleep： 是线程安全的，但是在非阻塞网络编程中，绝对不能用让线程挂起的方式来等待一段时间，程序会失去响应。正确的做法是注册一个时间回调函数。

getitimer 和 timer_create：也是用信号来 deliver 超时，在多线程程序中也会有麻烦。timer_create 可以指定信号的接收方是进程还是线程，算是一个进步，不过在信号处理函数(signal handler)能做的事情实在很受限。

timerfd_create ：把时间变成了一个文件描述符，该“文件”在定时器超时的那一刻变得可读，这样就能很方便地融入到 select/poll 框架中，用统一的方式来处理 IO 事件和超时事件，这也正是 Reactor 模式的长处。


## 异步LOG系统 
设计与实现：实现一个线程安全，LOG功能不影响功能逻辑。
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