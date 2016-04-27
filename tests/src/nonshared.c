#include <unistd.h>
#include <stdio.h>
#include <sys/procdesc.h>
#include <sys/mman.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <time.h>

#include "lwc.h"

int main() {

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

	sbuf[0] = 0;
	int src = -1;
	int new_snap;
	new_snap = Lwccreate(NULL, 0, NULL, NULL, 0, 0);

	int fd;

	if (new_snap >= 0) { // created a snap
		fprintf(stderr, "newsnap on new snap is %d, src on new snap is %d\n", new_snap, src);
		fd = new_snap;
	} else if (new_snap == LWC_SWITCHED) {
		fprintf(stderr, "src on snap jump is %d\n", src);
		sbuf[0]++;
		fd = src;
	} else if (new_snap == LWC_FAILED) {
		fprintf(stderr, "error doing snap create / jump to snap create\n");
		return 3;
	}


	if (sbuf[0] > 10) {
		close(src);
		return 0;
	}

	Lwcdiscardswitch(src, NULL, 0);


	printf("Should not get here\n");
	return 1;


	return 0;
}
