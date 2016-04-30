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
	sbuf[2] = 0;
	int src = 0;
	int lwc1 = 0;
	int lwc2 = 1;

	
	struct lwc_resource_specifier specs[10];

	/* share the file table */
	specs[0].flags = LWC_RESOURCE_FILES | LWC_RESOURCE_SHARE;
	specs[0].sub.descriptors.from = specs[0].sub.descriptors.to = -1;


	void *src_arg = (void*)20;;
	int num_args = 1;
	printf("my uid before first context create is %d\n", getuid());
	int new_lwc = lwccreate(specs, 1, &src, src_arg, &num_args, 0);
	if (new_lwc == LWC_SWITCHED) {
		setuid(1001);
		while(sbuf[2] < 10 && new_lwc != LWC_FAILED) {
			printf("Came out in first one with src=%d and arg=%d and uid=%d\n", src, (int) src_arg, getuid());
			sbuf[2]++;
			num_args = 1;
			new_lwc = lwcsuspendswitch(sbuf[lwc2], NULL, 0, &src, src_arg, &num_args);
		}
	} else if (new_lwc >= 0) {
		sbuf[lwc1] = src = new_lwc;
		printf("my uid before second context create is %d\n", getuid());
		num_args = 1;
		new_lwc = lwccreate(specs, 1, &src, src_arg, &num_args, 0);
		if (new_lwc >= 0) {
			sbuf[lwc2] = new_lwc;
			lwcdiscardswitch(sbuf[lwc1], NULL, 0);
		}
		while(sbuf[2] < 10 && new_lwc != LWC_FAILED) {
			if (sbuf[2] % 2 == 0) {
				specs[0].flags = LWC_RESOURCE_CREDENT | LWC_RESOURCE_COPY;
				lwcoverlay(sbuf[lwc1], specs, 1);
			}
			printf("Came out in second one with src=%d and arg=%d and uid=%d when sbuf[2]=%d\n", src, (int) src_arg, getuid(), sbuf[2]);
			new_lwc = lwcdiscardswitch(sbuf[lwc1], NULL, 0);
		}
	}

	if (new_lwc == LWC_FAILED) {
		printf("LWC failed\n");
		return EXIT_FAILURE;
	}


	return EXIT_SUCCESS;
}
