#ifndef SNAPPER_H
#define SNAPPER_H

#include <sys/syscall.h>
#include <unistd.h>

#define SNAP_DIAGNOSTIC


typedef int snap_id_t;

#define SNAP_TARGET_NOJUMP 0x0 
#define SNAP_CREATE 0x1

#ifdef SNAP_DIAGNOSTIC

snap_id_t snap_jump(snap_id_t dest);
snap_id_t snap_take();
snap_id_t snap(snap_id_t dest, snap_id_t *src, int flags);

#else

#define snap(dest, src, flags) syscall(547, dest, src, flags)
#define snap_jump(dest) syscall(547, dest, NULL, 0)
#define snap_take() syscall(547, SNAP_TARGET_NOJUMP, NULL, SNAP_CREATE);

#endif


#endif
