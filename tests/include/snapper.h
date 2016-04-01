#ifndef SNAPPER_H
#define SNAPPER_H

#include <sys/syscall.h>
#include <stdlib.h>
#include <unistd.h>

//#define SNAP_DIAGNOSTIC

#ifdef __cplusplus
namespace snapper {
extern "C" {
#endif

/* special targets */
#define SNAP_TARGET_NOJUMP (-1)

/* snap flags  */
#define SNAP_SHARE_VM     0x0001
#define SNAP_SHARE_FD     0x0002
#define SNAP_SHARE_CRED   0x0004
#define SNAP_SHARE_ALL    0x0007

#define SNAP_UPDATEABLE   0x0008
#define SNAP_UPDATE       0x0010
#define SNAP_NOTHING      0x0020

#define SNAP_FAILED (-1)
#define SNAP_JUMPED (-2)

// if you want to look purty
#define SNAP_ALL (0)

#include <string.h>
#include <errno.h>


int debug_snap(int dest, int *src, int flags);

#ifdef SNAP_DIAGNOSTIC

#define snap debug_snap

inline int Snap(int dest, int *src, int flags) {
	int rv = debug_snap(dest, src, flags);
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

#else

#define snap(dest, src, flags) syscall(547, dest, src, flags)

inline int Snap(int dest, int *src, int flags) {
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


#endif

#define snap_jump(dest) snap(dest, NULL, SNAP_NOTHING)
#define snap_take() snap(SNAP_TARGET_NOJUMP, NULL, SNAP_ALL);





#ifdef __cplusplus
}
};
#endif




#endif
