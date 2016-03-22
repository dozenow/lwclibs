#include <stdio.h>

#include "snapper.h"

#ifdef SNAP_DIAGNOSTIC

snap_id_t snap(snap_id_t dest, snap_id_t *src, int flags) {
	printf("called snap(%d,%s,%d)=\n", dest, src ? "NON_NULL" : "NULL", flags);
	int ret = syscall(547, dest, src, flags);
	printf("\tnewsnap =>%d, src snap => %d\n", ret, src ? *src : -1);
	return ret;
}

#endif
