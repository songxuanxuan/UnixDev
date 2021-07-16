#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <curses.h>

#define LEFTEDGE 10
#define RIGHTEDGE 30
#define ROW 10


void countdown(int);
int set_ticker(int);
void int_handler(int signum);
void test_sigaction();

int main() {
	test_sigaction();
}


void test_sigaction() {
	struct sigaction newhandler;
	sigset_t blocked;
	char in[128];

	newhandler.sa_handler = int_handler;
	newhandler.sa_flags = SA_RESETHAND | SA_RESTART;

	sigemptyset(&blocked);
	sigaddset(&blocked, SIGQUIT);
	newhandler.sa_mask = blocked;

	if (sigaction(SIGINT, &newhandler, NULL) == -1)
		perror("sigaction");
	else
		while (1) {
			fgets(in, sizeof(in), stdin);
			if (in[0] == 'q')
				kill(0, SIGINT);
			/*if (in[0] == '\\')
				kill(getpid(), SIGQUIT);*/
			printf("input : %s", in);
		}

}

void test_ALRM() {
	signal(SIGALRM, int_handler);
	if (set_ticker(500) == -1)
		perror("set_ticker");
	else
		while (1) {
			pause();
		}
	
}

void int_handler(int signum) {
	printf("called %d \n",signum);
	fflush(stdout);
	sleep(signum);
}


void countdown(int signum){
	static int num = 10;
	printf("%d..", num--);
	fflush(stdout);
	if (num < 0) {
		printf("DONE! \n");
		exit(0);
	}
}


int set_ticker(int n_msecs) {
	struct itimerval new_timeset;
	long n_sec, n_minsec;
	n_sec = n_msecs / 1000;
	n_minsec = (n_msecs % 1000) * 1000L;

	new_timeset.it_interval.tv_sec = n_sec;
	new_timeset.it_interval.tv_usec = n_minsec;

	new_timeset.it_value.tv_sec = n_sec;
	new_timeset.it_value.tv_usec = n_minsec;

	return setitimer(ITIMER_REAL, &new_timeset, NULL);

}

