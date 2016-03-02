#include <unistd.h>
#include <stdio.h>
#include <sys/procdesc.h>
#include <sys/mman.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdlib.h>
#include <time.h>

#include "snapper.h"

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

	sbuf[1] = 0;
	sbuf[0] = snap_take();
	if (sbuf[0] != 1) {
		printf("return value was %d\n", sbuf[0]);
		return 1;
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

	if (sbuf[1]++ == 9) {
		/* if we got here, we had nine of these without anything going badly. call it a win */
		return 0;
	}

	snap_jump(sbuf[0]);

	fprintf(stderr, "Should never reach here. Snap did not jump!\n");
	return 2;
}
