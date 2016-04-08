#include <stdio.h>

#include "snapper.h"


int Snap(int dest, int *src, int flags) {
	int rv = snap(dest, src, flags);
	if (rv == SNAP_FAILED) {
		fprintf(stderr, "snap got error %d: %s\n", errno, strerror(errno));
#ifdef __cplusplus
		throw strerror(errno);
#else
		abort();
#endif
	}
	return rv;
}


int debug_snap(int dest, int *src, int flags) {
	printf("called snap(%d,%s,%d)=\n", dest, src ? "NON_NULL" : "NULL", flags);
	int ret = syscall(547, dest, src, flags);
	printf("\tnewsnap =>%d, src snap => %d\n", ret, src ? *src : -1);
	return ret;
}

