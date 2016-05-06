#include <unistd.h>
#include <stdio.h>
#include <sys/procdesc.h>
#include <sys/mman.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <iostream>

#include <pthread.h>

#include "lwc.h"
#include "bench.h"

using std::cerr;
using std::endl;

int sem_id;

int count = 0;

#define COUNT 1000000

int my_id = 0;


//#define Semop(a,b,c) if (semop(a,b,c)) { printf("failed at %d: %s\n", __LINE__, strerror(errno)); abort(); }
#define Semop(a,b,c) semop(a,b,c)
/* I use semaphore one for my readiness. */
int child_work_function() {
	
	struct timespec start, end;

	int c = 0;

	struct sembuf sem;
	sem.sem_flg = 0;

	clock_gettime(CLOCK_REALTIME, &start);
	while(c < COUNT) {
		sem.sem_num = 1;
		sem.sem_op = -1;
		Semop(sem_id, &sem, 1);
		c++;
		sem.sem_num = 0;
		sem.sem_op = 1;
		Semop(sem_id, &sem, 1);
	}

	clock_gettime(CLOCK_REALTIME, &end);
	struct timespec res = diff(&start, &end);
	cerr << "Total time for " << COUNT << " switches is  " << res.tv_sec << " seconds " << res.tv_nsec << " nanoseconds" << endl;
	semctl(sem_id, 0, IPC_RMID);
	return 0;
}


/* I use semaphore zero for my readiness. */
int parent_work_function() {
	struct sembuf sem;
	sem.sem_flg = 0;
	int c = 0;
	while(c < COUNT) {
		sem.sem_num = 0;
		sem.sem_op = -1;
		Semop(sem_id, &sem, 1);
		c++;
		sem.sem_num = 1; 
		sem.sem_op = 1;
		Semop(sem_id, &sem, 1);

	}
	return 0;
}



int main(int argc, char *argv[]) {


	int *private_buf = (int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (private_buf == MAP_FAILED) {
		perror("Can't mmap. So many tears\n");
		return 0;
	}

	bzero(private_buf, 4096);

	char * shared_buf = (char*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
	if (shared_buf == MAP_FAILED) {
		perror("Can't mmap. So many tears\n");
		return 0;
	}

	char stack_buf[4096];

	memset(private_buf, 0, 4096); 
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
	
	pid_t p = fork();
	int ret;
	if (p == 0) {
		++my_id;
		ret = child_work_function();
	} else if (p > 0) {
		ret = parent_work_function();
	} else {
		ret = EXIT_FAILURE;
		perror("fork me");
	}
	
	semctl(sem_id, 1, IPC_RMID);
	return ret;
}
