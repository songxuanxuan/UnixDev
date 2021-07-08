#include <stdio.h>
#include <utmp.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>


#include "mywho.h"

#define SHOWHOST 1

int main(int ac,char* av[]) {
	int isAll = 0;
	char* ut_file = UTMP_FILE;
	if (ac == 2) {
		if (*av[1] == 'a')
			isAll = 1;
		if (*av[1] == 'w')
			ut_file = WTMP_FILE;
	}
	struct utmp *utbufp;
	if (utmp_open(ut_file) == -1) {
		perror(ut_file);
		exit(1);
	}
	while ((utbufp = utmp_next(isAll)) != NULLUT)
		show_info(utbufp);
	utmp_close();
	return 0;
}


void show_info(struct utmp* record)
{
	printf("%-8s ", record->ut_user);
	printf("%-8s ", record->ut_line);
	show_time(record->ut_tv.tv_sec);
#ifdef SHOWHOST
	printf("%-10s ", record->ut_host);
#endif // SHOWHOST
	printf("%d", record->ut_type);
	printf("\n");

}
void show_time(time_t time) {
	char* tp;
	time_t time_sec = time;
	tp = ctime(&time_sec);
	tp[24] = 0;
	printf("%-14s ", tp + 4);
}
