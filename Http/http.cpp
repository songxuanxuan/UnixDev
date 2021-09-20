#include<sys/socket.h> 
#define DEBUG 1

#include<netinet/in.h> 
#include<arpa/inet.h> 
#include<cstdio> 
#include<unistd.h> 
#include<cerrno> 
#include<cstring> 
#include<fcntl.h> 
#include<cstdlib> 
#include<cassert> 
#include<sys/epoll.h>

#include "threadpool.h"
#include "http_conn.h"
#include "http_time_minheap.h"

#define MAX_FD 65536 
#define MAX_EVENT_NUMBER 10000
#define TIME_SLOT 5

static timer_heap* timers = new timer_heap(MAX_FD);
static int sigpipe[2];

extern int addfd(int epollfd, int fd, bool one_shot);
extern int removefd(int epollfd, int fd);
extern int setnoblocking(int fd);

void sig_handler(int sig)
{

	int save_erro = errno;
	int msg = sig;
	int ret = send(sigpipe[1], (char*)&msg, 1, 0);
	errno = save_erro;
}

void addsig(int sig, void (handler)(int), bool restart = true)
{
	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = handler;
	if (restart)
		sa.sa_flags |= SA_RESTART;
	sigfillset(&sa.sa_mask);
	int ret = sigaction(sig, &sa, nullptr);
	assert(ret != -1);
}

void show_error(int connfd, const char* info)
{
	printf("%s", info);
	send(connfd, info, strlen(info), 0);
	close(connfd);
}

void time_checker()
{
	timers->tick();
#ifdef DEBUG
	printf("timechecker done\n");
#endif
	int i = alarm(TIME_SLOT);
}


int main(int argc, char* argv[]) {

#ifdef DEBUG
	argc = 3;
	argv[1] = "localhost";
	argv[2] = "8080";
#endif // DEBUG

	if (argc <= 2)
	{
		printf("usage:%s ip_address port_number\n", basename(argv[0]));
		return 1;
	}
	const char* ip = argv[1];
	int port = atoi(argv[2]);
	
	/*创建线程池*/
	threadpool<http_conn>* pool = nullptr;
	try
	{
		pool = new threadpool<http_conn>;
	}
	catch (...)
	{
		return 1;
	}
	/*预先为每个可能的客户连接分配一个http_conn对象*/
	http_conn* users = new http_conn[MAX_FD];
	assert(users);
	int user_count = 0;
	int listenfd = socket(PF_INET, SOCK_STREAM, 0);
	assert(listenfd >= 0);

	/*如果在发送数据的过程中(send()没有完成，还有数据没发送)而调用了closesocket(),以前我们一般采取的措施是"从容关闭"
	即让没发完的数据发送出去后在关闭socket*/
	struct linger tmp = { 1,0 };
	setsockopt(listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));

	int ret = 0;
	struct sockaddr_in address;
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &address.sin_addr);
	address.sin_port = htons(port);
	ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
	assert(ret >= 0);
	ret = listen(listenfd, 5);
	assert(ret >= 0);
	epoll_event events[MAX_EVENT_NUMBER];
	int epollfd = epoll_create(5);
	assert(epollfd != -1);
	addfd(epollfd, listenfd, false);
	http_conn::m_epollfd = epollfd;

	//设置计时信号处理
	ret = socketpair(PF_UNIX, SOCK_STREAM, 0, sigpipe);
	assert(ret != -1);
	setnoblocking(sigpipe[1]);
	addfd(epollfd, sigpipe[0], false);	//alarm 可能是由系统创建的不同线程处理,oneshot后会无法唤醒这个socket
	addsig(SIGALRM, sig_handler);
	addsig(SIGINT, sig_handler);
	alarm(TIME_SLOT);
	/*忽略SIGPIPE信号*/
	addsig(SIGPIPE, SIG_IGN);


	while (true)
	{
		int is_stop = false;
		int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
		if ((number < 0) && (errno != EINTR))
		{
			printf("epoll failure\n");
			break;
		}
		for (int i = 0; i < number; i++)
		{
			int sockfd = events[i].data.fd;
			if (sockfd == listenfd)
			{
				//新连接处理
				struct sockaddr_in client_address;
				socklen_t client_addrlength = sizeof(client_address);
				int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlength);
				if (connfd < 0)
				{
					printf("errno is:%d\n", errno);
					continue;
				}
				if (http_conn::m_user_count >= MAX_FD)
				{
					show_error(connfd, "Internal server busy");
					continue;
				}

				users[connfd].init(connfd, client_address);
				timer_unit* timer = new timer_unit(TIME_SLOT * 3);
				timer->cb_func = users[connfd].timeout_close;
				timer->conn_data = &users[connfd];
				users[connfd].set_timer(timer);
				timers->add_timer(timer);
			}
			else if (sockfd == sigpipe[0] && (events[i].events & EPOLLIN))
			{
				//计时器事件处理
				bool is_timeout = false;
				char sigs[1024];
				int ret = recv(sigpipe[0], sigs, sizeof(sigs), 0);
				if (ret == -1)
					continue;
				else if (ret == 0)
					continue;
				else
				{
					for (int i = 0; i < ret; i++)
					{
						switch (sigs[i])
						{
						case SIGALRM:
						{
							is_timeout = true;
							break;
						}
						case SIGINT:
						{
							is_stop = true;
							break;
						}
						default:
							break;
						}
					}
				}
				if (is_timeout)
				{
					is_timeout = false;
					time_checker();
				}
					
			}
			else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
				users[sockfd].close_conn();
			else if (events[i].events & EPOLLIN)
			{
				users[sockfd].get_timer()->fresh_expire(TIME_SLOT * 3);
				//数据传入处理
				if (users[sockfd].read())
					pool->append(users + sockfd);
				else
					users[sockfd].close_conn();
			}
			else if (events[i].events & EPOLLOUT)
			{
				//数据传出处理
				if (!users[sockfd].write())
					users[sockfd].close_conn();

			}
		}
		if(is_stop)
			break;
	}

	//todo : users里面大量fd需要关闭吗
	close(epollfd);
	close(listenfd);
	close(sigpipe[0]);
	close(sigpipe[1]);
	delete[] timers;
	delete[] users;
	delete pool;
	return 0;
}