#include <unistd.h>
#include <stdio.h>
#include <sys/procdesc.h>
#include <sys/mman.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/capsicum.h>

#include "lwc.h"

static char *sbuf;
static int monitor_lwc = 0;
static int monitored_lwc = 1;


#define strerror(x) (sys_errlist[x])
//#define fprintf(...)

void parent_loop(int ret, int src, register_t *to_args, register_t *from_args, int num_from) {

	while(1) {

		if (ret == LWC_FAILED) {
			perror("LWC failed in suspend\n");
			exit(2);
		} else if (ret != LWC_TRAPPED) {
			fprintf(stderr, "Got into refmon but didn't get trap retval, got %d instead", ret);
			exit(3);
		}

		/* handle syscall */

		/* first value is errno, second is return code */

		if (num_from < 1) {
			fprintf(stderr, "Not enough args\n");
			to_args[0] = EINVAL;
			to_args[1] = -1;
		} else {

			/* NOte that any pointers in from_args is pointing to the
			 * other lwc's memory. if we don't have it we can't see
			 * it. */


			if (from_args[0] == SYS_stat) {
				/* arg[0] == syscall number, arg[i] is (i-1)th argument to syscall */
				/* for printing we will assume arguments are all from
				 * shared memory. other stuff tests overlays */

				const char *path = (const char*) from_args[1];
				fprintf(stderr, "attempting to stat %s for child from parent\n", path);
			} else if (from_args[0] == SYS_open) {
				fprintf(stderr, "attempting open call of 0x%lx from child\n", from_args[0]);
			} else {
				fprintf(stderr, "attempting other syscall %ld %ld \n", from_args[0], from_args[1]);
			}

			fprintf(stderr, "[1] (%ld) 0x%lx, [2] (%ld) 0x%lx, [3] (%ld) 0x%lx\n", from_args[1], from_args[1], from_args[2], from_args[2], from_args[3], from_args[3]);
			errno = 0;
			to_args[1] = lwcsyscall(src, LWCR_FILES | LWCR_MEMORY, from_args[0], &from_args[1]);
			to_args[0] = errno;
			
		} 

		fprintf(stderr, "returning %ld %ld %s\n", to_args[0], to_args[1], strerror(errno));

		num_from = 10;
		ret = lwcswitch(sbuf[monitored_lwc], to_args, 2, NULL, from_args, &num_from);
	}
}

int main() {

	printf("Hit enter for go time\n");
	getchar();

	sbuf = mmap(NULL, 4096*10, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
	if (sbuf == MAP_FAILED) {
		perror("Can't mmap. So many tears\n");
		return 0;
	}

	bzero(sbuf, 4096);

	


	struct lwc_resource_specifier specs[10];
	/* share the file table */
	specs[0].flags = LWC_RESOURCE_FILES | LWC_RESOURCE_SHARE;
	specs[0].sub.descriptors.from = specs[0].sub.descriptors.to = -1;



	register_t to_args[11];
	register_t from_args[11];
	int num_from = 11;
	int src;


	int new_lwc = lwccreate(specs, 0, &src, from_args, &num_from, 0);
	if (new_lwc >= 0) {
		sbuf[monitor_lwc] = new_lwc;
		sbuf[monitored_lwc] = lwcgetlwc();

		int rv = fcntl(new_lwc, F_SETLWC, FLWC_TRAPTO);
		if (rv) {
			fprintf(stderr, "fcntl return value was %d: %s\n", rv, strerror(errno));
			return EXIT_FAILURE;
		}

		goto inner_main;
		
	} else if (new_lwc == LWC_FAILED) {
		perror("LWC failed\n");
		return EXIT_FAILURE;
	} else {
		parent_loop(new_lwc, src, to_args, from_args, num_from);
	}




  inner_main:
	setuid(1001);

	if (cap_enter()) {
		perror("Could not enter sandbox");
		return EXIT_FAILURE;
	}

	int ret;
	char *path = sbuf + 10;
	strcpy(path, "/etc/passwd");
	struct stat *sb = (struct stat *) (sbuf + 100);

	errno = 0;
	ret = stat(path, sb);
	if (ret == -1) {
		fprintf(stderr, "stat had an error: %s\n", strerror(errno));
	}
	fprintf(stderr, "return value of stat is %d and errno is %d. size is %ld\n", ret, errno, sb->st_size);

	ret = open(path, O_RDONLY);
	fprintf(stderr, "return value of open is %d and errno is %d and %s.\n", ret, errno, strerror(errno));

	char buf[128] = { 0 };
	fprintf(stderr, "read returned %zd\n", read(ret, buf, sizeof(buf)-1));
	fprintf(stderr, "errstr is %s\n", strerror(errno));
	//fprintf(stderr, "buf contains %s\n", buf);

	size_t frames = 42;
	size_t oldlen = sizeof(frames);
	ret = sysctlbyname("kern.lwc.max_frames", &frames, &oldlen, NULL, 0);
	fprintf(stderr, "return of sysctlbyname is %d and frames is now set to %zd. errstr(%d)=%s\n", ret, frames, errno, strerror(errno));

	fprintf(stderr, "exiting okay\n");
	return EXIT_SUCCESS;
}
