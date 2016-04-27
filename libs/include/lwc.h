#ifndef SNAPPER_H
#define SNAPPER_H

#include <sys/syscall.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

/* special targets */
#define LWC_TARGET_NOJUMP (-1)

#define LWC_FAILED (-1)
#define LWC_SWITCHED (-2)

#include <string.h>
#include <errno.h>



/* resource types */
#define LWC_RESOURCE_MEMORY	0x001
#define LWC_RESOURCE_FILES		0x002
#define LWC_RESOURCE_CREDENT	0x004

/* resource options */
#define LWC_RESOURCE_COPY		0x000
#define LWC_RESOURCE_SHARE		0x008
#define LWC_RESOURCE_UNMAP		0x010


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
			int x;
			/* not sure how to make this coarse yet */
		} credentials;
	} sub;
};

#define lwccreate(resources, numr, src, src_arg) syscall(546, resources, numr, src, src_arg)
#define lwcswitch(to, to_arg, src, src_arg, susp) syscall(547, to, to_arg, src, src_arg, susp)
#define lwcsuspendswitch(to, to_arg, src, src_arg) lwcswitch(to, to_arg, src, src_arg, 1)
#define lwcdiscardswitch(to, to_arg) lwcswitch(to, to_arg, NULL, NULL, 0)
#define lwcoverlay(to, resources, numr) syscall(548, to, resources, numr)

#ifdef __cplusplus
}
#endif




#endif
