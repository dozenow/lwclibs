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
#include "lwc.h"

int main() {

	setuid(1001);
	char *mbuf = mmap(NULL, 4096*10, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (mbuf == MAP_FAILED) {
		perror("Can't mmap. So many tears\n");
		return 0;
	}

	bzero(mbuf, 4096);

	int *sbuf = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
	if (sbuf == MAP_FAILED) {
		perror("Can't mmap. So many tears\n");
		return 0;
	}

	char stackbuf[4096];

	memset(mbuf, 1, 4096);
	memset(stackbuf, 1, 4096);

	sbuf[0] = -1;
	sbuf[1] = -1;
	sbuf[3] = 1;
	int src = 0;
	int lwc1 = 0;
	int lwc2 = 1;


	
	struct lwc_resource_specifier specs[10];

	for(size_t i = 0; i < 5; i++) {
		specs[i].flags = LWC_RESOURCE_MEMORY | LWC_RESOURCE_SHARE;
		specs[i].sub.memory.start = (vm_offset_t)mbuf + (2*i*4096);;
		specs[i].sub.memory.end = (vm_offset_t)mbuf + ((2*i+1)*4096);;
	}

	/* share the file table */
	specs[5].flags = LWC_RESOURCE_FILES | LWC_RESOURCE_SHARE;
	specs[5].sub.descriptors.from = specs[5].sub.descriptors.to = -1;

	/* share every other page */


	sbuf[2] = 0;
	sbuf[3] = 0;
	register_t src_arg[10] = {24};;
	int num_args = 1;
	printf("num args is at 0x%lx and src arg is at 0x%lx\n", (unsigned long) &num_args,
	       (unsigned long) src_arg);

	for(int i = 0; i < 100; ++i) {
		lwcclose(lwccreate(specs, 6, &src, src_arg, &num_args, 0));
	}

	int new_lwc = lwccreate(specs, 6, &src, src_arg, &num_args, 0);
	if (new_lwc == LWC_SWITCHED) {
		register_t regs[10] = { (register_t) "/tmp/FOO", O_CREAT, 0 };
		int fd = lwcsyscall(src, LWCR_CREDENT, SYS_open, regs);
		close(fd);
		printf("Came out in first one with src=%d and arg=%ld with %d args\n", src, src_arg[0], num_args);
	}
	if (new_lwc >= 0) {
		sbuf[lwc1] = src = new_lwc;
		new_lwc = lwccreate(specs, 6, &src, src_arg, &num_args, 0);
		if (new_lwc == LWC_SWITCHED)
			printf("Came out in second one with src=%d and arg=%ld with %d args\n", src, src_arg[0], num_args);
		if (new_lwc >= 0) {
			sbuf[lwc2] = new_lwc;
		}
	}

	if (new_lwc == LWC_FAILED) {
		perror("LWC failed\n");
		return EXIT_FAILURE;
	}

	while(sbuf[2] < 10) {
		sbuf[2]++;
		for(size_t i = 0; i < 10; ++i) {
			printf("mbuf[%lu] = %d\n", 4096*i+1, mbuf[4096 * i + 1]);
		}
		memset(mbuf, sbuf[2], 4096*10);
		sbuf[3] = (sbuf[3] + 1) % 2;
		printf("Switching to %d\n", sbuf[sbuf[3]]);
		register_t out[10] = { sbuf[2] };
		lwcdiscardswitch(sbuf[sbuf[3]], out, 1);
		printf("Oops!\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
