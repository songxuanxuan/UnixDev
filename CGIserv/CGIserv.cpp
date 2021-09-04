#include <cstring>
#include <cstdlib>
#include <arpa/inet.h>
#include "processpool.h"

class cgi_conn
{
public:
	cgi_conn() = default;
	~cgi_conn() = default;

	void init(int epollfd, int sockfd, const sockaddr_in& client_addr)
	{
		m_epollfd = epollfd;
		m_address = client_addr;
		m_sockfd = sockfd;
		memset(m_buf, '\0', sizeof(m_buf));
		m_read_idx = 0;
	}
	void process()
	{
		int idx = 0;
		int ret = -1;
		while (true)
		{
			idx = m_read_idx;
			ret = recv(m_sockfd, m_buf + idx, BUFFER_SIZE - 1 - idx, 0);
			if (ret < 0)
			{
				/*如果读操作发生错误,则关闭客户连接。但如果是暂时无数据可读，则退出循环*/
				if (errno != EAGAIN)
					removefd(m_epollfd, m_sockfd);
				break;
			}
			else if (ret == 0)
			{
				/*如果对方关闭连接，则服务器也关闭连接*/
				removefd(m_epollfd, m_sockfd);
				break;
			}
			else
			{
				//buf 的 index 往后移动
				m_read_idx += ret;
				printf("user content is:%s\n", m_buf);
				/*如果遇到字符“ \r\n ”  ，则开始处理客户请求*/
				for (; idx < m_read_idx; ++idx)
				{
					if (idx >= 1 && m_buf[idx - 1] == '\r' && m_buf[idx] == '\n')
						break;
				}
				if (idx == m_read_idx)
					continue;
				m_buf[idx - 1] = '\0';
				char* file_name = m_buf;
				/*判断客户要运行的CGI程序是否存在*/
				if (access(file_name, F_OK) == -1)
				{
					printf("%s no available\n", file_name);
					removefd(m_epollfd, m_sockfd);
					break;
				}
				ret = fork();
				if (ret == -1)
				{
					removefd(m_epollfd, m_sockfd);
					break;
				}
				else if (ret > 0)
				{
					removefd(m_epollfd, m_sockfd);
					break;
				}
				else
				{
					close(STDOUT_FILENO);
					dup(m_sockfd);
					execl(file_name, file_name, nullptr);
					exit(0);
				}
			}
		}
	}

private:
	static const int BUFFER_SIZE = 1024;
	static int m_epollfd;
	int m_sockfd;
	//好像没啥用
	sockaddr_in m_address;
	char m_buf[BUFFER_SIZE];
	int m_read_idx;
};
int cgi_conn::m_epollfd = -1;

#define DEBUG 0
int main(int argc, char* argv[])
{
#if DEBUG
	argc = 3;
	argv[1] = "localhost";
	argv[2] = "9999";

#endif
	if (argc <= 2) 
	{
		printf("usage:%s ip_address port_number\n", basename(argv[0]));
		return 1;
	} 
	const char* ip = argv[1];
	int port = atoi(argv[2]);
	int listenfd = socket(PF_INET, SOCK_STREAM, 0);
	assert(listenfd >= 0);
	int ret = 0;
	struct sockaddr_in address;
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &address.sin_addr);
	address.sin_port = htons(port);
	ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
	assert(ret != -1);
	ret = listen(listenfd, 5);
	assert(ret != -1);
	processpool<cgi_conn>* pool = processpool<cgi_conn>::create(listenfd);
	if (pool) 
	{
		pool->run();
		delete pool;
	} 
	close(listenfd);
	/*正如前文提到的，main函数创建了文件描述符listenfd，那么就由它亲自关闭之*/ 
	return 0;

}