#include <stdio.h>

#include "snapper.h"


int debug_snap(int dest, int *src, int flags) {
	printf("called snap(%d,%s,%d)=\n", dest, src ? "NON_NULL" : "NULL", flags);
	int ret = syscall(547, dest, src, flags);
	printf("\tnewsnap =>%d, src snap => %d\n", ret, src ? *src : -1);
	return ret;
}

