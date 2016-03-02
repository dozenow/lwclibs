#include <stdio.h>

#include "snapper.h"

#ifdef SNAP_DIAGNOSTIC

int snap(snap_id_t sid, int flags) {
	printf("called snap(%d,%d)=\n", sid, flags);
	int ret = syscall(547, sid, flags);
	printf("\t%d\n", ret);
	return ret;
}

int snap_jump(snap_id_t sid) {
	printf("called snap(%d,%d) (snap_jump)=\n", sid, 0);
	int ret = syscall(547, sid, 0);
	printf("\t%d\n", ret);
	return ret;
}

int snap_take() {
	printf("called snap(%d,%d) (snap_take)=\n ", SNAP_NOJUMP, SNAP_TAKE_SNAP);
	int ret = syscall(547, SNAP_NOJUMP, SNAP_TAKE_SNAP);
	printf("\t%d\n", ret);
	return ret;
}

#endif
