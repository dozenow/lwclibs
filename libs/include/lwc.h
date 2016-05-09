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

typedef int64_t register_t;
	
//extern int lwcdiscardswitch(int to, void *to_arg, int num_toargs);
extern int lwcsuspendswitch(int to, register_t *to_args, int num_toargs, int *src, register_t *src_args, int *num_args);
extern int lwcdiscardswitch(int to, register_t *to_arg, int num_toargs);
extern int lwccreate(struct lwc_resource_specifier *resources, int numr, int *src, void *src_arg, int *num_args, int flags);
extern int lwcoverlay(int from, struct lwc_resource_specifier *resources, int numr);
extern int lwcrestrict(int lwc, struct lwc_resource_specifier *resources, int numr);
extern int lwcsyscall(int lwc, unsigned int mask, int syscall, register_t arg1, register_t arg2, register_t arg3);
extern int lwcclose(int lwc);
extern int lwcgetlwc();

extern int Lwccreate(struct lwc_resource_specifier *resources, int numr, int *src, register_t *src_arg, int *num_args, int flags);
extern int Lwcdiscardswitch(int to, register_t *to_arg, int num_toargs);
extern int Lwcsuspendswitch(int to, register_t *to_args, int num_toargs, int *src, register_t *src_args, int *num_args);



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
#define LWC_TARGET_SELF	(-1)

/* user visible flags, note, should be disjoint from RETCREATE, etc */
#define LWC_SUSPEND_ONLY	0x0008
#define LWC_SYSTRAP			0x0040

#define LWC_FAILED (-1)
#define LWC_SWITCHED (-2)


/* resource types */
#define LWC_RESOURCE_MEMORY	0x001
#define LWC_RESOURCE_FILES		0x002
#define LWC_RESOURCE_CREDENT	0x004

/* resource options */
#define LWC_RESOURCE_COPY				0x000
#define LWC_RESOURCE_SHARE				0x008
#define LWC_RESOURCE_UNMAP				0x010
#define LWC_RESOURCE_MAY_OVERLAY		0x020

/* aliases for the sane */
#define LWCR_COPY				LWC_RESOURCE_COPY
#define LWCR_SHARE			LWC_RESOURCE_SHARE
#define LWCR_UNMAP			LWC_RESOURCE_UNMAP
#define LWCR_MAY_OVERLAY	LWC_RESOURCE_MAY_OVERLAY

#define LWCR_MEMORY	LWC_RESOURCE_MEMORY	
#define LWCR_FILES	LWC_RESOURCE_FILES		 
#define LWCR_CREDENT	LWC_RESOURCE_CREDENT	


#ifdef __cplusplus
}
#endif


#endif
