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
	//the write semaphore
	actions[0].sem_num = 1;
	actions[0].sem_flg = SEM_UNDO;
	actions[0].sem_op = 0;
	//the read semaphore
	actions[1].sem_num = 0;
	actions[1].sem_flg = SEM_UNDO;
	actions[1].sem_op = +1;

	if (semop(sem_id, actions, 2) == -1)
		oops("semop:lock");
}

void release_lock(int sem_id)
{
	struct sembuf actions[1];
	actions[0].sem_num = 0;
	actions[0].sem_flg = SEM_UNDO;
	actions[0].sem_op = -1;
	if (semop(sem_id, actions, 1) == -1)
		oops("semop:unlock");
}


int main()
{

	time_t now;
	seg_id = shmget(TIME_MEM_KEY, 100, IPC_CREAT | 0777);

	char* mem_ptr = shmat(seg_id, NULL, 0);

	semset_id = semget(TIME_SEM_KEY, 2, 0);	

	wait_and_lock(semset_id);
	
	printf("get time is %s \n", mem_ptr);

	release_lock(semset_id);


}