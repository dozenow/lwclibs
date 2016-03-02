#ifndef SNAPPER_H
#define SNAPPER_H

#include <sys/syscall.h>
#include <unistd.h>

#define SNAP_DIAGNOSTIC


typedef int snap_id_t;

#define SNAP_NOJUMP 0x0 
#define SNAP_TAKE_SNAP 0x1

#ifdef SNAP_DIAGNOSTIC

int snap_jump(snap_id_t sid);
int snap_take();
int snap(snap_id_t sid, int flags);

#else

#define snap(sid, flags) syscall(547, sid, flags)
#define snap_jump(sid) syscall(547, sid, 0)
#define snap_take() syscall(547, SNAP_NOJUMP, SNAP_TAKE_SNAP);

#endif


#endif
