#ifndef BENCH_H
#define BENCH_H

#include <time.h>


#ifdef __cplusplus
extern "C" {
#endif

extern struct timespec diff(struct timespec *start, struct timespec *end);


#ifdef __cplusplus
}
#endif



#endif
