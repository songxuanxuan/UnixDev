#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include "dgram.h"


#define PORT 4444
#define DEF_RESPONSE "TICK stat:200"

int main() {
	int sock_fd;
	int msglen;
	char buf[BUFSIZ];
	char *respons  = DEF_RESPONSE;
	struct sockaddr_in saddr;
	socklen_t saddrlen = sizeof(saddr);		//!!!必须先初始化,recvfrom 会先用len长度申请空间再接受数据

	if ((sock_fd = make_dgram_server_socket(PORT)) == -1)
		oops("make server");
	while ((msglen = recvfrom(sock_fd,buf,BUFSIZ,0,(struct sockaddr*)&saddr,&saddrlen)) >0)
	{
		buf[msglen] = '\0';
		printf("dgrecv: %s \n", buf);
		say_who_called(&saddr);
		if (sendto(sock_fd, respons, strlen(respons), 0,(struct sockaddr*)&saddr, saddrlen) == -1)
			printf("fail send \n");
	}
	return 0;
}




