#include <stdio.h>
#include <unistd.h>

#define oops(s) do{perror(s);exit(1);}while(0)

int main(int ac, char* av[]) {
	int pipefd[2];
	int pid;
	if (pipe(pipefd) == -1)
		oops("pipe");
	if ((pid = fork()) == -1)
		oops("fork");
	else if (pid > 0) {
		close(pipefd[1]);
		if (dup2(pipefd[0], 0) == -1)	
			//0 is set with fd[0],and now 0 and &fd[0] are both fd[0](will be stdout in child)
			oops("redirect input");
		close(pipefd[0]);
		execlp(av[2], av[2], NULL);
		oops(av[2]);
	}
	else {
		close(pipefd[0]);
		if (dup2(pipefd[1], 1) == -1)
			oops("redirect output");
		close(pipefd[1]);
		execlp(av[1], av[1], NULL);
		oops(av[1]);
	}
}