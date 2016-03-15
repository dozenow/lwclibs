#include <stdio.h>

#include "snapper.h"

#ifdef SNAP_DIAGNOSTIC

snap_id_t snap(snap_id_t dest, snap_id_t *src, int flags) {
	printf("called snap(%d,%s,%d)=\n", dest, src ? "NON_NULL" : "NULL", flags);
	int ret = syscall(547, dest, src, flags);
	printf("\tnewsnap =>%d, src snap => %d\n", ret, src ? *src : -1);
	return ret;
}

snap_id_t snap_jump(snap_id_t dest) {
	snap_id_t src;
	printf("called snap(%d,*,0) (snap_jump)=\n", dest);
	int ret = syscall(547, dest, &src, 0);
	printf("\tnewsnap =>%d, src snap => %d\n", ret, src);
	return ret;
}

snap_id_t snap_take() {
	snap_id_t src;
	printf("called snap(SNAP_TARGET_NOJUMP,*, SNAP_VM | SNAP_FD) (snap_take)=\n ");
	int ret = syscall(547, SNAP_TARGET_NOJUMP, &src, SNAP_VM | SNAP_FD);
	printf("\tnewsnap =>%d, src snap => %d\n", ret, src);
	return ret;
}

#endif
