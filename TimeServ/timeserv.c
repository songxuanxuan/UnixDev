#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <time.h>
#include <strings.h>	//for bzero
#include <netdb.h>	//for hostent ,gethostbyname
#include <netinet/in.h>
#include <string.h>
#include <ctype.h>

#define HOSTLEN 256
#define PORTNUM 13000
#define oops(s) do{perror(s);exit(1);}while(0)

void server();
void client(const char* ad, const char* port);

#define CLIENT 0

int main(int ac, char* av[]) {

#if CLIENT
	av[1] = "c";
	ac = 2;

#endif
#if !CLIENT
	av[1] = "s";
	ac = 2;
#endif
	if (ac != 2)
		oops("param incorrect");
	else
		if (av[1][0] == 's')
			server();
		else if (av[1][0] == 'c') {
#if CLIENT
			client("ubuntu", "13000");
			exit(1);
#endif
			char ad[HOSTLEN];
			char port[HOSTLEN];
			printf("input server:port\n");
			if (scanf("%s\n%s", ad, port) != 2)
				oops("syntax");
			client(ad, port);
		}
		else
			oops("param incorrect");

}

void sanitize(char* str) {
	char* sp, * dest;
	for (sp = dest = str; *sp; ++sp) {
		if (*sp == '/' || isalnum(*sp))
			*dest++ = *sp;
	}
	*dest = '\0';
}
void server() {
	struct sockaddr_in saddr;
	struct hostent* hp;
	char hostname[HOSTLEN];
	char buf[BUFSIZ];
	char cmd[BUFSIZ];
	char ch;
	int sock_id, sock_fd;
	FILE* sock_fpo, * sock_fpi;
	FILE* pipe_fp;
	time_t nowtime;

	sock_id = socket(PF_INET, SOCK_STREAM, 0);
	if (sock_id == -1)
		oops("socket");
	bzero((void*)&saddr, sizeof(saddr));

	gethostname(hostname, HOSTLEN);
	hp = gethostbyname(hostname);
	bcopy((void*)hp->h_addr, (void*)&saddr.sin_addr, hp->h_length);		//todo how to cast char* to int

#if 0	//cannot print hp->h_addr ?
	char cp[HOSTLEN];
	strcpy(cp, hp->h_addr);
	cp[hp->h_length] = '\0';
	puts(cp);
	printf("-> %x\n", saddr.sin_addr.s_addr);
#endif

	saddr.sin_port = htons(PORTNUM);
	saddr.sin_family = AF_INET;

	if (bind(sock_id, (struct sockaddr*)&saddr, sizeof(saddr)) != 0)
		oops("bind");
	if (listen(sock_id, 1) != 0)
		oops("listen");
	while (1) {

		//todo:receive next input
		sock_fd = accept(sock_id, NULL, NULL);
		if (sock_fd == -1)
			oops("accept");
		printf("got a call\n");

		if ((sock_fpi = fdopen(sock_fd, "r")) == NULL)
			oops("fdopen read");
		if ((sock_fpo = fdopen(sock_fd, "w")) == NULL)
			oops("fdopen write");
		nowtime = time(NULL);
		fprintf(sock_fpo, "The time is %s", ctime(&nowtime));
		fflush(sock_fpo);
		while (1)
		{
			if (fgets(buf, BUFSIZ - 5, sock_fpi) == NULL)		//why set -5
				oops("fgets sock input");
			if (buf[0] == 'q')
				oops("quit");
			sanitize(buf);
			sprintf(cmd, "ls %s", buf);
			if ((pipe_fp = popen(cmd, "r")) == NULL)
				oops("popen");

			//todo: get all include space
#if 0
			if (fgets(buf, BUFSIZ - 5, pipe_fp) == NULL)
				oops("fgets cmd back");

			if (fputs(buf, sock_fpo) == -1)
				oops("fputs sock out");
#endif

			while ((ch = getc(pipe_fp)) != EOF)
				putc(ch, sock_fpo);
			fflush(sock_fpo);
		}




		fclose(sock_fpo);
		fclose(sock_fpi);
		pclose(pipe_fp);
	}
}


void client(const char* ad, const char* port) {
	struct sockaddr_in servaddr;
	struct hostent* hp;
	int sock_id;
	char msg[BUFSIZ];
	int messlen;

	sock_id = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_id == -1)
		oops("socket");

	bzero(&servaddr, sizeof(servaddr));
	hp = gethostbyname(ad);
	bcopy(hp->h_addr, (struct sockaddr*)&servaddr.sin_addr, hp->h_length);
	servaddr.sin_port = htons(atoi(port));	//host to net short, covert to net byte order
	servaddr.sin_family = AF_INET;

	if (connect(sock_id, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0)
		oops("connect");
	if (read(sock_id, msg, BUFSIZ) == -1)
		oops("read preinfo");
	puts(msg);
	while (printf("ls : "), fgets(msg, BUFSIZ, stdin) != NULL && msg[0] != 'q')
	{

		if (write(sock_id, msg, strlen(msg)) == -1)
			oops("write sock out");
		messlen = read(sock_id, msg, BUFSIZ);
		if (messlen == -1)
			oops("read sock");
		if (write(1, msg, messlen) != messlen)
			oops("write std");
	}

	close(sock_id);
}