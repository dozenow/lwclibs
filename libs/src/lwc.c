#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "lwc.h"

int Lwccreate(struct lwc_resource_specifier *resources, int numr, int *src, void *src_args, int *num_args, int flags) {
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

int Lwcsuspendswitch(int to, void *to_args, int num_toargs, int *src, void *src_args, int *num_args) {
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

int Lwcdiscardswitch(int to, void *to_arg, int num_toargs) {
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

int Lwoverlay(int to, struct lwc_resource_specifier *specs, int numr) {
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

