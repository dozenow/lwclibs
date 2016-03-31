#include <unistd.h>
#include <stdio.h>
#include <sys/procdesc.h>
#include <sys/mman.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include <sys/capsicum.h>

#include <iostream>

//#define SNAP_DIAGNOSTIC
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
		int src;
		stack_buf[1] = private_buf[1] = (private_buf[1]+1);
		int ns = Snap(shared_buf[IDX_ORIG], NULL, SNAP_SHARED | SNAP_UPDATE | SNAP_NO_FD | SNAP_NO_CRED);
#if 0
		cerr << "In child with ns=" << ns << " and src=" << src << " with stack_buf and private buf = " << (int)stack_buf[1] << ' ' << (int)private_buf[1] << endl;
		cerr << "Child UID is " << getuid() << " and capped: " << (bool) cap_sandboxed() << endl;

		int fd = open("/tmp/foobar", O_RDWR | O_CREAT);
		if (fd < 0) {
			perror("file open in child: ");
		} else {
			close(fd);
		}
#endif


	}

	clock_gettime(CLOCK_REALTIME, &end);
	struct timespec res = diff(&start, &end);
	cerr << "Total time for 10 k switches is  " << res.tv_sec << " seconds " << res.tv_nsec << " nanoseconds" << endl;
	exit(0);
}


void parent_work_function(char * stack_buf, int *shared_buf, int *private_buf) {
	int i = 0;
	for(;;) {
		int src;
		int ns = Snap(shared_buf[IDX_CHLD], NULL, SNAP_SHARED | SNAP_UPDATE | SNAP_NO_FD | SNAP_NO_CRED);
#if 0
		cerr << "In parent with ns=" << ns << " and src=" << src << " with stack_buf and private buf = " << (int)stack_buf[1] << ' ' << (int) private_buf[1] << endl;
		cerr << "Parent UID is " << getuid() << " and capped: " << (bool) cap_sandboxed() << endl;
		//sleep(1);

		int fd = open("/tmp/foobar", O_RDWR | O_CREAT);
		if (fd < 0) {
			perror("file open in parent: ");
		} else {
			close(fd);
			unlink("/tmp/foobar");
		}
#endif

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

	shared_buf[IDX_CHLD] = -1;

	int src,cur;
	cur = Snap(SNAP_TARGET_NOJUMP, NULL, SNAP_ALL | SNAP_NO_FD);
	if (cur >= 0) {
		shared_buf[IDX_ORIG] = cur;

		setuid(1001);
		if (cap_enter() != 0) {
			perror("Cap enter: ");
			return 72;
		}

		// create child context

		cur = Snap(SNAP_TARGET_NOJUMP, NULL, SNAP_SHARED | SNAP_NO_FD ); //can't actually get back here!
		if (cur >= 0) {
			shared_buf[IDX_CHLD] = cur;
		} 

		child_work_function(stack_buf, shared_buf, private_buf);
		cerr << "Child improperly exited?" << endl;
		exit(1);
			
	} else if (cur == SNAP_JUMPED) {
		cur = Snap(SNAP_TARGET_NOJUMP, NULL, SNAP_SHARED | SNAP_NO_FD ); //to set up parent fast jumper
		if (cur >= 0) {
			shared_buf[IDX_ORIG] = cur;
		}

		parent_work_function(stack_buf, shared_buf, private_buf);
		cerr << "Parent improperly exited?" << endl;
		exit(2);
	}

	return 0;
}
