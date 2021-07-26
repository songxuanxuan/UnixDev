#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include "sh/varlib.h"


void test_forks(int n) {
	int ex_stat;
	int pids[n];
	int i;
	for ( i = 0; i < n; ++i)
	{
		printf("%d ----\n" , i);
		pids[i] = fork();
		if (pids[i] == -1)
			perror("fork");
		else if (pids[i] == 0) {
			printf("child: %d \n", getpid());
			sleep(3);
			exit(1);
		}
	}
	int pid;
	while (pid = wait(&ex_stat)) {
		int high = ex_stat >> 8;
		printf("patent( %d ) child( %d ) exited : %x \n", getpid(), pid, ex_stat);
		if (--i == 0)
			break;
	}
	
		
}
char* new_string(char* name, char* val) {
	char* ret = (char*)malloc(strlen(name) + strlen(val) + 2);
	if (ret != NULL)
		sprintf(ret, "%s=%s", name, val);	//do end with \0
	return ret;
}

#define CHILD_MSG "I WANT A COOKIE\n"
#define PARENT_MSG "TESTING...\n"
#define oops(s) {perror(s);exit(1);}
void test_pipe() {
	int pipefd[2];
	int len;
	char buf[BUFSIZ];
	if (pipe(pipefd) == -1)
		oops("pipe");
	switch (fork())
	{
	case -1:
		oops("fork");
	case 0:
		len = strlen(CHILD_MSG);
		while (1) {
			if (write(pipefd[1], CHILD_MSG, len) != len)
				oops("wirte");
			sleep(3);
		}
		break;
	default:
		len = strlen(PARENT_MSG);
		while (1)
		{
			if (write(pipefd[1], PARENT_MSG, len) != len)
				oops("parent wirte");
			sleep(2);
			int read_len = read(pipefd[0], buf, BUFSIZ);
			if(read_len <= 0)
				break;
			write(1, buf, read_len);
		}
		break;
	}
}
int main() {
	char a[10], b[10];
	int i = scanf("%s\n%s", a, b);
	int j = 1;
}