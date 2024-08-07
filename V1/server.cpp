#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<string.h>
#include<thread>

#define PORT 8888
using namespace std;
void error_handling(bool is_exit,const char *message){
    if(is_exit){
        perror(message);
        exit(0);
    }
}

void working_thread(int clientfd){
    char buf[1024];
    while(1){
    memset(buf, 0, sizeof(buf));

    int len = read(clientfd, buf, sizeof(buf));

    if(len > 0){
        printf("[%d] %s\n", clientfd, buf);
    }
    else if(len == 0){
        printf("[%d] disconnected\n", clientfd);
        
    }
    else{
        error_handling(true, "read error");
    }
    }

    close(clientfd);
    
}

int main(int argc, char *argv[]){

    int fd;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    error_handling(fd == -1, "socket error");

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    int result = bind(fd, (sockaddr *)&serv_addr, sizeof(serv_addr));
    error_handling(result != 0, "bind error");

    result = listen(fd, 10);
    error_handling(result != 0, "listen error");

    while(1){

        struct sockaddr_in cliaddr;
        socklen_t len = sizeof(cliaddr);

        int cfd = accept(fd, (sockaddr *)&cliaddr, &len);
        if(cfd == -1){
            perror("accept");
            continue;
        }


        char ip[1024] = {0};
        printf("new client fd accepted. fd: %d, ip:%s, port:%d\n", cfd, inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, ip, sizeof(ip)), htons(cliaddr.sin_port));

        std::thread t(working_thread, cfd);
        t.detach();

    }

    close(fd);
    return 0;

}


