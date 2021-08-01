#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include "tservlib.h"

pthread_mutex_t bytes_mutex;


void set_up(pthread_attr_t* attrp) {
	pthread_attr_init(attrp);
	pthread_attr_setdetachstate(attrp,PTHREAD_CREATE_DETACHED);
	serv_requset_num = 0;
	serv_bytes_num = 0;
}

void process_rq(char* req,int fd);
void handle_rest(FILE* fpin);

void* handle_call(void* fdp)
{
	FILE* fpin;
	char request[BUFSIZ];
	int fd = *(int*)fdp;
	free(fdp);
	fpin = fdopen(fd, "r");
	fgets(request, BUFSIZ, fpin);
	printf("got request: %s", request);
	handle_rest(fpin);
	process_rq(request,fd);
	fclose(fpin);
}

void handle_rest(FILE* fpin)
{
	char ch[BUFSIZ];
	while (fgets(ch,BUFSIZ,fpin) != NULL && strcmp(ch,"\r\n") != 0);
}


/************************************************************************/
/*           处理请求                                                           */
/************************************************************************/

void sanitize(char* str);
void no_imply(int);
int is_dir(const char* args);
int not_exist(const char* args);
void do_404(int);
void do_ls(int fd, const char* args);
void do_exec(int fd, const char* args);
void do_status(int fd);
int http_reply(int fd, FILE** fpp, int code, char* msg, char* type, char* content);


//implement
void process_rq(char* req,int fd)
{
	char cmd[BUFSIZ];
	char args[BUFSIZ];
	if (sscanf(req, "%s %s ", cmd, args) != 2)
		oops("request syntax error");
	sanitize(args);
	printf("sanitized : %s \n", args);
	if (strcmp(cmd, "GET") != 0)
		no_imply(fd);
	else
		if (strcmp(args, "status") == 0)
			do_status(fd);
		else if (not_exist(args))
			do_404(fd);
		else if (is_dir(args))
			do_ls(fd, args);
		else
			do_exec(fd, args);

}

void do_status(int fd)
{
	FILE* fp;
	http_reply(fd, &fp, 200, "OK", "status", NULL);
	fprintf(fp, "http request for %d times\r\n", serv_requset_num);
	fprintf(fp, "http request for %d bytes\r\n", serv_bytes_num);
	fclose(fp);
}



char* get_filetype(const char* args)
{
	char* cp;
	if ((cp = strrchr(args, '.')) != NULL)
		return cp + 1;
	return "";
}

void do_exec(int fd, const char* args)
{
	char* type = "text/plain";
	char* extense = get_filetype(args);
	if (strcmp(extense, "html") == 0)
		type = "text/html";
	else if (strcmp(extense, "gif") == 0)
		type = "image/gif";
	FILE* fpsock;
	int bytes_num = http_reply(fd, &fpsock, 200, "OK", type, NULL);
	FILE* fpfile = fopen(args, "r");
	char ch;
	if (fpfile != NULL) {
		while ((ch = getc(fpfile)) != EOF)
		{
			//todo : 处理html显示
				putc(ch, fpsock);
				bytes_num++;
			
		}
		fclose(fpfile);
	}
	if(fpsock)
		fclose(fpsock);	//todo 重复请求产生异常

	pthread_mutex_lock(&bytes_mutex);
	serv_bytes_num += bytes_num;
	pthread_mutex_unlock(&bytes_mutex);
}



void do_ls(int fd, const char* args)
{
	DIR* dirp;
	struct dirent* direntp;
	int bytes_num = 0;
	FILE* fp;
	bytes_num = http_reply(fd, &fp, 200, "OK", "text/plain", "listing of directory \n");
	dirp = opendir(args);
	if (dirp != NULL) {
		while (direntp = readdir(dirp))
		{
			bytes_num += fprintf(fp, "%s ", direntp->d_name);
		}
		closedir(dirp);
	}
	fclose(fp);
	pthread_mutex_lock(&bytes_mutex);
	serv_bytes_num += bytes_num;
	pthread_mutex_unlock(&bytes_mutex);

}


void sanitize(char* str) {
	char* sp, *dest;
	sp = dest = str;
	while (*sp) {
		if (strncmp(sp, "/../", 4) == 0)
			sp += 4;
		else if (strncmp(sp, "//", 2) == 0)
			sp++;
		else if (*sp == '/' ||*sp == '.'|| *sp == '_'|| *sp == '-' || isalnum(*sp))
			*dest++ = *sp++;
		else
			sp++;
	}
	*dest = '\0';

	if (str[0] == '\0' || strcmp(str, "./") == 0 || strcmp(str, "./..") == 0 )
		strcpy(str, ".");
	
}

int http_reply(int fd, FILE** fpp, int code, char* msg, char* type, char* content) {
	FILE* fp = fdopen(fd, "w");
	int bytes_num = 0;
	if (fp == NULL)
		oops("fdopen failure");
	bytes_num = fprintf(fp, "HTTP/1.0 %d %s\r\n", code, msg);
	bytes_num += fprintf(fp, "content_type: %s \r\n", type);
	if (content)
		bytes_num += fprintf(fp, "%s\r\n", content);
	fflush(fp);
	if (fpp)
		*fpp = fp;
	else
		fclose(fp);
	return bytes_num;

}

void no_imply(int fd)
{
	http_reply(fd, NULL, 501, "not implemented", "text/plain", NULL);
}
void do_404(int fd)
{
	http_reply(fd, NULL, 404, "not found", "text/plain", NULL);
}

int is_dir(const char* args)
{
	struct stat info;
	return (stat(args, &info) != -1 && S_ISDIR(info.st_mode));
}



int not_exist(const char* args)
{
	struct stat info;
	return (stat(args, &info) == -1);
}







