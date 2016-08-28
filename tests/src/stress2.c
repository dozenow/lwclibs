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

struct opts {
	unsigned int lwcs;
	unsigned int switches;
	unsigned int seconds;
	unsigned int children;
	unsigned int pages;
	unsigned int groups;
};

int getuint(const char *s, unsigned int *retval) {
	char *endptr;
	long rvl = strtol(s, &endptr, 10);
	if ((endptr != s) && (rvl >= 0 && rvl <= UINT_MAX)) {
		*retval = (unsigned int)rvl;
		return 0;
	}
	return -1;
}

static struct opts g_options = {
	.lwcs = 10,
	.switches = 1,
	.seconds = 10,
	.children = 1,
	.pages = 0,
	.groups = 0,
};


int readopts(int argc, char *const argv[], struct opts *res) {
	const char optstring[] = "l:s:w:c:p:h";
	const char usage[] ="options\n"
		"\t-l Number of LWCS to have active in the lwc pool\n"
		"\t-s Number of switches to perform before an lwc must be replaced\n"
		"\t-w Number of seconds in the time window before outputting timing statistics\n"
		"\t-c Number of children to fork\n"
		"\t-p Number of pages per page group to\n"
		"\t-h Print usage message\n";

	int c;
	int rv = 0;
	while(!rv && ((c=getopt(argc, argv, optstring)) != -1)) {
		switch(c) {
		case 'l':
			rv = getuint(optarg, &res->lwcs);
			break;
		case 's':
			rv = getuint(optarg, &res->switches);
			break;
		case 'w':
			rv = getuint(optarg, &res->seconds);
			break;
		case 'c':
			rv = getuint(optarg, &res->children);
			break;
		case 'p':
			rv = getuint(optarg, &res->pages);
			break;
		case 'g':
			rv = getuint(optarg, &res->groups);
			break;
		default:
			rv = -1;
			break;
		}
	}

	if (rv) {
		puts(usage);
	}

	return rv;
}

struct child_data {
	int pipe;
	pid_t pid;
};

struct record {
	unsigned int creations;
	unsigned int switches;
};

int do_child(int pipe, const struct opts *opts) {

	struct lwc_resource_specifier specs[10];

	/* share the file table */
	specs[0].flags = LWC_RESOURCE_FILES | LWC_RESOURCE_SHARE;
	specs[0].sub.descriptors.from = specs[0].sub.descriptors.to = -1;
	specs[1].flags = LWC_RESOURCE_CREDENT | LWC_RESOURCE_SHARE;
	//specs[2].flags = LWC_RESOURCE_MEMORY | LWC_RESOURCE_SHARE;
	//specs[2].sub.memory.start = specs[2].sub.memory.end = -1;

	/* so the pattern here is create child lwcs from the parent, switch in and out of them, and destroy them from the parent */

	int parent_snap = lwcgetlwc();

	if (parent_snap < 0) {
		printf("do not have parent snap here\n");
		sleep(1);
		abort();
	} else {
		//printf("parent snap is %d\n", parent_snap);
	}

	int *snaplist = alloca(opts->lwcs * sizeof(*snaplist));
	unsigned int *switchcnt = alloca(opts->lwcs * sizeof(*switchcnt));
	unsigned int snapcnt = 0;

	bzero(switchcnt, sizeof(*switchcnt) * opts->lwcs);
	memset(snaplist, ~0, sizeof(*snaplist) * opts->lwcs);

	time_t start = time(NULL);
	srand(start);
	time_t last = start;
	unsigned int creations = 0;
	unsigned int switches = 0;

	for(;;) {
		if (snapcnt == opts->lwcs) {
			int idx = rand() % opts->lwcs;
			int nxt = snaplist[idx];

			if (switchcnt[idx] < opts->switches) {
				if (lwcsuspendswitch(nxt, NULL, 0, NULL, NULL, 0) == LWC_FAILED) {
					printf("Failed to suspend: %s\n", strerror(errno));
					return EXIT_FAILURE;
				}
				++switches;
				++switchcnt[idx];
			}

			if (switchcnt[idx] >= opts->switches) {
				lwcclose(snaplist[idx]);
				snaplist[idx] = -1;
				snapcnt--;
			}
		} else {
			int new_snap = lwccreate(specs, 2, NULL, 0, 0, LWC_SUSPEND_ONLY);
			if (new_snap >= 0) {

				//lwcclose(new_snap);
				//exit(0);
				int idx = 0;
				for(;snaplist[idx] > 0; ++idx);
				snaplist[idx] = new_snap;
				switchcnt[idx] = 0;
				snapcnt++;

				creations++;

				time_t cur = time(NULL);
				if (cur - last >= opts->seconds) {
					struct record rec = {
						.creations = creations,
						.switches = switches
					};
					creations = 0;
					switches = 0;
					last = cur;
					ssize_t wrote = write(pipe, &rec, sizeof(rec));
					if (wrote < (ssize_t) sizeof(rec)) {
						printf("failed to write statistics (%s), bailing\n", strerror(errno));
						return EXIT_FAILURE;
					}
				}


			} else if (new_snap == LWC_SWITCHED) {

				for(;;) { /* child loop */
					if (lwcsuspendswitch(parent_snap, NULL, 0, NULL, NULL, 0) == LWC_FAILED)
						return EXIT_FAILURE;
				}
			} else if (new_snap == LWC_FAILED) {
				perror("Can't create snap");
				return EXIT_FAILURE;
			}
		}
	}
	return 0;
}

