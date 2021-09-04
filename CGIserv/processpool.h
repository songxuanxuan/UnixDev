#ifndef _PROCESSPOOL_H
#define _PROCESSPOOL_H

#include <fcntl.h>
#include <sys/epoll.h>
#include <csignal>
#include <cassert>
#include <unistd.h>
#include <sys/socket.h>
#include <cerrno>
#include <cstdio>
#include <netinet/in.h>
#include <sys/wait.h>


class process
{
public:
	pid_t m_pid;
	int m_pipefd[2];

	process() :m_pid(-1) {};
};

template<class T>
class processpool
{
private:
	processpool(int listenfd, int process_num = 8);
public:
	static processpool<T>* create(int listenfd, int process_num = 8)
	{
		if (!m_instance)
			m_instance = new processpool<T>(listenfd, process_num);
		return m_instance;
	}
	~processpool()
	{
		delete[] m_sub_process;
	}
	/*启动进程池*/
	void run();
private:
	void setup_sig_pipe();
	void run_parent();
	void run_child();
private:
	/*进程池允许的最大子进程数量*/
	static const int MAX_PROCESS_NUMBER = 16;
	/*每个子进程最多能处理的客户数量*/
	static const int USER_PER_PROCESS = 65536;
	/*epoll 最多能处理的事件数*/
	static const int MAX_EVENT_NUMBER = 10000;
	/*进程池中的进程总数*/
	int m_process_number;
	/*子进程在池中的序号，从0开始*/
	int m_idx;
	/*每个进程都有一个epoll内核事件表，用m_epollfd标识*/
	int m_epollfd;
	/*监听socket*/
	int m_listenfd;
	/*子进程通过m_stop来决定是否停止运行*/
	int m_stop;
	/*保存所有子进程的描述信息*/
	process* m_sub_process;
	/*进程池静态实例*/
	static processpool<T>* m_instance;
};
template<class T>
processpool<T>* processpool<T>::m_instance = nullptr;


//全局sig管道fd
static int sig_pipefd[2];

static int setnoblocking(int fd)
{
	int old_opt = fcntl(fd, F_GETFL);
	int new_opt = old_opt | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_opt);
	return old_opt;
}

static void addfd(int epollfd, int fd)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	setnoblocking(fd);
}

static void removefd(int epollfd, int fd)
{
	epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
	close(fd);
}

static void addsig(int sig, void (handler)(int), bool restart = true)
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
static void sig_handler(int sig) 
{ 
	int save_errno = errno; 
	int msg = sig; 
	send(sig_pipefd[1], (char*)&msg, 1, 0); 
	errno = save_errno; 
}

/*进程池构造函数。参数listenfd是监听socket，它必须在创建进程池之前被创建，否则子进程无法直接引用它。
参数process_number指定进程池中子进程的数量*/
template<class T>
processpool<T>::processpool(int listenfd, int process_num /*= 8*/)
	:m_listenfd(listenfd), m_process_number(process_num), m_idx(-1), m_stop(false)
{
	assert((process_num > 0) && (process_num <= MAX_PROCESS_NUMBER));
	m_sub_process = new process[process_num];
	assert(m_sub_process);
	for (int i = 0; i < process_num; ++i)
	{
		int ret = socketpair(PF_UNIX, SOCK_DGRAM, 0, m_sub_process[i].m_pipefd);
		assert(ret != -1);
		m_sub_process[i].m_pid = fork();
		//要保证都有pid
		assert(m_sub_process[i].m_pid >= 0);
		if (m_sub_process[i].m_pid > 0)
		{
			close(m_sub_process[i].m_pipefd[0]);
			continue;
		}
		else
		{
			close(m_sub_process[i].m_pipefd[1]);
			m_idx = i;
			break;
		}
	}
}

/*统一事件源*/
template<class T>
void processpool<T>::setup_sig_pipe()
{
	m_epollfd = epoll_create(5);
	assert(m_epollfd != -1);
	int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, sig_pipefd);
	assert(ret != -1);
	setnoblocking(sig_pipefd[1]);
	addfd(m_epollfd, sig_pipefd[0]);
	/*设置信号处理函数*/
	addsig(SIGCHLD, sig_handler);
	addsig(SIGTERM, sig_handler);
	addsig(SIGINT, sig_handler);
	addsig(SIGPIPE, SIG_IGN);
}

/*父进程中m_idx值为-1，子进程中m_idx值大于等于0，我们据此判断接下来要运行的是父进程代码还是子进程代码*/
template<typename T>
void processpool<T>::run()
{
	if (m_idx != -1)
	{
		run_child();
		return;
	}
	run_parent();
}


