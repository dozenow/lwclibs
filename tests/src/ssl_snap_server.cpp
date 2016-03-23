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

#include "netwrap.hpp"
#include "snapper.h"
#include "shared_malloc.h"

using namespace std;

using namespace snapper;

template <typename T>
inline T* MCHK(void *x) {
	if (x) {
		return (T*)x;
	}
	abort();
}

int work_function(int child_id, int counter, ssl::dumb_cxn *ssock) {
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

	struct sockaddr_un addr;
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, "/tmp/foobar");
	unlink("/tmp/foobar");

	int mom = Socket(AF_UNIX, SOCK_STREAM, 0);

	Bind(mom, (struct sockaddr *)&addr, strlen(addr.sun_path) + sizeof(addr.sun_family) + 1);
	Listen(mom, 5);


	snap_id_t *snaps = MCHK<snap_id_t>(sh_malloc(sizeof(snaps[0]) * 6));


	snap_id_t ns = Snap(SNAP_TARGET_NOJUMP, NULL, SNAP_VM|SNAP_FD);

	if (ns > 0) {
		snaps[0] = ns;
	} else { // jumped here.
		// do work and then jump to first child.
		cout << "I am the parent, pretend that I've done something\n";
		sleep(1);
		snap(snaps[1], NULL, 0);
	}

	ssl::server s;

	for(int child_id = 1; child_id <= 5; ++child_id) {
		int fd = Accept(mom, NULL, 0);

		//unique_ptr<ssl::cxn> ssock(s.accept(fd));
		unique_ptr<ssl::dumb_cxn> ssock(new ssl::dumb_cxn(fd));

		if (ssock) {
			//if (fd >= 0) {
			

			int *counter = MCHK<int>(sh_malloc(sizeof(*counter)));
			*counter = 0;
			snap_id_t ns = Snap(SNAP_TARGET_NOJUMP, NULL, SNAP_VM|SNAP_FD);

			if (ns > 0) {
				snaps[child_id] = ns;;
			} else { //jumped here
				// do work and then jump to next child or parent, round robin

				if (work_function(child_id, (*counter)++, ssock.get()) != 0) {
					cerr << "work function requested abort\n";
					exit(1);
				}

				snap(snaps[(child_id + 1) % 5], NULL, 0);
			}
		} else {
			cerr << "failure making sock\n";
			exit(1);
		}
	}

	Snap(snaps[0], NULL, 0);

}

