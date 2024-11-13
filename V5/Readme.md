# 设计思路
## 单Reactor设计
通过一个while循环来捕捉新的EPOLL关注事件（其中每次循环可能调用多个处理回调函数） 如果其中存在的处理函数特别耗时则可能阻塞其他IO的响应
> 接收事件为了防止阻塞使用epoll，处理事件也存在阻塞，单reactor并未解决这个问题

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


运行状态机:
EventLoop (拥有所有活跃的Channel的容器，实例化Epoll，主要执行loop函数逻辑阻塞10微秒) loop使用Epoll.wait 循环调用收到Channel list中相应的回调函数







### 需要review的知识：
> 水平触发和边缘触发的区别
> Level-Triggered (LT) 示例：即使事件已经被处理，epoll_wait 也会持续报告可读事件，直到所有数据被读取完毕。这意味着，如果处理不够快或数据没有完全读取完毕，可能会多次报告事件。

> Edge-Triggered (ET) 示例：epoll_wait 只会报告事件一次，之后需要应用程序处理所有的数据。处理不完全会导致遗漏，因为事件不会再被报告直到状态发生变化（例如数据被完全读取部分后又有新数据到达）。



