#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include "../DGramServ/dgram.h"

#define HOST "ubuntu"
#define PORT 4444
#define DFT_MSG "HELLO SERVER"

int main() {
	struct sockaddr_in saddr;
	struct sockaddr_in saddr_serv;
	char *msg = DFT_MSG;
	char buf[BUFSIZ];
	int sock_request_fd, sock_response_fd;
	int recvlen;
	int remote_socklen;
	
	sock_request_fd = make_dgram_client_socket();
	if (sock_request_fd == -1)
		oops("make socket");
	if (make_internet_address(HOST, PORT, &saddr) == -1)
		oops("make address");
	//todo : how to assign a port
	if (sendto(sock_request_fd, msg, strlen(msg), 0, (struct sockaddr*)&saddr,(socklen_t) sizeof(saddr)) == -1)
		oops("sendto ");
	//bind self to sock_fd
	//sock_response_fd = make_dgram_server_socket(my_port);
	if ((recvlen = recvfrom(sock_request_fd, buf, BUFSIZ, 0, (struct sockaddr*)&saddr_serv, &remote_socklen)) == -1)
		oops("receive  from ");
	buf[recvlen] = '\0';
	printf("--receive : %s  from %s : %d\n", buf,inet_ntoa(saddr_serv.sin_addr), ntohs(saddr_serv.sin_port));
	getchar();
	return 0;

}