/* return must mean in parent and success */
void do_fork(struct child_data *kid) {
	int desc[2];
	if (pipe(desc)) {
		perror("pipe failed: ");
		exit(EXIT_FAILURE);
	}


	pid_t res = fork();
	if (res > 0) {
		kid->pid = res;
		kid->pipe = desc[0];
		close(desc[1]);
		/* only valid return path */
	} else if (res == 0) {
		close(desc[0]);
		exit(do_child(desc[1], &g_options));
	} else {
		perror("fork: ");
		exit(EXIT_FAILURE);
	}
}

static struct child_data *kids;

void proc_exit() {
	for(;;) {
		int handled = 0;
		int status;
		pid_t p = wait3(&status, WNOHANG, NULL);
		for(unsigned int i = 0; i < g_options.children; ++i) {
			if (kids[i].pid == p) {
				kids[i].pid = -1;
				close(kids[i].pipe);
				handled = 1;
				printf("child %d exited(%d) with status %d\n", p, WIFEXITED(status), WEXITSTATUS(status));
				break;
			}
		}
		if (!handled) {
			printf("Did not correctly handle child %d\n", p);
		}
	}
}


int main(int argc, char * const argv[]) {


	if (readopts(argc, argv, &g_options)) {
		return EXIT_FAILURE;
	}

	kids = malloc((g_options.children)* sizeof(*kids));

	for(unsigned int g = 0; g < g_options.groups; ++g) {
		char *mbuf = mmap(NULL, getpagesize() * g_options.pages, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
		if (mbuf == MAP_FAILED) {
			printf("Can't mmap in group %d: %s\n", g, strerror(errno));
			return EXIT_FAILURE;
		}
		/* touch all the pages */
		bzero(mbuf, getpagesize() * g_options.pages);

		/* and...leak the pages */
	}

	for(unsigned int i = 0; i < g_options.children; ++i) {

		/* setup so return means in parent and success. */
		do_fork(&kids[i]);


	}

	for(int w = 0;;++w) {

		struct record sum = {0,0};
		for(unsigned int i = 0; i < g_options.children; ++i) {
			if (kids[i].pid == -1) {
				do_fork(&kids[i]);
			} else {
				//printf("parent attempting to read child %d\n", i);
				struct record rec;
				ssize_t rv = read(kids[i].pipe, &rec, sizeof(rec));
				if (rv < (ssize_t)sizeof(rec)) {
					printf("got %zd of %lu bytes, errstr = %s\n", rv, sizeof(rec), strerror(errno));
					return EXIT_FAILURE;
				}
#if 0
				printf("For window [%d,%d) and child %d got %.1f creations per second and %.1f switches per second\n",
				       w*g_options.seconds, (w+1)*g_options.seconds - 1,i,
				       (1.0*rec.creations) / (1.0 *g_options.seconds), (1.0*rec.switches) / (1.0*g_options.seconds));
#endif
				sum.creations += rec.creations;
				sum.switches += rec.switches;
				//printf("parent read child %d\n", i);
			}
		}
		printf("For window [%d,%d) got %.1f creations per second and %.1f switches per second\n",
		       w*g_options.seconds, (w+1)*g_options.seconds - 1,
		       (1.0*sum.creations) / (1.0 *g_options.seconds), (1.0*sum.switches) / (1.0*g_options.seconds));
	}


	return 0;
}
