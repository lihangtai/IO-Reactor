# 设计思路
单Reactor设计通过一个while循环来执行，如果处理函数特别耗时则可能阻塞其他IO的响应
``` while(1){
    int nums=epoll_wait(...);
    for(int i=0;i<nums;++i){
        channel[i]->handleEvent();
    }    
}
```

solution: reactor + threadpool，充分利用多核cpu -》线程池

> 所以我们可以使用固定大小的线程池，全部的IO工作（即是epoll_wait(),read(),send() 这些调用等等） 都在Reactor线程完成，而计算任务或者比较耗时的任务就交给thread pool。下图是其程序流程结构。

![](https://i-blog.csdnimg.cn/blog_migrate/777a6da9b4235a6c7032e33a53e2a9ce.webp?x-image-process=image/format,png)




## 抽象逻辑

最基础的类：  Socket,InetAddr，EPOLL

以文件描述符为索引： 
Epoll（注册fd） 提供方法操作 ->  
Channel (设置在epoll注册的fd的属性，设置读写关闭的回调函数，与另一个**对象绑定) 提供使用->  
Connection(设置一个连接（channel/fd）读写关闭的回调函数，设置fd的属性，绑定channel) 实现中包含读写的用户态缓冲区，，每个连接会对应不同的读写/关闭/错误的实现逻辑    设置Channel的回调函数，同时也为上层的创建提供接收回调函数的接口



运行状态机:
EventLoop (拥有所有活跃的Channel的容器，实例化Epoll，主要执行loop函数逻辑阻塞10微秒) loop使用Epoll.wait 循环调用收到Channel list中相应的回调函数







### 需要review的知识：
> 水平触发和边缘触发的区别
> Level-Triggered (LT) 示例：即使事件已经被处理，epoll_wait 也会持续报告可读事件，直到所有数据被读取完毕。这意味着，如果处理不够快或数据没有完全读取完毕，可能会多次报告事件。

> Edge-Triggered (ET) 示例：epoll_wait 只会报告事件一次，之后需要应用程序处理所有的数据。处理不完全会导致遗漏，因为事件不会再被报告直到状态发生变化（例如数据被完全读取后又有新数据到达）。



### 需要关注的语法：
1. channel中的 weak_ptr 和 shared_ptr

2. 回调函数如何写最好
>std::function 接收   std::move 传递参数给function

3. 多源程序和头文件防止重复包含