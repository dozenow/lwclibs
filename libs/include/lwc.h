#ifndef SNAPPER_H
#define SNAPPER_H

#include <sys/syscall.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

struct lwc_resource_specifier;

/*
 * system calls.
 */
	
#define lwccreate(resources, numr, src, src_args, num_args, flags) syscall(546, resources, numr, src, src_args, num_args, flags)
#define lwcswitch(to, to_args, num_to_args, src, src_args, numargs, susp) syscall(547, to, to_args, num_to_args, src, src_args, numargs, susp)
#define lwcsuspendswitch(to, to_args, num_to_args, src, src_args, num_args) lwcswitch(to, to_args, num_to_args, src, src_args, num_args, 1)
#define lwcdiscardswitch(to, to_args, num_to_args) lwcswitch(to, to_args, num_to_args, NULL, NULL, 0, 0)
#define lwcoverlay(to, resources, numr) syscall(548, to, resources, numr)

extern int Lwcdiscardswitch(int to, void *to_arg, size_t num_toargs);
extern int Lwcsuspendswitch(int to, void *to_args, size_t num_toargs, int *src, void *src_args, size_t *num_args);
extern int Lwccreate(struct lwc_resource_specifier *resources, size_t numr, int *src, void *src_arg, size_t *num_args, int flags);


struct lwc_resource_specifier {
	unsigned int flags; /* bitwise one type and one option */
	union {
		struct {
			vm_offset_t start;
			vm_offset_t end;
		} memory;
		struct {
			int from;
			int to;
		} descriptors;
		struct {
			char padding[1];
			/* not coarse */
		} credentials;
	} sub;
};



/* special targets */
#define LWC_TARGET_NOJUMP (-1)

/* user visible flags, note, should be disjoint from RETCREATE, etc */
#define LWC_SUSPEND_ONLY 0x0008

#define LWC_FAILED (-1)
#define LWC_SWITCHED (-2)

/* resource types */
#define LWC_RESOURCE_MEMORY	0x001
#define LWC_RESOURCE_FILES		0x002
#define LWC_RESOURCE_CREDENT	0x004

/* resource options */
#define LWC_RESOURCE_COPY		0x000
#define LWC_RESOURCE_SHARE		0x008
#define LWC_RESOURCE_UNMAP		0x010

#ifdef __cplusplus
}
#endif


#endif
