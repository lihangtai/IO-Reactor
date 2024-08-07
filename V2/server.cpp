#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define PORT 8888
const int SIZE = 1024;

void error_handling(bool condition, const char* message){
    if(condition){
        perror(message);
        exit(1);
    }
    
}

void setNonblocking(int fd){
    int flag = fcntl(fd, F_GETFL);
    flag = flag | O_NONBLOCK;
    fcntl(fd, F_SETFL, flag);
}

void handleReadevent(int cfd, int epfd){

    char buf[SIZE];
    memset(buf, 0, SIZE);

    int len = read(cfd, buf, SIZE);
    error_handling(len == -1, "read");

    if(len > 0){
        printf("the client say: %s", buf);
        write(cfd,"alread received~", strlen("alread received~"));
    }
    else if(len == 0){   //根据阅读为0，说明客户端关闭了连接
        printf("client close\n");
        epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, NULL);
        close(cfd);

    }
    else{
        perror("read");
        exit(0);
    }



}

int main(int argc, char* argv[]){

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    error_handling(fd == -1, "socket");
    setNonblocking(fd);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    int result = bind(fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    error_handling(result == -1, "bind");

    result = listen(fd, 64);
    error_handling(result == -1, "listen");

    int epfd = epoll_create(SIZE);
    error_handling(epfd == -1, "epoll_create");

    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN;
    ev.data.fd = fd;

    result = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
    if(result == -1){
        perror("epoll_ctl");
        close(fd);
        close(epfd);
        exit(1);
    }

    struct epoll_event evs[10];

    while(1){

    int num = epoll_wait(epfd, evs, 10, -1 );
    if(num == -1){
        perror("epoll_wait");
        continue;
    }

    for(int i = 0; i < num; i++){
        if(evs[i].events && EPOLLIN){
        if(evs[i].data.fd == fd){

            struct sockaddr_in client_addr;
            socklen_t len = sizeof(client_addr);

            int cfd = accept(fd, (struct sockaddr*)&client_addr, &len);
            error_handling(cfd == -1, "accept");

            setNonblocking(cfd);
            printf("tid:%d :client connect\n",gettid());
            memset(&ev, 0, sizeof(ev));
            ev.data.fd = cfd;
            ev.events = EPOLLIN;
            int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
            error_handling(ret == -1, "epoll_ctl");

        }
        else{

            handleReadevent(evs[i].data.fd, epfd);
        }
        }
    }


    }

}