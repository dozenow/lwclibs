#include "snap_ssl.hxx"

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
#include <sys/un.h>

#include <iostream>
#include <map>

#include "netwrap.hpp"
#define SNAP_DIAGNOSTIC
#include "snapper.h"
#include "shared_malloc.h"

using namespace std;

using namespace snapper;

int work_function(int child_id, int counter, ssl::cxn *ssock) {
	static char  buf[32];
	sprintf(buf, "server send %d for client %d", counter, child_id);
	cout << "(s) " << buf << endl;
	if (ssock->write((uint8_t*)buf, 32) <= 0) {
		cerr << "(s) bad write\n";
		return 1;
	}
	sprintf(buf, "rewritten");
	if (ssock->read((uint8_t*)buf, 32) <= 0) {
		cerr << "(s) bad read\n";
		return 1;
	}
	cout << "(s) " << buf << endl;
	return 0;
}


int main(int argc, char *argv[]) {


	int *snaps = (int*) mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);

	int *shared_buf = (int*) mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);

#define NUM_SNAPS 0
#define CUR_SNAP 1

	shared_buf[NUM_SNAPS] = 1;
	shared_buf[CUR_SNAP] = 0;

	int src;
	cout << "calling 0\n";
	int ns = Snap(SNAP_TARGET_NOJUMP, &src, SNAP_SHARE_FD);
	cout << "snap 0 has ns " << ns << " and src " << src << endl;
	if (ns >= 0) {
		snaps[0] = ns;
	} else if (ns == SNAP_JUMPED) {
		// This is the "registration" snap

		// see if we've seen this one yet
		if (src >= 0) {
			size_t idx;
			for(idx = 0; idx < 4096 / sizeof(snaps[0]) && idx < shared_buf[NUM_SNAPS]; ++idx) {
				cout << "snaps[" << idx << "]=" << snaps[idx] << endl;
				if (src == snaps[idx])
					break;
			}
			if (idx < shared_buf[NUM_SNAPS]) { // we've seen you before
				for(;;) {
					cout << "I am the parent, pretend that I've done something\n";
					usleep(500);
					shared_buf[CUR_SNAP] = (shared_buf[CUR_SNAP] + 1) % shared_buf[NUM_SNAPS];
					shared_buf[CUR_SNAP] = shared_buf[CUR_SNAP] ? shared_buf[CUR_SNAP] : 1; // don't jump to self
					cout << "calling 2\n";
					int fd = snaps[ shared_buf[CUR_SNAP] ];

					munmap(shared_buf, 4096); /* not for you my pretties */
					munmap(snaps, 4096);

					int ns = Snap(fd, NULL, SNAP_NOTHING);
					cout << "snap 2 has ns " << ns << endl;
					cerr << "got here\n";
					exit(1);
				}				
			} else { // you are new here, register and jump back

				cout << "Do not know src = " << src << endl;
				snaps[ shared_buf[NUM_SNAPS]++ ] = src;
				cout << "calling 1\n";
				
				munmap(shared_buf, 4096); /* not for you my pretties */
				munmap(snaps, 4096);

				int ns = Snap(src, NULL, SNAP_NOTHING);
				cout << "snap 1 has ns " << ns << endl;
				cerr << "should not get here\n";
				exit(1);

			}
		} else {
			cerr << "got bad source, src= " << src << endl;
			exit(1);
		}
		
	}


	// now entering child context

	struct sockaddr_un addr;
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, "/tmp/foobar");
	unlink("/tmp/foobar");

	int mom = Socket(AF_UNIX, SOCK_STREAM, 0);

	Bind(mom, (struct sockaddr *)&addr, strlen(addr.sun_path) + sizeof(addr.sun_family) + 1);
	Listen(mom, 5);


	int *accepted = (int*) mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);	
	// okay, updateable context now that prefix is done
	cout << "calling 3\n";

	ns = Snap(snaps[0], NULL, SNAP_ALL);


#if 1
#define MALLOC_AVAIL ((1<<20) * 1024)
	void *malloc_buf = mmap(NULL, MALLOC_AVAIL, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_SHARED, -1, 0);
	if (malloc_buf == MAP_FAILED) {
		perror("Can't mmap. So many tears\n");
		return 0;
	}

	sh_sbrk_init(malloc_buf, MALLOC_AVAIL); //for ssl shared malloc crap

	ssl::server s;
#endif




	cout << "snap 3 has ns " << ns << " accepted[0] = " << accepted[0] << endl;
	if (accepted[0] < 2) {

		int fd = Accept(mom, NULL, 0);
		unique_ptr<ssl::cxn> ssock(s.accept(fd));
		//unique_ptr<ssl::dumb_cxn> ssock(new ssl::dumb_cxn(fd));
		++accepted[0];

		//close(mom);
		int id = *accepted;
		munmap(accepted, 4096);
		accepted = NULL;

		cout << "calling 5\n";
		int ns = Snap(snaps[0], NULL, SNAP_SHARE_FD | SNAP_SHARE_VM); // actual entry point
		cout << "snap 5 has ns " << ns << endl;

		for(;;) {
			if (work_function(id, snaps[100]++, ssock.get()) != 0) {
				cerr << "work function requested abort\n";
				exit(1);
			}
			cout << "calling 6\n";
			int ns = Snap(snaps[0], NULL, SNAP_NOTHING); //yield to parent
			cout << "snap 6 has ns " << ns << endl;
		}
	} else {
		cout << "calling 7\n";
		int ns = Snap(snaps[0], NULL, SNAP_NOTHING); //yield to parent
		cout << "snap 7 has ns " << ns << endl;
	}

	cerr << "at program end" << endl;
	return EXIT_FAILURE;

}

