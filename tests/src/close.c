#include <unistd.h>
#include <stdio.h>
#include <sys/procdesc.h>
#include <sys/mman.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <errno.h>
#include "lwc.h"

int main() {

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
	bzero(sbuf, 4096);


	
	struct lwc_resource_specifier specs[10];

	/* share the file table */
	specs[0].flags = LWC_RESOURCE_FILES | LWC_RESOURCE_SHARE;
	specs[0].sub.descriptors.from = specs[0].sub.descriptors.to = -1;

	/* share every other page */
	for(size_t i = 1; i < 6; i++) {
		specs[i].flags = LWC_RESOURCE_MEMORY | LWC_RESOURCE_SHARE;
		specs[i].sub.memory.start = (vm_offset_t)mbuf + (2*i*4096);;
		specs[i].sub.memory.end = (vm_offset_t)mbuf + ((2*i+1)*4096);;
	}

	for(int i = 0; i < 10; ++i) {
		sbuf[i] = lwccreate(specs, 0, NULL, 0, NULL, 0);
		if (sbuf[i] < 0) {
			fprintf(stderr, "failed to create lwc: %s\n", strerror(errno));
			break;
		} 
	}

	for(int i = 0; i < 10; ++i) {
		if (sbuf[i] >= 0) {
			fprintf(stderr, "closing %d lwc\n", i);
			close(sbuf[i]);
		}
	}

/*
	fprintf(stderr, "hit any key to exit\n");
	getchar();
*/

	return EXIT_SUCCESS;
}
