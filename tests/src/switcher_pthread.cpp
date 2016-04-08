#include <unistd.h>
#include <stdio.h>
#include <sys/procdesc.h>
#include <sys/mman.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include <iostream>

#include <pthread.h>

#include "netwrap.hpp"
#include "snapper.h"

using namespace snapper;

pthread_t child_thr;
pthread_cond_t cond;
pthread_mutex_t mutex;


#define IDX_ORIG 0
#define IDX_SUPER 1
#define IDX_CHLD 2

using std::cerr;
using std::endl;



struct timespec diff(struct timespec *start, struct timespec *end)
{
	struct timespec temp;
	if ((end->tv_nsec-start->tv_nsec)<0) {
		temp.tv_sec = end->tv_sec-start->tv_sec-1;
		temp.tv_nsec = 1000000000+end->tv_nsec-start->tv_nsec;
	} else {
		temp.tv_sec = end->tv_sec-start->tv_sec;
		temp.tv_nsec = end->tv_nsec-start->tv_nsec;
	}
	return temp;
}


void* child_work_function(void *arg) {
	
	struct timespec start, end;

	int *buf = (int*)arg; //not really private, just didn't want to rename things

	clock_gettime(CLOCK_REALTIME, &start);

	while(buf[1] < 10000) {
		pthread_mutex_lock(&mutex);
		buf[1] = (buf[1]+1);
		//cerr << "do child\n";
		pthread_cond_signal(&cond);
		pthread_cond_wait(&cond, &mutex);
		pthread_mutex_unlock(&mutex);
	}

	clock_gettime(CLOCK_REALTIME, &end);
	struct timespec res = diff(&start, &end);
	cerr << "Total time for 10 k switches is  " << res.tv_sec << " seconds " << res.tv_nsec << " nanoseconds" << endl;
	exit(0);
	return NULL;
}


void parent_work_function() {
	for(;;) {
		pthread_mutex_lock(&mutex);
		//cerr << "do parent\n";
		pthread_cond_signal(&cond);
		pthread_cond_wait(&cond, &mutex);
		pthread_mutex_unlock(&mutex);
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
