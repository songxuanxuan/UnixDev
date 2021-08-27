#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include<assert.h> 
#include<stdio.h> 
#include<unistd.h> 
#include<errno.h> 
#include<string.h>
#include<fcntl.h>
#include<stdlib.h>
#include<sys/epoll.h>
#include<pthread.h>
#define MAX_EVENT_NUMBER 1024
#define BUFFER_SIZE 10
/*将文件描述符设置成非阻塞的*/
int setnonblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}
/*将文件描述符fd上的EPOLLIN注册到epollfd指示的epoll内核事件表中，参数enable_et指定是否对fd启用ET模式*/
void addfd(int epollfd, int fd, bool enable_et, bool oneshot)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN;
	if (enable_et)
	{
		event.events |= EPOLLET;
	}
	if (oneshot)
	{
		event.events |= EPOLLONESHOT;
	}
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	setnonblocking(fd);
}

/*重置fd上的事件。这样操作之后，尽管fd上的EPOLLONESHOT事件被注册，但是操作系统仍然会触发fd上的EPOLLIN事件，且只触发一次*/
void reset_oneshot(int epollfd, int fd)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
	epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}
/*LT模式的工作流程*/
void lt(epoll_event* events, int number, int epollfd, int listenfd)
{
	char buf[BUFFER_SIZE];
	for (int i = 0; i < number; i++)
	{
		int sockfd = events[i].data.fd;
		if (sockfd == listenfd)
		{
			struct sockaddr_in client_address;
			socklen_t client_addrlength = sizeof(client_address);
			int connfd = accept(listenfd, (struct sockaddr*)&client_address,
				&client_addrlength);
			addfd(epollfd, connfd, false, false);/*对connfd禁用ET模式*/
		}
		else if (events[i].events & EPOLLIN)
		{
			/*只要socket读缓存中还有未读出的数据，这段代码就被触发*/
			printf("event trigger once\n");
			memset(buf, '\0', BUFFER_SIZE);
			int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
			if (ret <= 0)
			{
				close(sockfd);
				continue;
			}
			printf("get%d bytes of content:%s\n", ret, buf);
		}
		else
		{
			printf("something else happened\n");
		}
	}
}

struct  fds
{
	int epollfd;
	int sockfd;
};

void* worker(void* arg)
{
	int sockfd = ((fds*)arg)->sockfd;
	int epollfd = ((fds*)arg)->epollfd;
	printf("start new thread to receive data on fd:%d\n", sockfd);
	char buf[BUFFER_SIZE];
	//memset(buf, '\0', BUFFER_SIZE); 
	/*循环读取sockfd上的数据，直到遇到EAGAIN错误*/
	printf("event trigger once\n");
	while (1)
	{
		memset(buf, '\0', BUFFER_SIZE);
		int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
		if (ret < 0)
		{
			/*对于非阻塞IO，下面的条件成立表示数据已经全部读取完毕。此后，
			epoll就能再次触发sockfd上的EPOLLIN事件，以驱动下一次读操作*/
			if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
			{
				reset_oneshot(epollfd, sockfd);
				printf("read later\n");
				break;
			}
			//close(sockfd); 
			break;
		}
		else if (ret == 0)
		{
			printf("forein closed\n");
			close(sockfd);
		}
		else
		{
			printf("get%d bytes of content:%s\n", ret, buf);
			sleep(5);
		}
	}
	printf("end thread receiving data on fd:%d\n", sockfd);
}
/*ET模式的工作流程*/
/*ONESHOT模式*/
void et(epoll_event* events, int number, int epollfd, int listenfd)
{
	char buf[BUFFER_SIZE];
	for (int i = 0; i < number; i++)
	{
		int sockfd = events[i].data.fd;
		if (sockfd == listenfd)
		{
			struct sockaddr_in client_address;
			socklen_t client_addrlength = sizeof(client_address);
			int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlength);
			addfd(epollfd, connfd, true, false);/*对connfd开启ET模式*/
		}
		else if (events[i].events & EPOLLIN)
		{
			/*这段代码不会被重复触发，所以我们循环读取数据，以确保把socket读缓存中的所有数据读出*/
			fds fds_of_new_worker;
			fds_of_new_worker.epollfd = epollfd;
			fds_of_new_worker.sockfd = sockfd;
			pthread_t thread;
			pthread_create(&thread, NULL, worker, &fds_of_new_worker);
		}
		else { printf("something else happened\n"); }
	}
}

int main(int argc, char* argv[]) {

	if (argc <= 2)
	{
		printf("usage:%s ip_address port_number\n", basename(argv[0])); return 1;
	}
	const char* ip = argv[1];
	int port = atoi(argv[2]);
	int ret = 0;
	struct sockaddr_in address;
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &address.sin_addr);
	address.sin_port = htons(port);
	int listenfd = socket(PF_INET, SOCK_STREAM, 0);
	assert(listenfd >= 0);
	ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
	assert(ret != -1);
	ret = listen(listenfd, 5);
	assert(ret != -1);
	epoll_event events[MAX_EVENT_NUMBER];
	int epollfd = epoll_create(5);
	assert(epollfd != -1);
	/*注意，监听socket
	listenfd上是不能注册EPOLLONESHOT事件的，否则应用程序只能处理一个客户连接！因为后续的客户连接请求将不再触发listenfd上的
	EPOLLIN事件*/
	addfd(epollfd, listenfd, true, false);
	while (1)
	{
		int ret = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
		if (ret < 0)
		{
			printf("epoll failure\n"); break;
		}
		//lt(events, ret, epollfd, listenfd);/*使用LT模式*/
		et(events, ret, epollfd, listenfd);/*使用ET oneshot 模式*/
	}
	close(listenfd);
	return 0;
}