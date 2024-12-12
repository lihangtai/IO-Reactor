#include"src/Server.h"
#include<stdio.h>
#include<functional>
#include<thread>
#include<memory>
#include<iostream>

using namespace std;

void log_test()
{

	std::thread t1([]() {
		for (int i = 0; i < 900; ++i)
			LOG_INFO << "1111111 " << 23 << 34 << "buox";
		});
	t1.detach();
	std::thread t2([]() {
		for (int i = 0; i < 1010; ++i)
			//LOG_DEBUG << "22woshisdfsd " << 23 << 34 << "buox";
			LOG_INFO << "2222" << 555 << " test";
		});
	t2.detach();
	for (int i = 0; i < 1010; ++i)
		LOG_INFO << "333333" << 23;
	std::this_thread::sleep_for(std::chrono::seconds(3));
}

int main()
{
	
	log_test();
	
	InetAddr servAddr(10000);
	EventLoop loop;
	Server server(servAddr, &loop);

	server.setConnectionCallback([](const ConnectionPtr& conn) {
		if (conn->connected()) {
			printf("Connection connected ip:port: %s  connected..\n", conn->peerAddress().toIpPort().c_str());
		}else {
			printf("Connection disconnected\n");
		}
	});

	server.setMessageCallback([](const ConnectionPtr& conn, Buffer* buf) {
			std::string msg(buf->retrieveAllAsString());
			printf("onMessage() %ld bytes reveived:%s\n", msg.size(), msg.c_str());
			conn->send(msg);  // »ØËÍœÓÊÜµÄÊýŸÝ 
			
	});

	server.start(2);	
	loop.loop();

	return 0;
}
