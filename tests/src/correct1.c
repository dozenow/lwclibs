#include <unistd.h>
#include <stdio.h>
#include <sys/procdesc.h>
#include <sys/mman.h>
#include <string.h>
#include <strings.h>
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
	snap_id_t late_src = -1;
	snap_id_t src = -1;
	snap_id_t new_snap;
	new_snap = snap(SNAP_TARGET_NOJUMP, &src, SNAP_ALL);

	if (new_snap > 0) { // created a snap
		sbuf[0] = new_snap;
	} else if (new_snap == 0) {
		if (src != sbuf[1]) {
			fprintf(stderr, "Jumped in with src=%d but expected %d\n", src, sbuf[1]);
			return 5;
		}
	} else if (new_snap < 0) {
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

	if (sbuf[1]++ == 10) {
		/* if we got here, we had nine of these without anything going badly. call it a win */
		snap(src, NULL, SNAP_NOTHING);
		fprintf(stderr, "Should not get here\n");
		return 6;
	}

	new_snap = snap(sbuf[0], &late_src, SNAP_ALL);
	if (new_snap != 0 && sbuf[0] != 1 && sbuf[1] != 10) {
		fprintf(stderr, "unexpected finish, new snap is %d and late src is %d and sbuf[0] is %d, sbuf[1] is %d\n", new_snap, late_src, sbuf[0], sbuf[1]);
		return 4;
	}



	return 0;
}
