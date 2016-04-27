#include <unistd.h>
#include <stdio.h>
#include <sys/procdesc.h>
#include <sys/mman.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdlib.h>
#include <time.h>

#include "lwc.h"

int main() {

	char *mbuf = mmap(NULL, 4096*10, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (mbuf == MAP_FAILED) {
		perror("Can't mmap. So many tears\n");
		return 0;
	}
	printf("mbuf is 0x%lx\n", (unsigned long) mbuf);

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

	/* share the file table */
	specs[0].flags = LWC_RESOURCE_FILES | LWC_RESOURCE_SHARE;
	specs[0].sub.descriptors.from = specs[0].sub.descriptors.to = -1;

	/* share every other page */

	sbuf[2] = 0;
	sbuf[3] = 0;
	void *src_args[1];
	size_t num_args = 1;
	int new_lwc = lwccreate(&specs, 1, &src, src_args, &num_args, 0);
	if (new_lwc == LWC_SWITCHED)
		printf("Came out in first one with src=%d and arg=%d\n", src, (int) src_args[0]);
	if (new_lwc >= 0) {
		sbuf[lwc1] = src = new_lwc;
		specs[1].flags = LWC_RESOURCE_MEMORY | LWC_RESOURCE_UNMAP;
		specs[1].sub.memory.start = (vm_offset_t)mbuf + 0;
		specs[1].sub.memory.end = (vm_offset_t)mbuf + 10*4096;
		num_args = 1;
		new_lwc = lwccreate(&specs, 2, &src, &src_args, &num_args, 0);
		if (new_lwc == LWC_SWITCHED) { 
			printf("Came out in second one with src=%d and arg=%d\n", src, (int) src_args[0]);
			//getchar();

			for(size_t i = 0; i < 5; i++) {
				specs[i].flags = LWC_RESOURCE_MEMORY | LWC_RESOURCE_SHARE;
				specs[i].sub.memory.start = (vm_offset_t)mbuf + (2*i*4096);
				specs[i].sub.memory.end = (vm_offset_t)mbuf + ((2*i+1)*4096);
				printf("overlaying 0x%lx - 0x%lx %d\n", (unsigned long) specs[i].sub.memory.start,
				       (unsigned long) specs[i].sub.memory.end, specs[i].sub.memory.end - specs[i].sub.memory.start);
			}
			printf("doing overlay\n");
			//getchar();
			lwcoverlay(sbuf[lwc1], &specs, 5);
			printf("Overlay done\n");
			//getchar();
			for(size_t i = 0; i < 5; i++) {
				if (mmap(mbuf + ((i*2+1)*4096), 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0) == MAP_FAILED) {
					perror("mmap failed\n");
				}
			}
			memset(mbuf, 42, 4096*10);
			printf("now filled in with mmap\n");
			//getchar();
			
		} else if (new_lwc >= 0) {
			sbuf[lwc2] = new_lwc;
		}
	}

	if (new_lwc == LWC_FAILED) {
		printf("LWC failed\n");
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
		lwcdiscardswitch(sbuf[sbuf[3]], sbuf[2], 1);
		printf("Oops!\n");
		return EXIT_FAILURE;
	}


	return EXIT_SUCCESS;
}
