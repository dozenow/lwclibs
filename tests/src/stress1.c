#include <unistd.h>
#include <stdio.h>
#include <sys/procdesc.h>
#include <sys/mman.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <time.h>

#include "lwc.h"

#define N 12
#define WIND 1
int main() {

	register_t *mbuf = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (mbuf == MAP_FAILED) {
		perror("Can't mmap. So many tears\n");
		return 0;
	}

	bzero(mbuf, 4096);

	register_t *sbuf = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
	if (sbuf == MAP_FAILED) {
		perror("Can't mmap. So many tears\n");
		return 0;
	}

	char stackbuf[4096];

	memset(sbuf, 0, 4096);
	memset(mbuf, 0, 4096);
	memset(stackbuf, 0, 4096);

	int switches_remaining = 10*N;
	struct lwc_resource_specifier specs[1];

	/* share the file table */
	specs[0].flags = LWC_RESOURCE_FILES | LWC_RESOURCE_SHARE;
	specs[0].sub.descriptors.from = specs[0].sub.descriptors.to = -1;

	time_t start = time(NULL);
	time_t last = start;
	size_t switches = 0;


	int parent_snap = lwcgetlwc();

	if (parent_snap < 0) {
		printf("do not have parent snap here\n");
		sleep(1);
		abort();
	} else {
		printf("parent snap is %d\n", parent_snap);
	}

	for(int i = 0; i < N; ++i) {
		int new_snap = lwccreate(specs, 1, NULL, 0, 0, 0);
		if (new_snap >= 0) {
			printf("new snap created at %d\n", new_snap);
			sbuf[i] = new_snap;
		} else if (new_snap == LWC_FAILED) {
			perror("Can't snap");
			abort();
		} else {
			//printf("Child %d doing something\n", lwcgetlwc());
			lwcdiscardswitch(parent_snap, NULL, 0);
			perror("child could not discard switch");
			abort();
		}
	}

	int cur = 0;
	for(;;) {
		//for(;sbuf[0] > 0 && switches_remaining > 0;--switches_remaining) {
		switches++;

		time_t curtm = time(NULL);
		if (curtm - last >= WIND) {
			printf("Running %ld seconds with %f switches per %d second window\n",
			       curtm - start, (1.0*switches) / (1.0 * WIND), WIND);
			switches = 0;
			last = curtm;
		}

/*
		printf("parent doing switch to sbuf[%d]=%ld, remaining = %d\n",
		       cur, sbuf[cur], switches_remaining);
*/
		lwcsuspendswitch(sbuf[cur], NULL, 0, NULL, NULL, 0);
		cur = (cur + 1) % N;
	}

	return 0;
}
