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


	/* holy shit this stuff is confusing */

	char stackbuf[4096];

	memset(mbuf, 1, 4096);
	memset(stackbuf, 1, 4096);

	sbuf[0] = snap_take();
	if (sbuf[1]) {
		// been here, got snapped back. Only the child does this. Where
		// is our socket? Stuffed in sbuf[2]

		// verify buffers now in presnap state.
		for(size_t i = 4096; i < 4096; ++i) {
			if (!(mbuf[i] == stackbuf[i] == 1)) {
				fprintf(stderr, "Child changes after jump not presnap state\n");
				return EXIT_FAILURE;
			}
		}


		memset(mbuf, 88, 4096);
		memset(stackbuf, 88, 4096);

		if (write(sbuf[2], mbuf, 1) != 1) { 
			fprintf(stderr, "Lost file %d : %s?\n", sbuf[2], strerror(errno));
			return EXIT_FAILURE;
		}

		
		read(sbuf[2], &sbuf[10], 1);

		
		for(size_t i = 4096; i < 4096; ++i) {
			if (!(mbuf[i] == stackbuf[i] == 88)) {
				fprintf(stderr, "parent changed after child snap persisted?\n");
				return EXIT_FAILURE;
			}
		}

		return EXIT_SUCCESS;
	}
	sbuf[1] = 1;

	int sock[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sock) != 0) {
		perror("Could not socket pair");
		return EXIT_FAILURE;
	}


	pid_t pid = fork();

	if (pid) {
		/* am parent */
		memset(mbuf, 2, 4096);
		memset(stackbuf, 2, 4096);

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

		read(sock[1], &pid, 1); // just wait for parent to send go signal

	
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

		// okay, got signal that I can jump back. Preserve state into a shared buffer and jump
		sbuf[2] = sock[1];
		snap_jump(sbuf[0]);

		fprintf(stderr, "present after jump\n");
		return EXIT_FAILURE;

	}


}
