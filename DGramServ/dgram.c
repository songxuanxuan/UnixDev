#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>  //inet_ntoa

#define HOSTLEN 256

int get_internet_address(char* host, int len, int* portp, struct sockaddr_in* addrp);

void say_who_called(struct sockaddr_in* saddr)
{
	char host[BUFSIZ];
	int port;
	get_internet_address(host, BUFSIZ, &port, saddr);
	printf("from :%s : %d \n", host, port);
}

int make_internet_address(char* hostname, int port, struct sockaddr_in* ap) {
	struct hostent* hp;
	if ((hp = gethostbyname(hostname)) == NULL)
		return -1;
	bzero(ap, sizeof(struct sockaddr_in));
	bcopy(hp->h_addr_list[0], &ap->sin_addr, hp->h_length);
	ap->sin_port = htons(port);
	ap->sin_family = AF_INET;
	return 0;


}

int make_dgram_server_socket(int port)
{
	struct sockaddr_in saddr;
	int sock_fd;
	char hostname[HOSTLEN];
	if ((sock_fd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
		return -1;
	gethostname(hostname, HOSTLEN);
	make_internet_address(hostname, port, &saddr);
	if (bind(sock_fd,(struct sockaddr*) &saddr, sizeof(saddr)) == -1)
		return -1;
	return sock_fd;
}


int get_internet_address(char* host, int len, int* portp, struct sockaddr_in* addrp)
{
	strncpy(host, inet_ntoa(addrp->sin_addr), len);
	*portp = ntohs(addrp->sin_port);
	return 0;
}

int make_dgram_client_socket() {
	return socket(PF_INET, SOCK_DGRAM, 0);
}