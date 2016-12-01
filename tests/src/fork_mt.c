#include <unistd.h>
#include <stdio.h>
#include <sys/procdesc.h>
#include <sys/mman.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/socket.h>

#include <pthread.h>

#include "lwc.h"

void * thr_start(void * arg) {
	//int id = (size_t) arg;
	for(;;) {
		usleep(10);
	}
	return arg;
}


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

int main() {

	printf("enter to go\n");
	getchar();

	char *mbuf = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (mbuf == MAP_FAILED) {
		perror("Can't mmap. So many tears\n");
		return 1;
	}

	bzero(mbuf, 4096);

	int *sbuf = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
	if (sbuf == MAP_FAILED) {
		perror("Can't mmap. So many tears\n");
		return 1;
	}



	char stackbuf[4096];

	memset(mbuf, 1, 4096);
	memset(stackbuf, 1, 4096);

	
	int sock[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sock) != 0) {
		perror("Could not socket pair");
		return EXIT_FAILURE;
	}

	int guv = Lwccreate(NULL, 0, NULL, NULL, 0, 0);
	if (guv == LWC_SWITCHED) {
		fprintf(stderr, "in guv, exiting\n");
		return EXIT_SUCCESS;
	}
	Lwccreate(NULL, 0, NULL, NULL, 0, 0);

	pthread_t thrs[10];
	for(int i = 0; i < 2; ++i) {
		int ret = pthread_create(&thrs[i], NULL, thr_start, (void*) (size_t)i);
		if (ret) {
			fprintf(stderr, "pthread_create failed: %s\n", strerror(errno));
			return EXIT_FAILURE;
		}
	}

	usleep(10);

	Lwccreate(NULL, 0, NULL, NULL, 0, 0);
	int last = Lwccreate(NULL, 0, NULL, NULL, 0, 0);
	if (last != LWC_SWITCHED) {
		Lwcswitch(last, NULL, 0, NULL, NULL, 0);		
	}


	pid_t pid = fork();
	if (pid) {
		waitpid(pid, NULL, 0);
		fprintf(stderr, "parent exiting\n");
	} else {
		Lwccreate(NULL, 0, NULL, NULL, 0, 0);
		Lwccreate(NULL, 0, NULL, NULL, 0, 0);
		Lwcswitch(guv, NULL, 0, NULL, NULL, 0);
		fprintf(stderr, "child got here, which is wrong. \n");
	}

	return 0;


}
