#include <unistd.h>
#include <stdio.h>
#include <sys/procdesc.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>
#include <getopt.h>

#include "lwc.h"

int main() {
	for(int i = 0; i < 10; ++i) {
		printf("lwcgetlwc() = %d\n", lwcgetlwc());
		sleep(1);
	}
	return 0;
}
