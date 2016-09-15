#include <unistd.h>
#include <stdio.h>
#include <sys/procdesc.h>
#include <sys/mman.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <errno.h>

#include <ucontext.h>

#include <iostream>

#include "lwc.h"
#include "bench.h"

using namespace std;

static ucontext_t ctx[3];


#define COUNT 1000000

void child_work_function() {
	
	struct timespec start, end;

	int c = 0;
	clock_gettime(CLOCK_REALTIME, &start);

	while(c < COUNT) {
		++c;
		//cerr << "in child function c= " << c << endl;
		if (swapcontext(&ctx[1], &ctx[2]) == -1) {
			cerr << "error swapping context in child: " << strerror(errno) << endl;
			exit(0);
		}
	}

	clock_gettime(CLOCK_REALTIME, &end);
	struct timespec res = diff(&start, &end);
	cerr << "Total time for " << COUNT << " switches is  " << res.tv_sec << " seconds " << res.tv_nsec << " nanoseconds" << endl;
	/* so dtrace can do ustacks!! */
	//for(;;) {sleep(1); }
	exit(0);
}


void parent_work_function() {
	int c = 0;

	while(c < COUNT) {
		++c;
		//cerr << "in parent function c= " << c << endl;
		if (swapcontext(&ctx[2], &ctx[1]) == -1) {
			cerr << "error swapping context in parent: " << strerror(errno) << endl;
			exit(0);
		}
	}
}



int main(int argc, char *argv[]) {

	char *stack1 = (char*)mmap(NULL, 1 << 23, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	char *stack2 = (char*)mmap(NULL, 1 << 23, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

	if (stack1 == MAP_FAILED || stack2 == MAP_FAILED) {
		perror("can't mmap stacks\n");
		return 0;
	}

	getcontext(&ctx[1]);
	ctx[1].uc_stack.ss_sp = stack1;
	ctx[1].uc_stack.ss_size = 1 << 23;
	ctx[1].uc_link = &ctx[0];
	makecontext(&ctx[1], child_work_function, 0);
		
	getcontext(&ctx[2]);
	ctx[2].uc_stack.ss_sp = stack2;
	ctx[2].uc_stack.ss_size = 1 << 23;
	ctx[2].uc_link = &ctx[1];
	makecontext(&ctx[2], parent_work_function, 0);

	swapcontext(&ctx[0], &ctx[2]);

	return 0;
}
