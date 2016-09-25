#include <unistd.h>
#include <stdio.h>
#include <sys/procdesc.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
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
	unsigned int pages_to_dirty; 
	unsigned int groups;
	unsigned int groups_to_dirty;
	unsigned int iterations;
	FILE *fp;
	int discard;
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
	.pages_to_dirty = 0,
	.groups = 0,
	.groups_to_dirty = 0,
	.discard = 0,
	.iterations = 0,
	.fp = NULL,
};


int readopts(int argc, char *const argv[], struct opts *res) {
	const char optstring[] = "l:s:w:c:p:hdg:G:P:f:i:";
	const char usage[] ="options\n"
		"\t-c Number of children to fork\n"
		"\t-d Indicates that all switches should be discard switches\n"
		"\t-g Number of pages groups\n"
		"\t-G Number of groups to dirty (page count set by -P) \n"
		"\t-l Number of LWCS to have active in the lwc pool\n"
		"\t-p Number of pages per page group to\n"
		"\t-P Number of pages per group to dirty between creates (whole number) \n"
		"\t-s Number of switches to perform before an lwc must be replaced\n"
		"\t-w Number of seconds in the time window before outputting timing statistics\n"
		"\t-i Number of iterations (windows) before quitting\n"
		"\t-f Filename to write data out to\n"
		"\t-h Print usage message\n";

	int c;
	int rv = 0;
	while(!rv && ((c=getopt(argc, argv, optstring)) != -1)) {
		switch(c) {
		case 'f':
			res->fp = fopen(optarg, "a");
			if (res->fp == NULL) {
				fprintf(stderr, "Could not open file %s: %s\n", optarg, strerror(errno));
				rv = -1;
			} else {
				setbuf(res->fp, NULL);
				rv = 0;
			}
			break;
		case 'i':
			rv = getuint(optarg, &res->iterations);
			break;
		case 'P':
			rv = getuint(optarg, &res->pages_to_dirty);
			break;
		case 'G':
			rv = getuint(optarg, &res->groups_to_dirty);
			break;
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
		case 'd':
			res->discard = 1;
			break;
		default:
			rv = -1;
			break;
		}
	}

	if (res->pages_to_dirty > res->pages) {
		fprintf(stderr, "-P %u > -p %u inconsistent\n", res->pages_to_dirty, res->pages);
		rv = -1;
	}

	if (res->groups_to_dirty > res->groups) {
		fprintf(stderr, "-G %u > -g %u inconsistent\n", res->groups_to_dirty, res->groups);
		rv = -1;
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

char **g_bufs = NULL;


int do_child(int pipe, const struct opts *opts) {


	for(unsigned int g = 0; g < opts->groups; ++g) {
		bzero(g_bufs[g], getpagesize() * opts->pages);
	}


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

	unsigned int check_time = 0; /* check when mod some value */

	for(;;) {

		/* this is just to reduce measurement overhead on this. didn't want to mess with rtdsc */
		check_time = (check_time + 1) % 1;//503;
		if (check_time == 0) {
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
		}


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
			int new_snap = lwccreate(specs, 2, NULL, 0, 0, opts->discard ? 0: LWC_SUSPEND_ONLY);
			unsigned int sum = 0;

			if (new_snap >= 0) {


				//lwcclose(new_snap);
				//exit(0);
				int idx = 0;
				for(;snaplist[idx] > 0; ++idx);
				snaplist[idx] = new_snap;
				switchcnt[idx] = 0;
				snapcnt++;

				creations++;


			} else if (new_snap == LWC_SWITCHED) {


				for(;;) { /* child loop */
					if (opts->groups_to_dirty + opts->pages_to_dirty > 0) {
						for(unsigned int g = 0; g < opts->groups; ++g) {
							for(unsigned int p = 0; p < opts->pages; ++p) {
								if (g < opts->groups_to_dirty && p < opts->pages_to_dirty) {
									g_bufs[g][p*4096] = 1;
								} else {
									sum += g_bufs[g][p*4096];
								}
							}
						}
					}

					if (opts->discard && (lwcdiscardswitch(parent_snap, NULL, 0) == LWC_FAILED)) {
						printf("discardswitch in child failed: %s\n", strerror(errno));
						return EXIT_FAILURE;
					} else if (lwcsuspendswitch(parent_snap, NULL, 0, NULL, NULL, 0) == LWC_FAILED) {
						printf("suspendswitch in child failed: %s\n", strerror(errno));
						return EXIT_FAILURE;
					}
				}
			} else if (new_snap == LWC_FAILED) {
				//this is really just so sum doesn't get compiled away
				fprintf(stderr, "sum was %u\n", sum);

				perror("Can't create snap: ");
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
	int status;
	for(int remaining = g_options.children; remaining; remaining--) {
		pid_t p = wait(&status);
		for(unsigned int i = 0; i < g_options.children; ++i) {
			if (kids[i].pid == p) {
				kids[i].pid = -1;
				close(kids[i].pipe);
				fprintf(stderr, "child %d exited(%d) with status %d\n", p, WIFEXITED(status), WEXITSTATUS(status));
			}
		}
	}
}

int main(int argc, char * const argv[]) {


	g_options.fp = stderr;
	if (readopts(argc, argv, &g_options)) {
		return EXIT_FAILURE;
	}

	g_bufs = malloc((g_options.groups) * sizeof(*g_bufs));
	kids = malloc((g_options.children)* sizeof(*kids));

	for(unsigned int g = 0; g < g_options.groups; ++g) {
		char *mbuf = mmap(NULL, getpagesize() * g_options.pages, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
		if (mbuf == MAP_FAILED) {
			fprintf(stderr, "Can't mmap in group %d: %s\n", g, strerror(errno));
			return EXIT_FAILURE;
		}
		/* touch all the pages */
		bzero(mbuf, getpagesize() * g_options.pages);
		g_bufs[g] = mbuf;
	}

	for(unsigned int i = 0; i < g_options.children; ++i) {

		/* setup so return means in parent and success. */
		do_fork(&kids[i]);
	}

	for(unsigned w = 0; (g_options.iterations == 0 || w < g_options.iterations) ;++w) {

		struct record sum = {0,0};
		for(unsigned int i = 0; i < g_options.children; ++i) {
			if (kids[i].pid == -1) {
				do_fork(&kids[i]);
			} else {
				struct record rec;
				ssize_t rv = read(kids[i].pipe, &rec, sizeof(rec));
				if (rv < (ssize_t)sizeof(rec)) {
					fprintf(stderr, "got %zd of %lu bytes, errstr = %s\n", rv, sizeof(rec), strerror(errno));
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
		fprintf(g_options.fp, "For window [%d,%d) got %.1f creations per second and %.1f switches per second\n",
		        w*g_options.seconds, (w+1)*g_options.seconds - 1,
		        (1.0*sum.creations) / (1.0 *g_options.seconds), (1.0*sum.switches) / (1.0*g_options.seconds));
	}

	signal(SIGHUP, SIG_IGN);
	kill(0, SIGHUP);
	signal(SIGHUP, SIG_DFL);
	proc_exit();
	fclose(g_options.fp);
	return 0;
}
