#include <unistd.h>
#include <stdio.h>
#include <sys/procdesc.h>
#include <sys/mman.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <time.h>

#include "lwc.h"

void waitwait(void) {
	printf("Would be exiting now\n");
	sleep(30000);
}

int main() {

	printf("Hit enter for go time\n");
	getchar();

	//atexit(waitwait);
	char *mbuf = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (mbuf == MAP_FAILED) {
		perror("Can't mmap. So many tears\n");
		return 0;
	}

	bzero(mbuf, 4096);

	int *sbuf = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
	if (sbuf == MAP_FAILED) {
		perror("Can't mmap. So many tears\n");
		return 0;
	}

	char stackbuf[4096];

	memset(mbuf, 1, 4096);
	memset(stackbuf, 1, 4096);

	sbuf[0] = lwcgetlwc();
	
	int late_src = -1;
	int src = -43;
	int new_snap;
	
	struct lwc_resource_specifier specs[1];

	/* share the file table */
	specs[0].flags = LWC_RESOURCE_FILES | LWC_RESOURCE_SHARE;
	specs[0].sub.descriptors.from = specs[0].sub.descriptors.to = -1;

#define SNAPS 10

	for(int i = 1; i <= SNAPS; ++i) {
		new_snap = lwccreate(specs, 1, &src, 0, 0, 0);

		if (new_snap >= 0) { // created a snap
			fprintf(stderr, "new snap is %d\n", new_snap);
			sbuf[i] = new_snap;
		} else if (new_snap == LWC_SWITCHED) {
			if (src != sbuf[i-1]) {
				fprintf(stderr, "Jumped in with src=%d but expected %d\n", src, sbuf[i-1]);
				return 5;
			} else {
				fprintf(stderr, "Jumped into %d from %d\n", lwcgetlwc(), src);
			}

			/* assert that mbuf and stackbuf is all ones, even though I overwrite them. */
			for(size_t i = 0; i < 4096; ++i) {
				if (!(mbuf[i] == stackbuf[i] == 1)) {
					fprintf(stderr, "mbuf or stackbuf changed not wiped away in the snap\n");
					return 1;
				}
			}
			memset(mbuf, 42, 4096);
			memset(stackbuf, 42, 4096);

			Lwcswitch(sbuf[(i+1) % SNAPS], NULL, 0, NULL, NULL, 0);

			fprintf(stderr, "should not have reached this point\n");
			return 8;

		} else if (new_snap == LWC_FAILED) {
			fprintf(stderr, "error doing snap create / jump to snap create\n");
			return 3;
		} else {
			fprintf(stderr, "unexpected return value from create: %d\n", new_snap);
			return 4;
		}
	} 

	if (Lwcswitch(sbuf[1], NULL, 0, NULL, NULL, 0) == LWC_SWITCHED) {
		fprintf(stderr, "All done\n");
	}

	return 0;

}
