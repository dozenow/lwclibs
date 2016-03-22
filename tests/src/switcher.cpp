#include <unistd.h>
#include <stdio.h>
#include <sys/procdesc.h>
#include <sys/mman.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include <iostream>

#include "netwrap.hpp"
#include "snapper.h"

using namespace std;
using namespace snapper;

#define IDX_ORIG 0
#define IDX_SUPER 1
#define IDX_CHLD 2


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


void child_work_function(char * stack_buf, int *shared_buf, int *private_buf) {
	
	struct timespec start, end;

	clock_gettime(CLOCK_REALTIME, &start);

	while(private_buf[1] < 10000) {
		snap_id_t src;
		stack_buf[1] = private_buf[1] = (private_buf[1]+1);
		snap_id_t ns = Snap(shared_buf[IDX_ORIG], &src, SNAP_SHARED | SNAP_UPDATE | SNAP_VM);
		//cerr << "In child with ns=" << ns << " and src=" << src << " with stack_buf and private buf = " << (int)stack_buf[1] << ' ' << (int)private_buf[1] << endl;
	}

	clock_gettime(CLOCK_REALTIME, &end);
	struct timespec res = diff(&start, &end);
	cerr << "Total time for 10 k switches is  " << res.tv_sec << " seconds " << res.tv_nsec << " nanoseconds" << endl;
	exit(0);
}


void parent_work_function(char * stack_buf, int *shared_buf, int *private_buf) {
	for(;;) {
		snap_id_t src;
		snap_id_t ns = Snap(shared_buf[IDX_CHLD], &src, SNAP_SHARED | SNAP_UPDATE | SNAP_VM);
		//cerr << "In parent with ns=" << ns << " and src=" << src << " with stack_buf and private buf = " << (int)stack_buf[1] << ' ' << (int) private_buf[1] << endl;
		//sleep(1);
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

	memset(private_buf, 0, 4096);
	memset(shared_buf, 0, 4096);
	memset(stack_buf, 0, 4096);

	snap_id_t src,cur;
	cur = Snap(SNAP_TARGET_NOJUMP, &src, SNAP_VM);
	if (cur > 0) {
		shared_buf[IDX_ORIG] = cur;

		// create child context

		cur = Snap(SNAP_TARGET_NOJUMP, &src, SNAP_SHARED | SNAP_VM); //can't actually get back here!
		if (cur > 0) {
			shared_buf[IDX_CHLD] = cur;
		} 

		child_work_function(stack_buf, shared_buf, private_buf);
		cerr << "Child improperly exited?" << endl;
		exit(1);
			
	} else if (cur == 0) {
		cur = Snap(SNAP_TARGET_NOJUMP, &src, SNAP_SHARED | SNAP_VM); //to set up parent fast jumper
		if (cur > 0) {
			shared_buf[IDX_ORIG] = cur;
		}

		parent_work_function(stack_buf, shared_buf, private_buf);
		cerr << "Parent improperly exited?" << endl;
		exit(2);
	}

	return 0;
}
