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

	sbuf[0] = 42;
	sbuf[1] = 1;
	int src = -1;
	int new_lwc;

	void *src_arg;
	new_lwc = lwccreate(&specs, 10, &src, &src_arg);

	if (new_lwc >= 0) { // created a lwc
		mbuf[0] = 1;
		sbuf[0] = new_lwc;
		int x = lwcdiscardswitch(sbuf[0], 42);
		printf("Should not see this. new snap is %d, x is %d\n", new_lwc, x);
		return EXIT_FAILURE;
	} else if (new_lwc == LWC_SWITCHED) {
		printf("src is %d and src_arg = %d\n", src, (int) src_arg);
		if (!(mbuf[10] == stackbuf[10] == 1)) {
			fprintf(stderr, "mbuf[10](%d) != stackbuf[10](%d)\n", mbuf[10], stackbuf[10]);
			if (sbuf[1] != 100) {
				sbuf[1] = 100;
				lwcdiscardswitch(sbuf[0], 45);
			}
			return EXIT_FAILURE;
		}

		if (stackbuf[10] != 1) {
			fprintf(stderr, "stack changed? sbuf[10] = %d\n", stackbuf[10]);
			return EXIT_FAILURE;
		}

		sbuf[2]++;

		mbuf[0]++;
		memset(mbuf+4, mbuf[0], 4092);
		memset(stackbuf, mbuf[0], 4096);

		if (sbuf[2] < 10) {
			lwcdiscardswitch(sbuf[0], 46);
		}
	} else {
		fprintf(stderr, "switch failed: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
