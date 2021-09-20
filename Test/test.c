#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/time.h>
#include "sh/varlib.h"


void child_signal(int signum) {
	while (waitpid(-1, NULL, WNOHANG) > 0);
	printf("--wait \n");
}

void test_signal_fork() {
	int i = fork();
	signal(SIGCHLD, child_signal);
	while (1) {
		if (i == 0) {
			printf("child ... \n");
			sleep(3);
			execlp("ls", "ls", "-l", "/home", NULL);
			perror("execlp error");
		}
		sleep(2);
		//printf("pid : %d i : %d\n", getpid(),i);
	}
	signal(SIGCHLD, SIG_DFL);
}

void test_forks(int n) {
	int ex_stat;
	int pids[n];
	int i;
	for (i = 0; i < n; ++i)
	{
		printf("%d ----\n", i);
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
			if (read_len <= 0)
				break;
			write(1, buf, read_len);
		}
		break;
	}
}
void showdata(const char* name,int fd) {
	char buf[BUFSIZ];
	int n;
	printf("%s \n", name);
	n = read(fd, buf, BUFSIZ);
	if (n != -1)
		write(1, buf, n);
	write(1, "\n", 1);
}

//select: 非阻塞等待read
void select_demo() {
	char* args[2] = { "/dev/tty", "/dev/input/mice" };
	int fd1 = open(args[0], O_RDONLY);
	int fd2 = open(args[1], O_RDONLY);
	if (fd2 == -1)
		perror("mouse error \n");
	int maxfd = 1 + (fd1 > fd2 ? fd1 : fd2);
	struct timeval timeout;
	int retval;
	fd_set readfds;
	while (1)
	{
		//每次select都重置参数吗?
		FD_ZERO(&readfds);
		FD_SET(fd1, &readfds);
		FD_SET(fd2, &readfds);

		timeout.tv_sec = 5;
		timeout.tv_usec = 0;

		retval = select(maxfd, &readfds, NULL, NULL, &timeout);
		if (retval > 0) {
			if (FD_ISSET(fd1, &readfds))
				showdata(args[0], fd1);
			else if (FD_ISSET(fd2, &readfds))
				showdata(args[1], fd2);
		}
		else
			printf("time out \n");
	}

}
int main() {
	char name[128];
	char pwd[128];
	scanf("%[^&]%*[^a-z0-9A-Z]%[^&]", name, pwd);
	getchar();
}