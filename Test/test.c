#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

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

int main() {
	test_forks(3);
	getchar();
}