#include <unistd.h>
#include <stdio.h>
#include <sys/procdesc.h>
#include <sys/mman.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <time.h>

#include "lwc.h"

#define CON 128
#define SWITCHES 2
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

	struct lwc_resource_specifier specs[1];

	/* share the file table */
	specs[0].flags = LWC_RESOURCE_FILES | LWC_RESOURCE_SHARE;
	specs[0].sub.descriptors.from = specs[0].sub.descriptors.to = -1;


	/* so the pattern here is create child lwcs from the parent, switch in and out of them, and destroy them from the parent */

	int parent_snap = lwcgetlwc();

	if (parent_snap < 0) {
		printf("do not have parent snap here\n");
		sleep(1);
		abort();
	} else {
		printf("parent snap is %d\n", parent_snap);
	}

	int snaplist[CON];
	int switchcnt[CON] = {0};
	int snapcnt = 0;
	memset(snaplist, ~0, sizeof(*snaplist) * CON);

	time_t start = time(NULL);
	srand(start);
	time_t last = start;
	size_t creations = 0;

	for(;;) {
		if (snapcnt == CON) {
			int idx = rand() % CON;
			int nxt = snaplist[idx];

			lwcsuspendswitch(nxt, NULL, 0, NULL, NULL, 0);
			if (++switchcnt[idx] == SWITCHES) {
				lwcclose(snaplist[idx]);
				snaplist[idx] = -1;
				snapcnt--;
			}
		} else {
			int new_snap = lwccreate(specs, 1, NULL, 0, 0, 0);
			if (new_snap >= 0) {
				int idx = 0;
				for(;snaplist[idx] > 0; ++idx);
				snaplist[idx] = new_snap;
				switchcnt[idx] = 0;
				snapcnt++;

				creations++;

				time_t cur = time(NULL);
				if (cur - last >= WIND) {
					printf("Running %ld seconds with %f creations per %d second window\n",
					       cur - start, (1.0*creations) / (1.0 * WIND), WIND);
					creations = 0;
					last = cur;
				}


			} else if (new_snap == LWC_SWITCHED) {

				for(;;) { /* child loop */
					lwcsuspendswitch(parent_snap, NULL, 0, NULL, NULL, 0);
				}
			} else if (new_snap == LWC_FAILED) {
				perror("Can't create snap");
				return EXIT_FAILURE;
			}
		}
	}


	return 0;
}
