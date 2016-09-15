#include <unistd.h>
#include <stdio.h>
#include <sys/procdesc.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include <iostream>

#include <pthread.h>

#include "lwc.h"
#include "bench.h"

pthread_t child_thr;
pthread_cond_t cond;
pthread_mutex_t mutex;


#define IDX_ORIG 0
#define IDX_SUPER 1
#define IDX_CHLD 2

using std::cerr;
using std::endl;

int sem_id;
int my_id = 0;

#define Semop(a,b,c) semop(a,b,c)


#define COUNT 1000000

void* child_work_function(void *arg) {
	
	struct timespec start, end;

	int *buf = (int*)arg; //not really private, just didn't want to rename things
	int c = 0;
	struct sembuf sem;
	sem.sem_flg = 0;

	clock_gettime(CLOCK_REALTIME, &start);
	while(c < COUNT) {
		sem.sem_num = 1;
		sem.sem_op = -1;
		Semop(sem_id, &sem, 1);
		c++;
		//cerr << "do child " << c << endl;;
		sem.sem_num = 0;
		sem.sem_op = 1;
		Semop(sem_id, &sem, 1);
	}


	clock_gettime(CLOCK_REALTIME, &end);
	struct timespec res = diff(&start, &end);
	cerr << "Total time for " << COUNT << " switches is  " << res.tv_sec << " seconds " << res.tv_nsec << " nanoseconds" << endl;
	semctl(sem_id, 0, IPC_RMID);

	/* so dtrace can do ustacks!! */
	exit(0);
	//exit(0);
	//pthread_mutex_lock(&mutex); /* just so we don't get too many extra counts */
	//for(;;) {sleep(1); }

	return NULL;
}


void parent_work_function() {
	struct sembuf sem;
	sem.sem_flg = 0;
	int c = 0;
	while(c < COUNT) {
		sem.sem_num = 0;
		sem.sem_op = -1;
		Semop(sem_id, &sem, 1);
		c++;
		//cerr << "do parent " << c << endl;
		sem.sem_num = 1; 
		sem.sem_op = 1;
		Semop(sem_id, &sem, 1);
	}
}



int main(int argc, char *argv[]) {


	int *private_buf = (int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (private_buf == MAP_FAILED) {
		perror("Can't mmap. So many tears\n");
		return 0;
	}

	bzero(private_buf, 4096);

	int *shared_buf = (int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
	if (shared_buf == MAP_FAILED) {
		perror("Can't mmap. So many tears\n");
		return 0;
	}

	char stack_buf[4096];

	memset(private_buf, 0, 4096); // NOT REALLY PRIVATE
	memset(shared_buf, 0, 4096);
	memset(stack_buf, 0, 4096);

	
	key_t key = ftok("/etc/passwd", 'A');
	sem_id = semget(key, 2, 0666 | IPC_CREAT | IPC_EXCL);
	if (sem_id < 0) {
		perror("semaphore problems: ");
		return EXIT_FAILURE;
	}

	union semun init;
	init.val = 0;
	semctl(sem_id, 0, SETVAL, init);

	init.val = 1;
	semctl(sem_id, 1, SETVAL, init);




	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);
	//	pthread_t pthread_attr_t attr;

	//pthread_attr_init(&attr);
	pthread_create(&child_thr, NULL, child_work_function, private_buf);

	for(;;) {
		parent_work_function();
	}

	

	return 0;
}
