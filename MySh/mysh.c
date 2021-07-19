#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ARG_LEN 64
#define MAX_ARGS 20

void execute(char* arglist[]);

char* makestring(char* buf);

int main() {
	char* arglist[MAX_ARGS + 1];  //end with NULL
	char argbuf[ARG_LEN];
	int i;
	for (i = 0; i < MAX_ARGS; ) {
		printf("\n arg[%d]? ", i);
		if (fgets(argbuf, ARG_LEN, stdin) && *argbuf != '\n') {
			if (argbuf[0] == 'q')
				exit(0);
			arglist[i++] = makestring(&argbuf);
		}
		else if (*argbuf == '\n') {
			if(i <= 0)
				continue;
			arglist[i] = NULL;
			execute(arglist);
			for (--i; i >= 0; --i) {
				free(arglist[i]);
			}
			++i;  //set i to 0
		}
		else {
			perror("input");
			exit(1);
		}
	}
	return 0;

}



void execute(char* arglist[])
{
	int pid, exit_stat;
	pid = fork();
	if (pid == -1) {
		perror("fork");
		exit(1);
	}
	else if (pid == 0) {
		execvp(arglist[0], arglist);
		perror("execvp");
		exit(1);
	}
	else {
		wait(&exit_stat);
		printf("child( %d ) exited with status %d, %d", pid, exit_stat >> 8, exit_stat & 0377);
	}

}


char* makestring(char* buf)
{
	buf[strlen(buf) - 1] = '\0';	//½« \n »»³É \0
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
