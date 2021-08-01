#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <bits/types.h>
#include "../DGramServ/dgram.h"

#define PORT 2020

#define MSGLEN 128
#define HOSTLEN 512
#define MAX_USERS 3
#define TICKET_AVAIL 0

int ticket_array[MAX_USERS];
int num_ticket_out = 0;
int sd = -1;



void narrate(const char* msg1,const char* msg2, struct sockaddr_in* cp);

void free_all_ticket() {
	for (int i = 0; i < MAX_USERS; ++i) {
		ticket_array[i] = TICKET_AVAIL;
	}
}

int set_up() {
	sd = make_dgram_server_socket(PORT);
	if (sd == -1)
		oops("make socket");
	free_all_ticket();
	return sd;
}

char* do_hello(const char* req)
{
	static char replybuf[MSGLEN];
	int i;
	if (num_ticket_out >= MAX_USERS)
		return "FAIL no ticket available";
	for (i = 0; i < MAX_USERS && ticket_array[i] != TICKET_AVAIL; ++i);
	if (i == MAX_USERS) {
		narrate("database corrupt", "", NULL);
		return "FAIL database corrupt";
	}
	ticket_array[i] = atoi(req + 5);
	sprintf(replybuf, "TICK %d.%d", ticket_array[i], i);
	num_ticket_out++;
	return replybuf;
}

char* do_goodbye(const char* req)
{
	int pid, slot;
	if (sscanf(req + 5, "%d.%d", &pid, &slot) != 2 || (ticket_array[slot] != pid)) {
		narrate("bogus ticket", req + 5, NULL);
		return "FAIL invalid ticket";
	}
	ticket_array[slot] = TICKET_AVAIL;
	--num_ticket_out;
	return("THNX see ya");
}


char* do_validate(char* req) {
	int pid, slot;
	if (sscanf(req + 5, "%d.%d", &pid, &slot) == 2 || (ticket_array[slot] == pid)) {
		return "GOOD valid ticket";
	}
	narrate("Bogus ticket", req + 5, NULL);
	return "FAIL invalid ticket";
}

void handle_request(const char* req, struct sockaddr_in* client) {
	char* respons;
	if (strncmp(req, "HELO", 4) == 0)
		respons = do_hello(req);
	else if (strncmp(req, "GBYE", 4) == 0)
		respons = do_goodbye(req);
	else if (strncmp(req, "VALD", 4) == 0)
		respons = do_validate(req);
	else
		respons = "FAIL incompatible request";
	narrate("SAID:", respons, client);
	if (sendto(sd, respons, strlen(respons), 0,(struct sockaddr*) client, sizeof(*client)) == -1)
		perror("failed send");
}

#define RECLAIM_INTERVAL 60
void ticket_reclaim() {
	printf("ticket checking... \n");
	char tick[BUFSIZ];
	for (int i = 0; i < MAX_USERS; ++i) {
		//kill 发送 0 信号判断是否存在这个进程  todo:如果实际不是一个机器上,pid 怎么判断
		if (ticket_array[i] != TICKET_AVAIL
			&& (kill(ticket_array[i], 0) == -1) && (errno == ESRCH)) {		
			sprintf(tick, "%d.%d", ticket_array[i], i);
			narrate("freeing", tick, NULL);
			ticket_array[i] = TICKET_AVAIL;
			--num_ticket_out;
		}
	}
	printf("ticket checked \n");
	alarm(RECLAIM_INTERVAL);

}



void narrate(const char* msg1,const char* msg2,struct sockaddr_in* cp) {
	fprintf(stdout, "\tSERVER : %s %s", msg1, msg2);
	if (cp)
		fprintf(stdout, "(%s:%d) ", inet_ntoa(cp->sin_addr), ntohs(cp->sin_port));
	putc('\n', stdout);
}