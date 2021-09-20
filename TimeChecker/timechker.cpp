#include<sys/types.h> 
#include<sys/socket.h> 
#include<netinet/in.h> 
#include<arpa/inet.h> 
#include<cassert>
#include<cstdio> 
#include<csignal> 
#include<unistd.h> 
#include<cerrno> 
#include<cstring> 
#include<fcntl.h> 
#include<cstdlib> 
#include<sys/epoll.h> 
#include<pthread.h> 
#include"time_util.h" 
#define FD_LIMIT 65535 
#define MAX_EVENT_NUMBER 1024 
#define TIMESLOT 5

static sort_time_lst timer_lst;
static int epollfd = 0;
static int pipefd[2];

//����fd������
int set_noblocking(int fd)
{
	int oldopt = fcntl(fd, F_GETFL);
	int newopt = oldopt | O_NONBLOCK;
	fcntl(fd, F_SETFL, newopt);
	return oldopt;
}

//���fd(������)����epollfd����
void addfd(int epollfd, int fd)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET;	//���� �������ͱ�Եģʽ
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	set_noblocking(fd);
}

//�źŴ�����,���źŷ��ͳ�ȥ������
void sig_handler(int sig)
{
	int save_erro = errno;
	int msg = sig;
	send(pipefd[1], (char*)&msg, 1, 0);
	errno = save_erro;
}

//�����ź�
void add_sig(int sig)
{
	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = sig_handler;
	sa.sa_flags |= SA_RESTART;	//����ź��ж�ʱ����ϵͳ������,�򷵻�ʱ���¿�ʼ�õ���
	sigfillset(&sa.sa_mask);
	assert(sigaction(sig, &sa, nullptr) != -1);
}

//ʱ�䴦����
void timer_handler()
{
	timer_lst.tick();
	alarm(TIMESLOT);
}

//��ʱ���ص�:ɾ���ǻsock����
void cb_func(client_data* usr_data)
{
	epoll_ctl(epollfd, EPOLL_CTL_DEL, usr_data->sockfd, 0);
	assert(usr_data);
	close(usr_data->sockfd);
	printf("close fd %d\n", usr_data->sockfd);
}

//���ü����˿�port,����listenfd
int set_listen(int port)
{
	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	const char* ip = "localhost";
	inet_pton(PF_INET, ip, &addr.sin_addr);
	int listenfd = socket(PF_INET, SOCK_STREAM, 0);
	assert(listenfd >= 0);
	int ret = bind(listenfd, (struct sockaddr*)&addr, sizeof(addr));
	assert(ret != -1);
	ret = listen(listenfd, 5);
	assert(ret != -1);
	return listenfd;
}
#define  DEBUG 1
int main(int ac, char* av[])
{
#if DEBUG
	ac = 2;
	av[1] = "8888";
#endif
	if (ac < 2)
	{
		printf("usage:%s port", basename(av[0]));
		return -1;
	}

	int listenfd = set_listen(atoi(av[1]));

	//����epoll����
	epoll_event events[MAX_EVENT_NUMBER];
	int epollfd = epoll_create(5);
	assert(epollfd != -1);
	addfd(epollfd, listenfd);
	int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd);
	assert(ret != -1);
	set_noblocking(pipefd[1]);
	addfd(epollfd, pipefd[0]);

	//�����źŴ���
	add_sig(SIGALRM);
	add_sig(SIGTERM);
	bool stop = false;
	client_data* usrs = new client_data[FD_LIMIT];
	bool timeout = false;

	alarm(TIMESLOT);
	while (!stop)
	{
		int num = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
		//alarm �ᷢ��interupt �ź�EINTR
		if (num < 0 && errno != EINTR)
		{
			printf("epoll failure\n");
			break;
		}
		
		for (int i = 0; i < num; i++)
		{
			int sockfd = events[i].data.fd;
			if (sockfd == listenfd)
			{
				struct sockaddr_in clnt_addr;
				socklen_t addr_len = sizeof(clnt_addr);
				int connfd = accept(listenfd, (struct sockaddr*)&clnt_addr, &addr_len);
				addfd(epollfd, connfd);
				usrs[connfd].address = clnt_addr;
				usrs[connfd].sockfd = connfd;

				//������ʱ��
				util_timer* timer = new util_timer;
				timer->usr_data = &usrs[connfd];
				timer->expire = time(nullptr) + 3 * TIMESLOT;
				timer->cb_func = cb_func;
				timer_lst.add_time(timer);
				usrs[connfd].timer = timer;
			}
			else if (sockfd == pipefd[0] && (events[i].events & EPOLLIN))	
			{
				//�����ź�
				int sig;
				char signals[1024];
				ret = recv(pipefd[0], signals, sizeof(signals), 0);
				if (ret == -1)
					continue;
				else if (ret == 0)
					continue;
				else
				{
					for (int i = 0; i < ret; i++)
					{
						switch (signals[i])
						{
						case SIGALRM:
						{
							timeout = true;
							break;
						}
						case SIGTERM:
						{
							stop = true;
							break;
						}
						default:
							break;
						}
					}
				}

			}
			else if (events[i].events & EPOLLIN)
			{
				memset(usrs[sockfd].buf, '\0', BUFFER_SIZE);
				ret = recv(sockfd, usrs[sockfd].buf, BUFFER_SIZE - 1, 0);
				printf("get%d bytes of client data%s from%d\n", ret, usrs[sockfd].buf, sockfd);
				util_timer* timer = usrs[sockfd].timer;
				if (ret < 0)
				{
					if (errno!=EAGAIN)
					{
						cb_func(&usrs[sockfd]);
						if (timer)
							timer_lst.del_timer(timer);
					}
				}
				else if (ret == 0)
				{
					cb_func(&usrs[sockfd]);
					if(timer)
						timer_lst.del_timer(timer);
				}
				else 
				{
					//���µ��������������ӵ�ʱ��
					if (timer)
					{
						timer->expire = time(nullptr) + 3 * TIMESLOT;
						printf("adjust expire\n");
						timer_lst.adjust_timer(timer);
					}
				}
			}
			if (timeout)
			{
				timer_handler();
				timeout = false;
			}

		}
		
	}
	close(listenfd);
	close(pipefd[0]);
	close(pipefd[1]);
	close(epollfd);
	delete[] usrs;
	return 0;
}