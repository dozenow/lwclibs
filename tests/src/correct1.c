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

	atexit(waitwait);
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

	sbuf[0] = 42;
	sbuf[1] = lwcgetlwc();
	int late_src = -1;
	int src = -1;
	int new_snap;
	
	struct lwc_resource_specifier specs[1];

	/* share the file table */
	specs[0].flags = LWC_RESOURCE_FILES | LWC_RESOURCE_SHARE;
	specs[0].sub.descriptors.from = specs[0].sub.descriptors.to = -1;

	new_snap = lwccreate(specs, 1, &src, 0, 0, 0);

	if (new_snap >= 0) { // created a snap
		fprintf(stderr, "src on new snap is %d\n", src);
		sbuf[0] = new_snap;
	} else if (new_snap == LWC_SWITCHED) {
		if (src != sbuf[1]) {
			fprintf(stderr, "Jumped in with src=%d but expected %d\n", src, sbuf[1]);
			return 5;
		}
	} else if (new_snap == LWC_FAILED) {
		fprintf(stderr, "error doing snap create / jump to snap create\n");
		return 3;
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

	if (sbuf[1]++ == 14) {
		/* if we got here, we had nine of these without anything going badly. call it a win */
		lwcdiscardswitch(src, NULL, 0);
		perror("Should not get here\n");
		return 6;
	}
	sbuf[1] = lwcgetlwc();
	new_snap = lwcsuspendswitch(sbuf[0], NULL, 0, &late_src, NULL, NULL);
	if (new_snap != LWC_SWITCHED && sbuf[0] != 1 && sbuf[1] != 14) {
		fprintf(stderr, "unexpected finish, new snap is %d and late src is %d and sbuf[0] is %d, sbuf[1] is %d\n", new_snap, late_src, sbuf[0], sbuf[1]);
		return 4;
	}

	return 0;
}
