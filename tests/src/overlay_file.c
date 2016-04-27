#include <unistd.h>
#include <stdio.h>
#include <sys/procdesc.h>
#include <sys/mman.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "lwc.h"

int main() {

	int *sbuf = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
	if (sbuf == MAP_FAILED) {
		perror("Can't mmap. So many tears\n");
		return 0;
	}

	int src = -1;
	int parent = 0;
	int lwc1 = 1;
	int lwc2 = 2;

	
	struct lwc_resource_specifier specs[10];

	int new_lwc = lwccreate(NULL, 0, &src, NULL, NULL, 0);
	if (new_lwc >= 0) {
		sbuf[parent] = new_lwc;
		lwcdiscardswitch(new_lwc, NULL, 0);
	} else if (new_lwc == LWC_SWITCHED) {
		printf("just manually creating the proto snap/lwc to play around and allow for suspendswitch src messed because wasn't in proto ctx %d\n", src);
	} else if (new_lwc == LWC_FAILED) {
		printf("I suck\n");
		return EXIT_FAILURE;
	}

	unlink("/tmp/parent1.txt");
	unlink("/tmp/parent2.txt");
	int fd1 = open("/tmp/parent1.txt", O_RDWR | O_CREAT | O_TRUNC);
	int fd2 = open("/tmp/parent2.txt", O_RDWR | O_CREAT | O_TRUNC);

	if (fd1 < 0 || fd2 < 0) {
		perror("could not open: \n");
		return EXIT_FAILURE;
	}

	/* only let them snap to parent */
	specs[0].flags = LWC_RESOURCE_FILES | LWC_RESOURCE_UNMAP;
	specs[0].sub.descriptors.from = sbuf[parent]+1;
	specs[0].sub.descriptors.to = 1000;


	printf("sbuf[parent] + 1 = %d\n", sbuf[parent]+1);
	new_lwc = lwccreate(&specs, 1, NULL, NULL, 0, 0);
	if (new_lwc >= 0) {
		sbuf[lwc1] = new_lwc;
		new_lwc = lwccreate(&specs, 1, NULL, NULL, 0, 0);
		if (new_lwc >= 0) {
			sbuf[lwc2] = new_lwc;
		} else if (new_lwc == LWC_FAILED) {
			printf("creating lwc 2 failed\n");
			return EXIT_FAILURE;
		} else {
			struct stat buf;
			for(;;) {
				// lwc 2 context
				if (fstat(fd2, &buf) == 0) {
					fprintf(stderr, "fd2 accessible at %d\n", fd2);
					return EXIT_FAILURE;
				} else {
					fprintf(stderr, "overlaying fd2 (%d)...\n", fd2);
					specs[0].flags = LWC_RESOURCE_FILES | LWC_RESOURCE_COPY;
					specs[0].sub.descriptors.from = specs[0].sub.descriptors.to = fd2;
					lwcoverlay(sbuf[parent], &specs, 1);
					char str[] = "lwc 2 writing into fd2\n";
					fprintf(stderr, "%s", str);
					if (write(fd2, str, sizeof(str)-1) == -1) {
						perror("Could not write to fd2 even after overlay\n");
						return EXIT_FAILURE;
					}
				}
				lwcdiscardswitch(sbuf[parent], 0, 0);
			}
		}
	} else if (new_lwc == LWC_FAILED) {
		printf("creating lwc 1 failed\n");
		return EXIT_FAILURE;
	} else {
		struct stat buf;
		for(;;) {
			// lwc 1 context
			if (fstat(fd1, &buf) == 0) {
				fprintf(stderr, "fd1 accessible\n");
				return EXIT_FAILURE;
			} else {
				fprintf(stderr, "overlaying fd1 (%d)...\n", fd1);
				specs[0].flags = LWC_RESOURCE_FILES | LWC_RESOURCE_COPY;
				specs[0].sub.descriptors.from = specs[0].sub.descriptors.to = fd1;
				lwcoverlay(sbuf[parent], &specs, 1);
				char str[] = "lwc 1 writing into fd1\n";
				fprintf(stderr, "%s", str);
				if (write(fd1, str, sizeof(str)-1) == -1) {
					perror("Could not write to fd1 even after overlay\n");
					return EXIT_FAILURE;
				}
			}
			lwcdiscardswitch(sbuf[parent], 0, 0);
		}
	}
	
	int next = lwc1;
	for(sbuf[10] = 0; sbuf[10] < 2; sbuf[10] ++) {
		next = next == lwc2 ? lwc1 : lwc2;
		fprintf(stderr, "main context suspending to %d\n", sbuf[next]);
		lwcsuspendswitch(sbuf[next], NULL, 0, NULL, NULL, NULL);
	}

	return EXIT_SUCCESS;
}
