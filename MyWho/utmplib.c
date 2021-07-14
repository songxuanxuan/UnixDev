#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <utmp.h>
#include <time.h>
#include <string.h>
#include "mywho.h"

static char utmpbuf[NRECS * UTSIZE];
static int num_recs;
static int cur_rec;
static int fd_utmp = -1;

int utmp_open(const char* file_path) {
	fd_utmp = open(file_path, O_RDONLY);
	cur_rec = num_recs = 0;
	return fd_utmp;
}

int utmp_reload() {
	int amt_read;
	amt_read = read(fd_utmp, utmpbuf, NRECS * UTSIZE);
	num_recs = amt_read / UTSIZE;
	cur_rec = 0;
	return num_recs;
}

int utmp_close() {
	if (fd_utmp != -1)
		close(fd_utmp);
}

struct utmp* utmp_next(int isAll) {
	struct utmp* recp;
	if (fd_utmp == -1)
		return NULLUT;
	if (cur_rec == num_recs && utmp_reload() == 0)
		return NULLUT;
	if(isAll == 0)
		while (cur_rec < num_recs &&(recp = (struct utmp*)&utmpbuf[cur_rec++ * UTSIZE])->ut_type != USER_PROCESS);
	else if (isAll == 1 && cur_rec < num_recs)
		while (cur_rec < num_recs && 
			((recp = (struct utmp*)&utmpbuf[cur_rec++ * UTSIZE])->ut_type < INIT_PROCESS 
				&& recp->ut_type > DEAD_PROCESS));
	return recp;

}
int logout_tty(const char* line) {
	int fd;
	struct utmp rec;
	int len = sizeof(struct utmp);
	int retval = -1;
	if ((fd = open(UTMP_FILE, O_RDWR)) == -1)
		return -1;
	while (read(fd, &rec, len) == len) {
		
		if (strncmp(rec.ut_line, line, sizeof(rec.ut_line)) == 0) {
			rec.ut_type = DEAD_PROCESS;
			if ((rec.ut_tv.tv_sec = time(NULL)) != -1)
				if (lseek(fd, -len, SEEK_CUR))
					if (write(fd, &rec, len) == len)
						retval = 0;
			break;
		}
	}
		
	if (close(fd) == -1)
		retval = -1;
	return retval;
}

