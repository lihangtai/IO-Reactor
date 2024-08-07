#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <thread>
using namespace std;
void perror_handling(bool condition, const char *info){

    if(condition){
        perror(info);
        exit(1);
    }
}

int main(){

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    perror_handling(fd == -1, "socket error");

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET; 
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    serv_addr.sin_port = htons(8888);

    int result = connect(fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    perror_handling(result == -1, "connect error");

    int i = 0;
    while(1){

        char buf[1024];
        snprintf(buf, sizeof(buf), "tid:%ld, hello world %d\n", pthread_self(), i++);
        write(fd, buf, strlen(buf));
        memset(buf, 0,sizeof(buf));
        sleep(1);
    }
   
   close(fd);
   return 0;

}