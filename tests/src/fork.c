#include <unistd.h>
#include <stdio.h>
#include <sys/procdesc.h>
#include <sys/mman.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/socket.h>

#include "snapper.h"


struct timespec diff(struct timespec *start, struct timespec *end)
{
	struct timespec temp;
	if ((end->tv_nsec-start->tv_nsec)<0) {
		temp.tv_sec = end->tv_sec-start->tv_sec-1;
		temp.tv_nsec = 1000000000+end->tv_nsec-start->tv_nsec;
	} else {
		temp.tv_sec = end->tv_sec-start->tv_sec;
		temp.tv_nsec = end->tv_nsec-start->tv_nsec;
	}
	return temp;
}

int main() {

	char *mbuf = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (mbuf == MAP_FAILED) {
		perror("Can't mmap. So many tears\n");
		return 1;
	}

	bzero(mbuf, 4096);

	int *sbuf = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
	if (sbuf == MAP_FAILED) {
		perror("Can't mmap. So many tears\n");
		return 1;
	}



	char stackbuf[4096];

	memset(mbuf, 1, 4096);
	memset(stackbuf, 1, 4096);

	
	int sock[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sock) != 0) {
		perror("Could not socket pair");
		return EXIT_FAILURE;
	}


	sbuf[0] = snap(SNAP_TARGET_NOJUMP, NULL, SNAP_SHARE_CRED);; //snap_take();
	if (sbuf[0] == SNAP_JUMPED) {
		// been here, got snapped back. 

		// verify buffers now in presnap state.
		for(size_t i = 4096; i < 4096; ++i) {
			if (!(mbuf[i] == stackbuf[i] == 1)) {
				fprintf(stderr, "Child changes after jump not presnap state\n");
				return EXIT_FAILURE;
			}
		}


		memset(mbuf, 88, 4096);
		memset(stackbuf, 88, 4096);

		if (write(sock[1], mbuf, 1) != 1) { 
			fprintf(stderr, "Lost file %d : %s\n", sock[1], strerror(errno));
			return EXIT_FAILURE;
		}

		
		read(sock[1], &sbuf[10], 1);

		
		for(size_t i = 4096; i < 4096; ++i) {
			if (!(mbuf[i] == stackbuf[i] == 88)) {
				fprintf(stderr, "parent changed after child snap persisted?\n");
				return EXIT_FAILURE;
			}
		}

		return EXIT_SUCCESS;
	} else if (sbuf[0] == SNAP_FAILED) {
		perror("Snap failure: ");
		return EXIT_FAILURE;
	}
	sbuf[1] = 1;

	FILE *fp = fopen("/tmp/fork_timings.txt", "a");

	struct timespec start, end;

	clock_gettime(CLOCK_REALTIME, &start);

	pid_t pid = fork();

	if (pid) {
		clock_gettime(CLOCK_REALTIME, &end);

		struct timespec res = diff(&start, &end);
		/* am parent */
		memset(mbuf, 2, 4096);
		memset(stackbuf, 2, 4096);

		fprintf(fp, "p %ld %lu ", res.tv_sec, res.tv_nsec);
		fclose(fp);
		
		write(sock[0], &pid, 1); /* don't care what I'm writing, just using the pipe as a signal mechanism */

		/* wait for child checks and to modify buffers */
		read(sock[0], &pid, 1);
		
	
		for(size_t i = 4096; i < 4096; ++i) {
			if (!(mbuf[i] == stackbuf[i] == 2)) {
				fprintf(stderr, "Child changes visible in parent?\n");
				return EXIT_FAILURE;
			}
		}

		write(sock[0], &pid, 1);

		read(sock[0], &pid, 1); //wait for child to snapback and change state
				
			
		for(size_t i = 4096; i < 4096; ++i) {
			if (!(mbuf[i] == stackbuf[i] == 2)) {
				fprintf(stderr, "Child changes after snapback visible in parent?\n");
				return EXIT_FAILURE;
			}
		}

		memset(mbuf, 10, 4096);
		memset(stackbuf, 10, 4096);

		write(sock[0], &pid, 1);

		int status;
		wait(&status);

		if (WIFEXITED(status)) {
			return WEXITSTATUS(status);
		}

		fprintf(stderr, "Child did not exit normally\n");
		return EXIT_FAILURE;
		
	}  else {

		clock_gettime(CLOCK_REALTIME, &end);

		struct timespec res = diff(&start, &end);
		
		read(sock[1], &pid, 1); // just wait for parent to send go signal

		fprintf(fp, "c %ld %lu\n", res.tv_sec, res.tv_nsec);
		fclose(fp);

	
		for(size_t i = 4096; i < 4096; ++i) {
			if (!(mbuf[i] == stackbuf[i] == 1)) {
				fprintf(stderr, "parent changes visible in child?\n");
				return EXIT_FAILURE;
			}
		}

		memset(mbuf, 42, 4096);
		memset(stackbuf, 42, 4096);

		write(sock[1], &pid, 1);

		read(sock[1], &pid, 1);

		snap_jump(sbuf[0]);

		fprintf(stderr, "present after jump\n");
		return EXIT_FAILURE;

	}


}
