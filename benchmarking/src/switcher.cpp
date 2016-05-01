#include <unistd.h>
#include <stdio.h>
#include <sys/procdesc.h>
#include <sys/mman.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/capsicum.h>

#include <iostream>

#include "lwc.h"
#include "bench.h"

using namespace std;

#define IDX_ORIG 0
#define IDX_CHLD 1

#define TEST_CAP 0

#define COUNT 1000000

void child_work_function(char * stack_buf, int *shared_buf, int *private_buf) {
	
	struct timespec start, end;

	clock_gettime(CLOCK_REALTIME, &start);

	while(private_buf[1] < COUNT) {
		int src = -1;
		stack_buf[1] = private_buf[1] = (private_buf[1]+1);
		int ns = Lwcsuspendswitch(shared_buf[IDX_ORIG], NULL, 0, NULL, NULL, NULL);
#if(TEST_CAP)
		if (1) {
			cerr << "In child with ns=" << ns << " and src=" << src << " with stack_buf and private buf = " << (int)stack_buf[1] << ' ' << (int)private_buf[1] << endl;
			cerr << "Child UID is " << getuid() << " and capped: " << (bool) cap_sandboxed() << endl;
			sleep(1);
			int fd = open("/tmp/foobar", O_RDWR | O_CREAT);
			if (fd < 0) {
				perror("file open in child: ");
			} else {
				close(fd);
				unlink("/tmp/foobar");
			}
		}
#endif

	}

	clock_gettime(CLOCK_REALTIME, &end);
	struct timespec res = diff(&start, &end);
	cerr << "Total time for " << COUNT << " switches is  " << res.tv_sec << " seconds " << res.tv_nsec << " nanoseconds" << endl;
	exit(0);
}


void parent_work_function(char * stack_buf, int *shared_buf, int *private_buf) {
	for(;;) {
		int src = -1;
		int ns = Lwcsuspendswitch(shared_buf[IDX_CHLD], NULL, 0, NULL, NULL, NULL);
#if(TEST_CAP)
		if (1) {
			cerr << "In parent with ns=" << ns << " and src=" << src << " with stack_buf and private buf = " << (int)stack_buf[1] << ' ' << (int) private_buf[1] << endl;
			cerr << "Parent UID is " << getuid() << " and capped: " << (bool) cap_sandboxed() << endl;
			sleep(1);

			int fd = open("/tmp/foobar", O_RDWR | O_CREAT);
			if (fd < 0) {
				perror("file open in parent: ");
			} else {
				close(fd);
				unlink("/tmp/foobar");
			}
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

	struct lwc_resource_specifier specs[1];

	/* share the file table */
	specs[0].flags = LWC_RESOURCE_FILES | LWC_RESOURCE_SHARE;
	specs[0].sub.descriptors.from = specs[0].sub.descriptors.to = -1;

	int cur;
	cur = Lwccreate(specs, 1, NULL, NULL, NULL, LWC_SUSPEND_ONLY);
	if (cur >= 0) {
		shared_buf[IDX_ORIG] = cur;

		Lwcdiscardswitch(cur, NULL, 0);

			
	} else if (cur == LWC_SWITCHED) {

		if (shared_buf[IDX_CHLD] == -1) {
			cur = Lwccreate(specs, 1, NULL, NULL, NULL, LWC_SUSPEND_ONLY);
			if (cur >= 0) {
				shared_buf[IDX_CHLD] = cur;
			} else {
#ifdef TEST_CAP
				cap_enter();
#endif
				child_work_function(stack_buf, shared_buf, private_buf);
			}
		}
		parent_work_function(stack_buf, shared_buf, private_buf);


	}

	return 0;
}
