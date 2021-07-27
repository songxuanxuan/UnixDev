#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "servlib.h"

#define PORTNUM 8088
#define oops(s) do{perror(s);exit(1);}while(0)



void process_rq(char* rq, int fd);

//drop other request info
void read_until_crnl(FILE * fp);

int main() {
	int child_fd,sock_fd;
	FILE* fpin;
	char request[BUFSIZ];
	if ((sock_fd = make_server_socket(PORTNUM)) == -1)
		oops("fail make server");
	if(signal(SIGCHLD, child_wait) == SIG_ERR)
		printf("sigerr");
	while (1)
	{
		child_fd = accept(sock_fd, NULL, NULL);
		fpin = fdopen(child_fd, "r");
		fgets(request, BUFSIZ,fpin);
		if (request[0] == 'q')
			return 0;
		printf("got a request ;request = %s\n",request);
		read_until_crnl(fpin);
		process_rq(request, child_fd);
		fclose(fpin);
	}
	signal(SIGCHLD, SIG_DFL);
}


void read_until_crnl(FILE* fp)
{
	char buf[BUFSIZ];
	while (fgets(buf, BUFSIZ, fp) != NULL && strcmp(buf, "\r\n") != 0);
}



void cannot_do(int fd);
void do_404(const char* args,int fd);
void do_ls(const char* args,int fd);
void do_exec(const char* args,int fd);
void do_cat(const char* args,int fd);
int not_exist(const char* args);
int is_dir(const char* args);
int ends_in_cgi(const char* args);

//dispatch request
void process_rq(char* rq, int fd) {
	char cmd[BUFSIZ], args[BUFSIZ];
	if (fork() != 0)
		return;
	strcpy(args, "./");
	sscanf(rq, "%s %s", cmd, args + 2);
	if (strcmp(cmd, "GET") != 0)
		cannot_do(fd);
	else if (not_exist(args))
		do_404(args, fd);
	else if (is_dir(args))
		do_ls(args, fd);
	else if (ends_in_cgi(args))
		do_exec(args, fd);
	else
		do_cat(args, fd);
}
void header(FILE* fp, char* content_type)
{
	fprintf(fp, "HTTP/1.0 200 OK\r\n");
	if (content_type)
		fprintf(fp, "Content-type: %s\r\n", content_type);
}

int ends_in_cgi(const char* args)
{
	char* cp;
	if ((cp = strrchr(args, '.')) != NULL)	//search from rear
		return (strcmp(cp + 1, "cgi") == 0);
	return 0;
}


int is_dir(const char* args)
{
	struct stat info;
	return (stat(args, &info) != -1 && S_ISDIR(info.st_mode));
}



int not_exist(const char* args)
{
	struct stat info;
	return (stat(args, &info) == -1) ;
}


void cannot_do(int fd)
{
	FILE* fp = fdopen(fd, "w");
	fprintf(fp, "Http/1.0 501 NOT Implemented\r\n");
	fclose(fp);
	exit(1);
}


void do_404(const char* args, int fd)
{
	FILE* fp = fdopen(fd, "w");
	fprintf(fp, "HTTP/1.0 404 NOT FOUND\r\n");
	fprintf(fp, "the item you request: %s\r\n is not found \r\n",args);
	fclose(fp);
	exit(1);
}


void do_ls(const char* args, int fd)
{
	FILE* fp = fdopen(fd, "w");
	header(fp, "text/plain");
	fprintf(fp, "\n\r");
	fflush(fp);
	dup2(fd, 1);	//stdout
	dup2(fd, 2);	//stderr
	close(fd);
	execlp("ls", "ls", args, NULL);
	oops("execlp ls error");
}


void do_exec(const char* args, int fd)
{
	FILE* fp = fdopen(fd, "w");
	header(fp, "NULL");
	fflush(fp);
	dup2(fd, 1);	//stdout
	dup2(fd, 2);	//stderr
	close(fd);
	execlp(args, args, NULL);
	oops("execlp ls error");
}

char* file_type(char* f) {
	char* cp;
	if ((cp = strrchr(f, '.')) != NULL)
		return cp + 1;
	return "";
}

void do_cat(const char* args, int fd)
{
	char* extension = file_type(args);
	FILE* fpsock, * fpfile;
	char ch;

	fpsock = fdopen(fd, "w");
	fpfile = fopen(args, "r");
	if (fpsock != NULL && fpfile != NULL) {
		header(fpsock, extension);
		fprintf(fpsock, "\n\r");
		while ((ch = getc(fpfile)) != EOF)
			putc(ch, fpsock);
		fclose(fpsock);
		fclose(fpfile);
	}
	exit(0);
}

