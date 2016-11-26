#include <unistd.h>
#include <stdio.h>
#include <sys/procdesc.h>
#include <sys/mman.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

#include <pthread.h>
#include <stdatomic.h>

#include "shared_malloc.h"
#include "lwc.h"



/* global crud */
static struct sh_memory_pool *g_pool;
static int *g_lwcs;
static atomic_int *g_cnt;

#define N_THR 4
#define N_LWCS 5
#define POOL_SIZE (getpagesize() * 10)

void * Smalloc(size_t s) {
	char * ret = sh_malloc(s, g_pool);
	if (!ret) abort();
	return ret;
}

void * thr_start(void * arg) {
	int id = (size_t) arg;

	int *count = Smalloc(sizeof(*count));

	*count = 0;

	atomic_fetch_add(g_cnt, 1);

	for(int x = atomic_load(g_cnt); x < N_THR+1; x = atomic_load(g_cnt)) {
		//fprintf(stderr, "x =%d\n", x);
		usleep(100);
	}

	*count += 1;
	fprintf(stderr, "thread %d(0x%ld) passed fake barrier for the %d time in lwc %d\n", id, (unsigned long) pthread_self(), *count, lwcgetlwc());
	if (*count == N_LWCS) {
		fprintf(stderr, "%d switching to g_lwcs[0]=%d\n", id, g_lwcs[0]);
		if (Lwcswitch(g_lwcs[0], NULL, 0, NULL, NULL, 0) == LWC_FAILED) {
			fprintf(stderr, "switch failed: %s\n", strerror(errno));
			abort();
		}
	}

	/* this thread should be at the barrier for every lwc created */
	for(int i = 1; i <= N_LWCS; ++i) {
		if (g_lwcs[i] != lwcgetlwc()) {
			fprintf(stderr, "il %d switching to g_lwcs[%d]=%d\n", id, i, g_lwcs[i]);
			if (Lwcswitch(g_lwcs[i], NULL, 0, NULL, NULL, 0) == LWC_FAILED) {
				fprintf(stderr, "switch failed: %s\n", strerror(errno));
				abort();
			}

			if (lwcgetlwc() == g_lwcs[0]) {
				fprintf(stderr, "%d exiting\n", id);
				return arg;
			}
		}
	}

	fprintf(stderr, "should not get here\n");
	return NULL;

}


void init_globals() {
	char *sbuf = mmap(NULL, POOL_SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
	if (sbuf == MAP_FAILED) {
		perror("Can't mmap. So many tears\n");
		abort();
	}

	g_pool = init_sh_mempool(sbuf, POOL_SIZE);

	g_cnt = Smalloc(sizeof(*g_cnt));
	*g_cnt = 0;
	g_lwcs = Smalloc(sizeof(*g_lwcs)*(N_LWCS+1));
}

int main() {

	printf("Hit enter for go time\n");
	getchar();

	init_globals();



	/* do out of smalloc in case it gets odd with isolated address spaces */
	pthread_t *thrs = Smalloc(sizeof(*thrs) * (N_THR+1)); /* one index for consistency with lws */
	for(int i = 1; i <= N_THR; ++i) {
		int ret = pthread_create(&thrs[i], NULL, thr_start, (void*) (size_t)i);
		if (ret) {
			fprintf(stderr, "pthread_create failed: %s\n", strerror(errno));
		}
	}


	
	struct lwc_resource_specifier specs[1];

	/* share the file table */
	specs[0].flags = LWC_RESOURCE_FILES | LWC_RESOURCE_SHARE;
	specs[0].sub.descriptors.from = specs[0].sub.descriptors.to = -1;

	g_lwcs[0] = lwcgetlwc();

	/* okay all lwcs are created. spin for all to startup */
	while(atomic_load(g_cnt) < N_THR) {
		usleep(10);
	}

	usleep(10); //technically can race, but just a dumb test. 
	fprintf(stderr, "everyone at their barrier, creating lwcs\n");


	int src;
	for(int i = 1; i <= N_LWCS; ++i) {
		int new_snap = lwccreate(specs, 1, &src, 0, 0, 0);

		if (new_snap >= 0) { // created a lwc
			g_lwcs[i] = new_snap;
		} else if (new_snap == LWC_SWITCHED) {
			fprintf(stderr, "switched into lwc at i = %d, lwc is %d from %d\n", i, lwcgetlwc(), src);
			return 10; //not supposed to happen in this test.
			

			Lwcswitch(g_lwcs[(i+1) % N_LWCS], NULL, 0, NULL, NULL, 0);
			fprintf(stderr, "should not have reached this point\n");
			return 8;

		} else if (new_snap == LWC_FAILED) {
			fprintf(stderr, "error doing snap create / jump to snap create\n");
			return 3;
		} else {
			fprintf(stderr, "unexpected return value from create: %d\n", new_snap);
			return 4;
		}
	}


	/* okay, unleash the threads */
	atomic_fetch_add(g_cnt, 1);
	for(;atomic_load(g_cnt) < N_THR+1;) {
		usleep(10);
	}

	fprintf(stderr, "awaiting join in parent\n");

	for(int i = 1; i <= N_THR; ++i) {
		void *val;
		int rv;
		if ((rv = pthread_join(thrs[i], &val)) != 0) {
			fprintf(stderr, "pthread_join failed on thr %d with rv %d: %s\n", i, rv, strerror(errno));
			break;
		}
	}

	return 0;

}
