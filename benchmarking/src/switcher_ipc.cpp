#include <unistd.h>
#include <stdio.h>
#include <sys/procdesc.h>
#include <sys/mman.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>


#include "lwc.h"
#include "bench.h"


using std::cerr;
using std::endl;


#define COUNT 1000000

int child_work_function(int sock) {

	struct timespec start, end;

	int c = 0;

	clock_gettime(CLOCK_REALTIME, &start);

	char buf[1];
	while(c < COUNT) {

		if (read(sock, buf, 1) != 1) {
			perror("child could not read: ");
			return EXIT_FAILURE;
		}
		c++;
		//cerr << "do child\n";
		buf[0] = 'c';

		if (write(sock, buf, 1) != 1) {
			perror("child could not write: ");
		}
	}

	buf[0] = 'e';
	write(sock, buf, 1);

	clock_gettime(CLOCK_REALTIME, &end);
	struct timespec res = diff(&start, &end);
	cerr << "Total time for " << COUNT << " switches is  " << res.tv_sec << " seconds " << res.tv_nsec << " nanoseconds" << endl;
	return 0;
}


int parent_work_function(int sock) {

	char buf[1] = {0};
	for(;buf[0] != 'e';) {
		buf[0] = 'p';
		if (write(sock, buf, 1) != 1) {
			perror("parent could not write: ");
		}
		//cerr << "do parent\n";
		//sleep(1);
		if (read(sock, buf, 1) != 1) {
			perror("parent could not read: ");
			return EXIT_FAILURE;
		}

	}
	return 0;
}


int main(int argc, char *argv[]) {

	int *private_buf = (int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (private_buf == MAP_FAILED) {
		perror("Can't mmap. So many tears\n");
		return 0;
	}

	bzero(private_buf, 4096);

	int * shared_buf = (int*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
	if (shared_buf == MAP_FAILED) {
		perror("Can't mmap. So many tears\n");
		return 0;
	}

	char stack_buf[4096];

	memset(private_buf, 0, 4096);
	memset(shared_buf, 0, 4096); 
	memset(stack_buf, 0, 4096);

	int sv[2];
	int rv = socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
	if (rv)
		perror("socket to me");

	pid_t p = fork();

	if (p == 0) {
		close(sv[0]);
		return child_work_function(sv[1]);
	} else if (p > 0) {
		close(sv[1]);
		return parent_work_function(sv[0]);
	} else {
		perror("fork me");
	}


	return 0;
}
