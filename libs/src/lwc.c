#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "lwc.h"


int Lwccreate(struct lwc_resource_specifier *resources, size_t numr, int *src, void *src_args, size_t *num_args, int flags) {
	int rv = lwccreate(resources, numr, src, src_args, num_args, flags);
	if (rv == LWC_FAILED) {
		fprintf(stderr, "lwccreate got error %d: %s\n", errno, strerror(errno));
#ifdef __cplusplus
		throw strerror(errno);
#else
		abort();
#endif
	}
	return rv;
}

int Lwcsuspendswitch(int to, void *to_args, size_t num_toargs, int *src, void *src_args, size_t *num_args) {
	int rv = lwcsuspendswitch(to, to_args, num_toargs, src, src_args, num_args);
	if (rv == LWC_FAILED) {
		fprintf(stderr, "lwcsuspend got error %d: %s\n", errno, strerror(errno));
#ifdef __cplusplus
		throw strerror(errno);
#else
		abort();
#endif
	}
	return rv;
}

int Lwcdiscardswitch(int to, void *to_arg, size_t num_toargs) {
	int rv = lwcdiscardswitch(to, to_arg, num_toargs);
	if (rv == LWC_FAILED) {
		fprintf(stderr, "lwcsuspend got error %d: %s\n", errno, strerror(errno));
#ifdef __cplusplus
		throw strerror(errno);
#else
		abort();
#endif
	}
	return rv;
}

int Lwoverlay(int to, struct lwc_resource_specifier *specs, size_t numr) {
	int rv = lwcoverlay(to, specs, numr);
	if (rv == LWC_FAILED) {
		fprintf(stderr, "lwcoverlay got error %d: %s\n", errno, strerror(errno));
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

