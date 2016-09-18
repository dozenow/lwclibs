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

#define COUNT 10000

int snapshot() {
	struct lwc_resource_specifier specs[1];

	/* share the file table */
	specs[0].flags = LWC_RESOURCE_FILES | LWC_RESOURCE_SHARE;
	specs[0].sub.descriptors.from = specs[0].sub.descriptors.to = -1;

	register_t src_args[1];
	int num_args = sizeof(src_args)/(sizeof(src_args[0]));;
	int ret = Lwccreate(specs, 1, NULL, src_args, &num_args, 0);
	if (ret >= 0) {
		return ret;
	} else {
		return -1;
	}
}

void rollback(int snap) {
	register_t to_args[] = { lwcgetlwc() };
	int num_toargs = sizeof(to_args)/(sizeof(to_args[0]));
	Lwcdiscardswitch(snap, NULL, 0);
	cerr << "rollback returned!";
	abort();
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

	shared_buf[0] = 0;

	struct timespec start, end;

	clock_gettime(CLOCK_REALTIME, &start);

	int snap = snapshot();
	shared_buf[0] += 1;
	if (shared_buf[0] < COUNT)
		rollback(snap);

	clock_gettime(CLOCK_REALTIME, &end);
	struct timespec res = diff(&start, &end);
	cerr << "Total time for " << COUNT << " rollbacks is  " << res.tv_sec << " seconds " << res.tv_nsec << " nanoseconds" << endl;


	return 0;
}
