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
#include <sys/syscall.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/capsicum.h>

#include "lwc.h"

static char *sbuf;
static int parent_lwc = 0;
static int child_lwc = 1;

void parent_loop() {


	register_t to_args[11];
	register_t from_args[11];
	int num_from = 11;
	if (lwcsuspendswitch(sbuf[child_lwc], to_args, 0, NULL, from_args, &num_from) == LWC_FAILED) {
		perror("LWC suspend in parent failed: ");
		exit(1);
	}

	while(1) {
		/* handle syscall */
		register_t to_args[10];
		/* first value is errno, second is return code */

		if (num_from < 1) {
			goto inval;
		} else {
			if (from_args[0] == SYS_stat) {
				/* arg[0] == syscall number, arg[i] is (i-1)th argument to syscall */
				if (num_from != 3)
					goto inval;

				/* this time we will assume arguments are all from
				 * shared memory. other stuff tests overlays */

				const char *path = (const char*) from_args[1];
				struct stat *sb = (struct stat *) from_args[2];
				fprintf(stderr, "attempting to stat %s for child from parent\n", path);
				errno = 0;
				to_args[1] = stat(path, sb);;
				to_args[0] = errno;
				goto ret;

			}
			goto inval; /* test program only does stat */
		}

	  inval:
		to_args[0] = EINVAL;
		to_args[1] = -1;

	  ret:

		fprintf(stderr, "returning %ld %ld\n", to_args[0], to_args[1]);

		num_from = 10;
		if (lwcsuspendswitch(sbuf[child_lwc], to_args, 2, NULL, from_args, &num_from) == LWC_FAILED) {
			perror("LWC failed in suspend\n");
			exit(2);
		}
	}
}

int main() {

	setuid(1001);
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


	sbuf[parent_lwc] = lwcgetlwc();

	int new_lwc = lwccreate(specs, 1, NULL, NULL, NULL, LWC_TRAP_SYSCALL);
	if (new_lwc >= 0) {
		sbuf[child_lwc] = new_lwc;
		parent_loop();
	} else if (new_lwc == LWC_FAILED) {
		perror("LWC failed\n");
		return EXIT_FAILURE;
	}

	/* now in child */

	if (cap_enter()) {
		perror("Could not enter sandbox");
		return EXIT_FAILURE;
	}

	char *path = sbuf + 10;
	strcpy(path, "/etc/passwd");
	struct stat *sb = (struct stat *) (sbuf + 100);

	errno = 0;
	int ret = stat(path, sb);
	if (ret == -1) {
		printf("stat had an error: %s\n", strerror(errno));
	}
	printf("return value of stat is %d and errno is %d. size is %ld\n", ret, errno, sb->st_size);
	return EXIT_SUCCESS;
}
