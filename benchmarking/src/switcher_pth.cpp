#include <unistd.h>
#include <stdio.h>
#include <sys/procdesc.h>
#include <sys/mman.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>

#include <iostream>

#include <pth/pth.h>

#include "lwc.h"
#include "bench.h"


using std::cerr;
using std::endl;

pth_t orig = NULL;
pth_t chld = NULL;

pth_mutex_t mutex;
pth_cond_t cond;

FILE *fp;

#define COUNT 1000000

void* child_work_function(void *arg) {
	fprintf(fp, "in child wf:\n");
	pth_ctrl(PTH_CTRL_DUMPSTATE, fp);
	
	struct timespec start, end;

	int *buf = (int*)arg; //not really private, just didn't want to rename things

	clock_gettime(CLOCK_REALTIME, &start);

	while(buf[1] < COUNT) {
		pth_mutex_acquire(&mutex, false, NULL);
		buf[1] = (buf[1]+1);
		//cerr << "do child\n";
		pth_cond_notify(&cond, 1);
		pth_cond_await(&cond, &mutex, NULL);
		pth_mutex_release(&mutex);
	}

	clock_gettime(CLOCK_REALTIME, &end);
	struct timespec res = diff(&start, &end);
	cerr << "Total time for " << COUNT << " switches is  " << res.tv_sec << " seconds " << res.tv_nsec << " nanoseconds" << endl;
	exit(0);
	return NULL;
}


void parent_work_function() {
	fprintf(fp, "in parent wf:\n");
	pth_ctrl(PTH_CTRL_DUMPSTATE, fp);

	for(;;) {
		pth_mutex_acquire(&mutex, 0, NULL);
		//cerr << "do parent\n";
		pth_cond_notify(&cond, 1);
		pth_cond_await(&cond, &mutex, NULL);
		pth_mutex_release(&mutex);		
		//sleep(1);

	}
}


int main(int argc, char *argv[]) {

	if (!pth_init())
		return EXIT_FAILURE;


	int *private_buf = (int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (private_buf == MAP_FAILED) {
		perror("Can't mmap. So many tears\n");
		return 0;
	}

	bzero(private_buf, 4096);

	int * shared_buf = (int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
	if (shared_buf == MAP_FAILED) {
		perror("Can't mmap. So many tears\n");
		return 0;
	}

	char stack_buf[4096];

	memset(private_buf, 0, 4096); // NOT REALLY PRIVATE
	memset(shared_buf, 0, 4096); // none of this is private here 
	memset(stack_buf, 0, 4096);

	pth_mutex_init(&mutex);
	pth_cond_init(&cond);


	fp = fopen("/tmp/pth.txt", "w+");
	if (!fp) {
		perror("i suck ");
		return EXIT_FAILURE;
	}
	fprintf(fp, "pre spawn:\n");
	pth_ctrl(PTH_CTRL_DUMPSTATE, fp);
	pth_t thr = pth_spawn(PTH_ATTR_DEFAULT, child_work_function, private_buf);
	fprintf(fp, "post spawn:\n");
	pth_ctrl(PTH_CTRL_DUMPSTATE, fp);
	

	if (!thr) {
		perror("could not spawn child");
		return EXIT_FAILURE;
	}


	orig = pth_self();
	chld = thr;
	fprintf(fp, "orig=0x%lx and chld=0x%lx\n", orig, chld);

	for(;;) {
		parent_work_function();
	}


	return 0;
}
