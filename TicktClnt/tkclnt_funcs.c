#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include "../DGramServ/dgram.h"


#define HOST "ubuntu"

#ifdef DEBUG
#define PORT 4444
#else
#define PORT 2020
#endif // DEBUG



#define DFT_MSG "HELLO SERVER"
#define MSGLEN 128
#define HOSTLEN 512

static int pid = -1;
static int sd = -1;
static struct sockaddr_in serv_addr;
static char ticket_buf[MSGLEN];
static int have_ticket = 0;
static int serv_alen;



char* do_transaction(char* msg);
void narrate(char* msg1, char* msg2);

void set_up() {
	char hostname[HOSTLEN];
	pid = getpid();
	sd = make_dgram_client_socket();
	if (sd == -1)
		oops("create socket failed");
	gethostname(hostname, HOSTLEN);
	make_internet_address(hostname, PORT, &serv_addr);
	serv_alen = sizeof(serv_addr);
}

void shut_down() {
	close(sd);
}

int get_ticket() {
	char* respons;
	char buf[MSGLEN+5];
	if (have_ticket)
		return 0;
	sprintf(buf, "HELO %d", pid);
	if ((respons = do_transaction(buf)) == NULL)
		return -1;
	if (strncmp(respons, "TICK", 4) == 0) {
		strcpy(ticket_buf, respons + 5);
		have_ticket = 1;
		narrate("got ticket", ticket_buf);
		return 0;
	}
	if (strncmp(respons, "FAIL", 4) == 0)
		narrate("get tick fail", respons);
	else
		narrate("unknown msg", respons);
	return -1;

}

int release_ticket() {
	char* respons;
	char buf[MSGLEN +5];

	if (!have_ticket)
		return 0;
	sprintf(buf, "GBYE %s", ticket_buf);
	if ((respons = do_transaction(buf)) == NULL)
		return -1;
	if (strncmp(respons, "THNX", 4) == 0) {
		have_ticket = 0;
		narrate("release ticket OK", ticket_buf);
		return 0;
	}
	else if (strncmp(respons, "FAIL", 4) == 0)
		narrate("release failed", respons);
	else
		narrate("unknown msg", respons);
	return -1;
}


#define perror_rt(x) do{perror(x);return NULL;}while(0)
char* do_transaction(char* msg) {
	struct sockaddr_in retaddr;
	static char buf[MSGLEN];
	socklen_t salen = sizeof(retaddr);

	if (sendto(sd, msg, MSGLEN, 0,(struct sockaddr*) &serv_addr, serv_alen) == -1)
		perror_rt("send to");

	//这里中断会产生 recv 一直等待的情况
	if (recvfrom(sd, buf, MSGLEN, 0, (struct sockaddr*)&retaddr, &salen) == -1)
		perror_rt("rev from");
	return buf;
}

int check_validate() {
	char buf[MSGLEN + 5];
	char* respons;
	if (!have_ticket)
		return -1;
	sprintf(buf, "VALD %s", ticket_buf);
	if ((respons = do_transaction(buf)) == NULL)		//这里重复使用static buf 应该不影响
		return -1;
	if(strncmp(respons,"GOOD",4) != 0){
		narrate("valid ticket now", ticket_buf);
		return 0;
	}
	have_ticket = 0;
	return -1;
}


void narrate(char* msg1, char* msg2) {
	fprintf(stdout, "CLIENT[%d] : %s %s\n", pid, msg1, msg2);
}