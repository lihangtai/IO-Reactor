/*

增加应用层IO buffer    server中有两种对象  1. TCP connections: 有状态  2. acceptor负责接收连接的

主体类并没有发生改变，但增加了实现的细节 （例如buffer，区分连接类型）

*/ 

#include "src/Server.h"
#include <stdio.h>
#include <functional>
#include <thread>





int main(){

    InetAddr serverAddr(8888);
}