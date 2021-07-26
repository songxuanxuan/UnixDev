#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "varlib.h"

#define ARG_LEN 64
#define MAX_ARGS 20
#define BUFSIZ (ARG_LEN+1)*MAX_ARGS
#define FLAGE ">>"

extern char** environ;

int execute(char** arglist);

char* makestring(char* buf);
char* next_cmd(char*, FILE*);
void free_list(char**);
char** splitline( char* line);
int process(char** args);
void setup();

int main() {
	char** arglist;  //end with NULL
	char* argbuf;
	char* flag = FLAGE;
	int i;
	setup();
	while ((argbuf = next_cmd(flag,stdin)) != NULL ) {
		if ((arglist = splitline(argbuf)) != NULL) {
			process(arglist);
			free_list(arglist);
		}
		free(argbuf);
	}
	return 0;

}


void setup() {
	VLenviron2table(environ);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
}
void free_list(char** list) {
	char** lcp = list;
	while (*list != NULL) {
		free(*list++);
	}
	free(lcp);
}

void replace_symbol(char** args) {
	for (int i = 0; i < MAX_ARGS && args[i] != NULL; ++i) {
		if (args[i][0] == '$') {
			char* symbol = VLlookup(args[i] + 1);		//return " " for NULL
			args[i] = (char*)realloc(args[i], strlen(symbol));
			strcpy(args[i], symbol);
		}
			
	}
}

int execute(char** arglist)
{
	int pid, exit_stat = -1;
	pid = fork();
	if (pid == -1) {
		perror("fork");
		exit(1);
	}
	else if (pid == 0) {
		char** tmp = VLtable2environ();
		if (tmp != NULL)
			environ = tmp;
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		replace_symbol(arglist);
		execvp(arglist[0], arglist);
		perror("execvp");
		exit(1);
	}
	else {
		wait(&exit_stat);
	}
	return exit_stat;
}


char* makestring(char* buf)
{
	buf[strlen(buf) - 1] = '\0';	//将 \n 换成 \0
	char* ch = malloc(strlen(buf));
	if (ch == NULL) {
		perror("malloc");
		exit(1);
	}
	else {
		strcpy(ch, buf);
		return ch;
	}

}

char* next_cmd(char* flag, FILE* fp) {
	printf("%s", flag);
	char* ret = (char*)malloc(BUFSIZ + 1);
	fgets(ret, BUFSIZ, fp);
	if (strcmp(ret, "quit\n") == 0) {
		free(ret);
		exit(0);
	}
	ret[strlen(ret) - 1] = '\0';
	return ret;
}

#define is_delim(x) ((x)==' '||(x)=='\t')

char* cpstr(const char* sp, int len) {
	char* ret = malloc(len + 1);
	strncpy(ret, sp, len);
	ret[len] = '\0';
	return ret;
}


char** splitline(char* line) {
	if (line == NULL)
		return NULL;
	char* cp = line;
	char** args = (char**)malloc(sizeof(char*)*MAX_ARGS);
	int ac = 0;
	int bufspace = MAX_ARGS;
	while (*cp != '\0') {
		while (is_delim(*cp))
			++cp;
		if(*cp == '\n')
			break;

		if (ac + 1 >= bufspace) {
			bufspace = 2 * bufspace;
			if ((args = realloc(args, bufspace)) == NULL) {
				perror("realloc");
				exit(1);
			}
			
		}
		//到这里必然是有字符,跳过当前位置判断
		int len = 1;
		while (*++cp != '\0' && !is_delim(*cp))		//注意 && 和 宏 对++的影响
			++len;
		args[ac++] = cpstr(cp - len,len);
	}
	args[ac] = NULL;
	return args;
}