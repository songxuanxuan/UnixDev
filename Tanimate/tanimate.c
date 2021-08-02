#include <stdio.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <curses.h>
#include <pthread.h>


#define MAXTHREAD 10
#define TIMEUNIT 20000

typedef struct {
	char* str;
	int row;
	int dir;
	int delay;
	int col;
} mov_unit;

#define oops(x) do{perror(x);exit(1);}while (0)

void* animate(void* unit);
int set_up(int strnum, char* str[], mov_unit units[]);

void* agent(void* num);

pthread_mutex_t mv_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t flag = PTHREAD_COND_INITIALIZER;
mov_unit* up = NULL;

int main(int ac, char* av[]) {
#if 1
	ac = 4;
	av[1] = "123";
	av[2] = "3333";
	av[3] = "asdfasdf";
#endif
	if (ac == 1)
		oops("params error");
	pthread_t thrds[MAXTHREAD];
	pthread_t agent_thrd;
	mov_unit units[MAXTHREAD];
	
	int units_num = set_up(ac - 1, av + 1, units);
	pthread_mutex_lock(&mv_mutex);
	if (pthread_create(&agent_thrd, NULL, agent, &units_num) == -1) {
		perror("create agent error");
		endwin();
		exit(1);
	}

	for (int i = 0; i < units_num; i++) {
		if (pthread_create(&thrds[i], NULL, animate, &units[i]) == -1) {
			perror("create pthread error");
			endwin();
			exit(1);
		}
	}
	int ch;
	while (1)
	{
		ch = getch();
		if (ch == 'q')	break;
		if (ch == ' ')
			for (int i = 0; i < units_num; i++)
				units[i].dir = -units[i].dir;
		if (ch >= '0' && ch <= '9') {
			int i = ch - '0';
			if (i < units_num)
				units[i].dir = -units[i].dir;
		}

	}

	pthread_mutex_lock(&mv_mutex);
	for (int i = 0; i < units_num; i++)
		pthread_cancel(thrds[i]);
	endwin();
	return 0;
}

int set_up(int strnum, char* str[], mov_unit units[])
{
	int num = strnum > MAXTHREAD ? MAXTHREAD : strnum;
	/*assign units*/
	for (int i = 0; i < num; i++)
	{
		units[i].str = str[i];
		units[i].delay = 1 + (rand() % 10);
		units[i].dir = ((rand() % 2) ? 1 : -1);
		units[i].row = i;
		units[i].col = 0;
	}
	/*set curses*/

	initscr();
	crmode();	//没有缓存,直接显示
	noecho();
	clear();
	mvprintw(LINES - 1, 0, "'q' to quit ; 0-%d to bounce", num);

	return num;
}



/*****
		P1
		IF() P2
		ACTION
		V2
		V1
*****/
void* animate(void* unit)
{
	mov_unit* info = (mov_unit *) unit;
	int len = strlen(info->str) + 2;
	info->col = rand() % (COLS - len);
	while (1)
	{
		usleep(info->delay * TIMEUNIT);
		info->col += info->dir;
		if (info->col == 0 && info->dir == -1)
			info->dir = 1;
		else if (info->col + len == COLS - 1 && info->dir == 1)
			info->dir = -1;

		//P1
		pthread_mutex_lock(&mv_mutex);
		/*if 中间变量agent还在使用中*/
		//P2
		if (up != NULL)
			pthread_cond_wait(&flag, &mv_mutex);
		
		up = info;
		//V2
		pthread_cond_signal(&flag);
		//V1
		pthread_mutex_unlock(&mv_mutex);
		
	}
}

/****
		P2
		ACTION
		V2
****/
void* agent(void* pnum)
{
	int* np =(int*) pnum;
	
	while (*np)
	{
		

		//P2
		pthread_cond_wait(&flag, &mv_mutex);
		move(up->row, up->col);
		addch(' ');
		addstr(up->str);
		addch(' ');

		move(LINES - 1, COLS - 1);
		refresh();
		up = NULL;
		//V2
		pthread_cond_signal(&flag);
	}
	return NULL;

}