template<class T>
void processpool<T>::run_parent()
{
	setup_sig_pipe();
	addfd(m_epollfd, m_listenfd);
	int sub_process_counter = 0;
	int new_conn = 1;
	int number = 0;
	int ret = -1;
	epoll_event events[MAX_EVENT_NUMBER];
	while (!m_stop)
	{
		number = epoll_wait(m_epollfd, events, MAX_EVENT_NUMBER, -1);
		if (number < 0 && errno != EINTR)
		{
			printf("epoll failure\n");
			break;
		}
		for (int i = 0; i < number; ++i)
		{
			int sockfd = events[i].data.fd;
			if (sockfd == m_listenfd)
			{
				/*如果有新连接到来，就采用Round Robin 方式将其分配给一个子进程处理*/
				int i = sub_process_counter;
				do
				{
					//转一圈找到第一个m_pid != -1 的线程
					if (m_sub_process[i].m_pid != -1)
						break;
					i = (i + 1) % m_process_number;
				} while (i != sub_process_counter);
				if (m_sub_process[i].m_pid == -1)
				{
					m_stop = true;
					break;
				}
				sub_process_counter = (i + 1) % m_process_number;
				send(m_sub_process[i].m_pipefd[1], &new_conn, sizeof(new_conn), 0);
				printf("send request to child%d\n", i);
			}
			else if (sockfd == sig_pipefd[0] && (events[i].events & EPOLLIN))
			{
				//int sig;
				char signals[1024];
				ret = recv(sig_pipefd[0], signals, sizeof(signals), 0);
				if (ret <= 0)
					continue;
				for (int i = 0; i < ret; ++i)
				{
					switch (signals[i])
					{
					case SIGCHLD:
					{
						/*如果进程池中第i个子进程退出了，则主进程关闭相应的通信管道，
						并设置相应的m_pid为-1，以标记该子进程已经退出*/
						pid_t pid;
						while ((pid = waitpid(-1, nullptr, WNOHANG) > 0))
						{
							for (int i = 0;i<m_process_number;++i)
							{
								if (m_sub_process[i].m_pid == pid)
								{
									printf("child%d join\n", i);
									close(m_sub_process[i].m_pipefd[0]);
									m_sub_process[i].m_pid = -1;
								}
							}
						}
						/*如果所有子进程都已经退出了，则父进程也退出*/
						m_stop = true; 
						for (int i = 0; i<m_process_number; ++i) 
							if (m_sub_process[i].m_pid != -1) 
								m_stop = false;  
						break;
					}
					case SIGTERM:
					case SIGINT:
					{
						printf("kill all the clild now\n");
						for (int i = 0; i<m_process_number; ++i) 
						{ 
							int pid = m_sub_process[i].m_pid; 
							if (pid != -1) 
								kill(pid, SIGTERM);  
						}
						m_stop = true;
						break;
					}
					default:
						break;
					}
				}
			}
			else
				continue;
		}
	}
	close(m_epollfd);

}


template<class T>
void processpool<T>::run_child()
{
	setup_sig_pipe();
	int pipefd = m_sub_process[m_idx].m_pipefd[0];
	addfd(m_epollfd, pipefd);
	epoll_event events[MAX_EVENT_NUMBER];
	T* users = new T[USER_PER_PROCESS];
	assert(users);
	int number = 0;
	int ret = -1;
	while (!m_stop)
	{
		number = epoll_wait(m_epollfd, events, MAX_EVENT_NUMBER, -1);
		if ((number < 0) && (errno != EINTR))
		{
			printf("epoll failure\n");
			break;
		}
		for (int i = 0; i < number; ++i)
		{
			int sockfd = events[i].data.fd;
			if (sockfd == pipefd && (events[i].events & EPOLLIN))
			{
				int client = 0;
				ret = recv(pipefd, &client, sizeof(client), 0);
				//错误发生,处理下一个
				if ((ret < 0 && errno != EAGAIN) || ret == 0)
					continue;
				else
				{
					struct sockaddr_in client_addr;
					socklen_t addr_len = sizeof(client_addr);
					int connfd = accept(m_listenfd, (sockaddr*)&client_addr, &addr_len);
					if (connfd < 0)
					{
						printf("errno is:%d\n", errno);
						continue;
					}
					addfd(m_epollfd, connfd);
					/*模板类T必须实现init方法，以初始化一个客户连接。我们直接使用connfd来索引逻辑处理对象（T类型的对象），
					以提高程序效率*/
					//todo:init && connfd可能大于MAX吗
					users[connfd].init(m_epollfd, connfd, client_addr);

				}
			}
			else if (sockfd == sig_pipefd[0] && (events[i].events & EPOLLIN))
			{
				//int sig;
				char signals[1024];
				ret = recv(sig_pipefd[0], signals, sizeof(signals), 0);
				if (ret <= 0)
					continue;
				for (int i = 0; i < ret; ++i)
				{
					switch (signals[i])
					{
					case SIGCHLD:
					{
						pid_t pid;
						while ((pid = waitpid(-1, nullptr, WNOHANG) > 0))
							continue;
						break;
					}
					case SIGTERM:
					case SIGINT:
					{
						m_stop = true;
						break;
					}
					default:
						break;
					}
				}
			}
			/*如果是其他可读数据，那么必然是客户请求到来。调用逻辑处理对象的process方法处理*/
			else if (events[i].events & EPOLLIN)
			{
				users[sockfd].process();
			}
			else
			{
				continue;
			}
		}
	}
	delete[] users;
	users = nullptr;
	close(pipefd);
	close(m_epollfd);
}




#endif // _PROCESSPOOL_H__
