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

#include "shared_malloc.h"
#include "netwrap.hpp"
#include "snapper.h"



using namespace std;

using namespace snapper;


int main(int argc, char *argv[]) {

#if 1
#define MALLOC_AVAIL ((1<<20) * 128)
	void *malloc_buf = mmap(NULL, (1 << 20) * 128, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
	if (malloc_buf == MAP_FAILED) {
		perror("Can't mmap. So many tears\n");
		return 0;
	}

	// not that we use the shared stuff for the client ...
	sh_sbrk_init(malloc_buf, MALLOC_AVAIL); //for ssl shared malloc crap

#endif


	struct sockaddr_un addr;
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, "/tmp/foobar");

	int fd = Socket(AF_UNIX, SOCK_STREAM, 0);

	Connect(fd, (struct sockaddr *) &addr, strlen(addr.sun_path) + 1 + sizeof(addr.sun_family));
	

	ssl::client c;
	unique_ptr<ssl::cxn> ssock(c.connect(fd));
	//unique_ptr<ssl::dumb_cxn> ssock(new ssl::dumb_cxn(fd));
	if (ssock) {
		for(int i = 0;1;++i) {
			char buf[32];
			if (ssock->read((uint8_t*)buf, 32) <= 0) {
				cerr << "(c) bad read\n";
				return 1;
			}
			cout << "(c) " << buf << endl;
			sprintf(buf, "client send %d", i);
			cout << "(c) " << buf << endl;
			if (ssock->write((uint8_t*)buf, 32) <= 0) {
				cerr << "(c) bad write\n";
				return 1;
			}
		}
	}

	

	return 0;
}
