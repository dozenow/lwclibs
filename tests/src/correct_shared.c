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

	fprintf(stderr, "Program doesn't test current semantics. Not a valid test\n");
	return 0;

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


	new_snap = snap(SNAP_TARGET_NOJUMP, &src, SNAP_VM | SNAP_SHARED);

	// This just verifies that a shared snap is indeed shared and that
	// updates work.

	if (new_snap > 0) { // created a snap
		sbuf[0] = new_snap;
	} else if (new_snap == 0) {
		if (src != sbuf[1]) {
			fprintf(stderr, "Jumped in with src=%d but expected %d\n", src, sbuf[1]);
			return 5;
		}

		if (mbuf[1] != 42) {
			fprintf(stderr, "jumped in, but mbuf[1] == %d != 42\n", mbuf[1]);
			return 12;
		}

		// okay, now do an update

		
		new_snap = snap(SNAP_TARGET_NOJUMP, &src, SNAP_VM | SNAP_SHARED | SNAP_UPDATE);

		if (new_snap < 0) {
			perror("Error doing snap update\n");
			return 6;
		} else if (new_snap > 0) {
			if (new_snap != sbuf[0]) {
				fprintf(stderr, "got an update, but now have a different new snap?\n");
				return 24;
			}
		} else { //jumped in

			if (src != sbuf[1]) {
				fprintf(stderr, "in update jumped in with src=%d, but expected %d\n", src, sbuf[1]);
				return 8;
			}

			if (sbuf[1] > 2 && mbuf[1] != 84) {
				fprintf(stderr, "jumped in after the first time, but mbuf[1] == %d != 84\n", mbuf[2]);
				return 7;
			} else {
				return 0; //okay fine
			}

			mbuf[1] = 84;

			// okay, now fork it off
			new_snap = snap(SNAP_TARGET_NOJUMP, 0, SNAP_VM);
			if (new_snap < 0) {
				fprintf(stderr, "Could not snap here\n");
				return 9;
			}
			mbuf[1] = 85; //this should not be visible when we go back to the update
			sbuf[2] = 7;

			if (snap(SNAP_TARGET_NOJUMP, &sbuf[1], 0) < 0) {
				fprintf(stderr, "could not getsid\n");
				return 10;
			}

			snap(sbuf[0], NULL, 0);

			fprintf(stderr, "Got here, this should not happen\n");
			return 11;
			
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

	fprintf(stderr, "Setting mbuf\n");
	memset(mbuf, 42, 4096);
	memset(stackbuf, 42, 4096);

	sbuf[1] = 2;
	new_snap = snap(sbuf[0], &late_src, 0);

	if (new_snap < 0) {
		perror("snap error: ");
	}
	fprintf(stderr, "Should not be here\n");

	return 20;
}
