#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

union semun
{
	int val;
	struct semid_ds* buf;
	ushort* array;
};

#define TIME_MEM_KEY 99
#define TIME_SEM_KEY 99001

#define oops(x) do{perror(x);exit(1);}while (0)

int seg_id, semset_id;

void set_sem_value(int id, int num, int val)
{
	union semun initval;
	initval.val = val;
	if (semctl(id, num, SETVAL, initval) == -1)
		oops("semctl");
}

void wait_and_lock(int sem_id)
{
	struct sembuf actions[2];
	//read for
	actions[0].sem_num = 0;
	actions[0].sem_flg = SEM_UNDO;
	actions[0].sem_op = 0;
	//for write
	actions[1].sem_num = 1;
	actions[1].sem_flg = SEM_UNDO;
	actions[1].sem_op = +1;

	if (semop(sem_id, actions, 2) == -1)
		oops("semop:lock");
}

void release_lock(int sem_id)
{
	struct sembuf actions[1];
	actions[0].sem_num = 1;
	actions[0].sem_flg = SEM_UNDO;
	actions[0].sem_op = -1;
	if (semop(sem_id, actions, 1) == -1)
		oops("semop:unlock");
}

void clean_up(int n)
{
	shmctl(seg_id, IPC_RMID, NULL);
	semctl(semset_id, 0, IPC_RMID, NULL);
}

int main()
{
	time_t now;
	seg_id = shmget(TIME_MEM_KEY, 100, IPC_CREAT | 0777);

	char* mem_ptr = shmat(seg_id, NULL, 0);

	semset_id = semget(TIME_SEM_KEY, 2, (0666 | IPC_CREAT | IPC_EXCL));		//IPC_EXCL : fail if exist

	set_sem_value(semset_id, 0, 0);
	set_sem_value(semset_id, 1, 0);

	signal(SIGINT, clean_up);

	for (int i = 0;i<60;i++)
	{
		time(&now);

		printf("\tlocking\n");
		wait_and_lock(semset_id);
		printf("\tupdating\n");
		strcpy(mem_ptr, ctime(&now));
		sleep(5);
		release_lock(semset_id);
		printf("\tunlocked\n");
		sleep(1);
	}
	clean_up(0);

}