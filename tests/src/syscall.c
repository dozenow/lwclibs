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
#include "lwc.h"

int main(int argc, const char *argv[]) {

	/* we are going to create a file as root after doing setuid, assuming you run this via sudo */

	
	int *sbuf = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
	if (sbuf == MAP_FAILED) {
		perror("Can't mmap. So many tears\n");
		return 0;
	}


	struct lwc_resource_specifier specs[10];
	/* share the file table, not the point of this exercise otherwise */
	specs[0].flags = LWC_RESOURCE_FILES | LWC_RESOURCE_SHARE;
	specs[0].sub.descriptors.from = specs[0].sub.descriptors.to = -1;

	if (getuid() != 0) {
		printf("Run test as root\n");
		return 1;
	}

	int src = 32;
	int num_args = 10;
	register_t src_args[10];
	int ret = lwccreate(specs, 1, &src, &src_args[0], &num_args, 0);
	printf("ret is %d and src is %d address is 0x%lx and uid=%d euid=%d\n", ret, src, (unsigned long)&src, getuid(), geteuid());
	if (ret >= 0) {
		sbuf[0] = ret;

		setuid(1001);
		sbuf[1] = lwcgetlwc();
		char local_string[20];
		strcpy(local_string, "/tmp/foobar");
		printf("local string is at 0x%lx %s. My id is %d. Will switch to %d\n", (unsigned long) local_string, local_string, getuid(), sbuf[0]);
		register_t foo[10] = { (register_t)local_string };
		num_args = 1;
		int src;
		num_args = 10;
		lwcsuspendswitch(sbuf[0], foo, 1, &src, src_args, &num_args);

		printf("got back %d\n", src);
		return 0;

	} else if (ret == LWC_SWITCHED) {

		if (getuid() != 0) {
			printf("uid not de-escalated in lwc=%d, uid=%d euid=%d\n", lwcgetlwc(), getuid(), geteuid());
			return 4;
		}
		register_t foo[10];

		printf("being asked to open 0x%lx for %d with %d args and uid %d in lwc %d\n", src_args[0], src, num_args, getuid(), lwcgetlwc());
		int fd = lwcsyscall(src, LWCR_MEMORY | LWCR_FILES, SYS_open, src_args[0], O_CREAT, 0);
		printf("got %d : %s\n", fd, fd < 0 ? strerror(errno) : "open successful");
		foo[0] = fd;
		int src;
		lwcsuspendswitch(sbuf[1], foo, 1, &src, src_args, &num_args);
		return 3;
			
	}
}
