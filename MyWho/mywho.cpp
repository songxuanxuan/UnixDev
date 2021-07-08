#include <stdio.h>
#include <utmp.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define SHOWHOST

void show_info(struct utmp*);
void show_time(long time);
int main() {
	struct utmp current_record;
	int utmpfd;
	int rec_len = sizeof(current_record);

	if ((utmpfd = open(UTMP_FILE, O_RDONLY)) == -1) {
		perror(UTMP_FILE);
		exit(1);
	}
	while (read(utmpfd, &current_record, rec_len) == rec_len)
		show_info(&current_record);
	close(utmpfd);
	return 0;
}
void show_info(struct utmp* record)
{
	if (record->ut_type != USER_PROCESS)
		return;
	printf("%-8s ", record->ut_user);
	printf("%-8s ", record->ut_line);
	show_time(record->ut_tv.tv_sec);
#ifdef SHOWHOST
	printf("%s ", record->ut_host);
#endif // SHOWHOST
	printf("\n");

}
void show_time(long time) {
	char* tp;
	time_t time_sec = time;
	tp = ctime(&time_sec);
	printf("%12s ", tp + 4);
}
