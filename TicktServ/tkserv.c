#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>

#define MSGLEN 128

int set_up();
void handle_request(const char* req, struct sockaddr_in* client);
void ticket_reclaim();

#define  RECLAIM_INTERVAL 60

int main() {
	int sock;
	char buf[MSGLEN];
	struct sockaddr_in client_addr;
	int addrlen = sizeof(client_addr);
	unsigned time_left;
	sock = set_up();

	signal(SIGALRM, ticket_reclaim);
	alarm(RECLAIM_INTERVAL);
	int retlen;
	while (1) {
		if ((retlen = recvfrom(sock, buf, MSGLEN, 0, (struct sockaddr*)&client_addr, &addrlen)) != -1) {
			buf[retlen] = '\0';
			narrate("GOT:", buf, &client_addr);
			time_left = alarm(0);
			handle_request(buf, &client_addr);
			alarm(time_left);
		}
		else if (errno != EINTR)
			perror("recvfrom");
	}

}