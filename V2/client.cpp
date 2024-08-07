#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<string.h>

//
void perror_if(bool condition, const char* errorMessage)
{
	if (condition) {
		perror(errorMessage);
		exit(1);
	}
}

int main()
{
	//
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	perror_if(fd == -1, "socket");

	//
	struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET; 
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    serv_addr.sin_port = htons(8888);

	int ret = connect(fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	perror_if(ret == -1, "connect");


	// 3. 
	int n = 0;
	while(1)
	{
		// 
		char buf[512] = { 0 };
		sprintf(buf, "hi, I am client...%d\n", n++);
		write(fd, buf, strlen(buf));

		// 
		memset(buf, 0, sizeof(buf));
		int len = read(fd, buf, sizeof(buf));
		if (len > 0)
		{
			printf("server say: %s\n", buf);
		}
		else if (len == 0)
		{
			printf("server disconnect...\n");
			break;
		}
		else
		{
			perror("read");
			break;
		}
		sleep(1);   // 
	}

	close(fd);

	return 0;
}