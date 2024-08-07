#include "Epoll/Epoll.h"
#include "InetAddr/InetAddr.h"
#include "socket/socket.h"
#include "util/util.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

void handleEvent(int sockfd, Epoll& epoll);
int main(){

    InetAddr serv_addr(8888);
    Socket serv_socket;
    serv_socket.bind(&serv_addr);
    serv_socket.listen();
    serv_socket.setNonblock();

    Epoll epoll;
    epoll.update(serv_socket.fd(), EPOLLIN, EPOLL_CTL_ADD);

    while(1){

        vector<epoll_event> active;
        epoll.Epoll_wait(active);
        int num = active.size();

        for(int i = 0;i<num;i++){

            int curfd = active[i].data.fd;

            if(active[i].events & EPOLLIN ){
                if(curfd == serv_socket.fd()){
                    InetAddr client_addr;

                    Socket *client_socket = new Socket(serv_socket.accept(&client_addr));

                    client_socket->setNonblock();

                    epoll.update(client_socket->fd(), EPOLLIN, EPOLL_CTL_ADD);

                }
                else{
                    handleEvent(curfd, epoll);                }

            }
        }
    }

    return 0;
}

void handleEvent(int sockfd, Epoll& epoll){

    char buf[1024];
    memset(buf, 0, sizeof(buf));

    ssize_t ret = read(sockfd, buf, sizeof(buf));

    if(ret == -1){
        perror_if(ret == -1, "read");
    }
    else if(ret == 0){
        printf("the client fd is disconnected/n");
        epoll.epoll_delete(sockfd);
        close(sockfd);
    
    }
    else{
        printf("client fd %d says %s", sockfd, buf);
    }
}