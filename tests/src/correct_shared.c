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
	int new_snap;


	mbuf[0] = 0;

	new_snap = Snap(SNAP_TARGET_NOJUMP, &src, SNAP_SHARE_ALL);
	if (new_snap >= 0) { // created a snap
		mbuf[0] = 1;
		sbuf[0] = new_snap;
		int x = Snap(sbuf[0], NULL, SNAP_NOTHING);
		printf("Should not see this. new snap is %d, x is %d\n", new_snap, x);
	} else if (new_snap == SNAP_JUMPED) {

		if (mbuf[10] != mbuf[0]) {
			fprintf(stderr, "mbuf[10](%d) != mbuf[0](%d) (mbuf 0x%lx)\n", mbuf[10], mbuf[0], (unsigned long) mbuf);
			if (sbuf[1] != 100) {
				sbuf[1] = 100;
				Snap(sbuf[0], NULL, SNAP_NOTHING);
			}
			return EXIT_FAILURE;
		}

		if (stackbuf[10] != 1) {
			fprintf(stderr, "stack changed? sbuf[10] = %d\n", stackbuf[10]);
			return EXIT_FAILURE;
		}

		mbuf[0]++;
		memset(mbuf+4, mbuf[0], 4092);
		memset(stackbuf, mbuf[0], 4096);

		if (mbuf[0] < 10) {
			Snap(sbuf[0], NULL, SNAP_NOTHING);
		}
	} else {
		fprintf(stderr, "Snap failed: %s\n", strerror(errno));
	}

	return EXIT_SUCCESS;

}
