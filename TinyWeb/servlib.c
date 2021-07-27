#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <sys/wait.h>

#define QUENUM 1
#define HOSTLEN 256


static int make_server_socket_q(int portnum,int quenum);

int make_server_socket(int portnum) {
	return make_server_socket_q(portnum, QUENUM);
}


//todo
int connect_to_server(char* host, int portnum)
{

}

void child_wait(int signum)
{
	while (waitpid(-1, NULL, WNOHANG) > 0);		//wait all children process,no block
}

int make_server_socket_q(int portnum, int quenum) {
	struct sockaddr_in saddr;
	struct hostent* hp;
	int sock_id;
	char hostname[HOSTLEN];

	bzero(&saddr, sizeof(saddr));
	gethostname(hostname, HOSTLEN);
	hp = gethostbyname(hostname);
	bcopy(hp->h_addr_list[0], &saddr, hp->h_length);
	saddr.sin_port = htons(portnum);
	saddr.sin_family = AF_INET;
	if ((sock_id = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		return -1;
	if (bind(sock_id, (struct sockaddr*)&saddr, sizeof(saddr)) == -1)
		return -1;
	if (listen(sock_id, quenum) != 0)
		return -1;
	return sock_id;
	

}