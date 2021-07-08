#ifndef _MYWHO_H
#define _MYWHO_H

#define NRECS 16
#define NULLUT ((struct utmp*)NULL)
#define UTSIZE (sizeof(struct utmp))

void show_info(struct utmp*);
void show_time(time_t time);

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	struct utmp* utmp_next(int isAll);
	int utmp_open(const char*);
	int utmp_reload();
	int utmp_close();
	int logout_tty(const char* line);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _MYWHO_H__
