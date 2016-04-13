include ../Makefile.inc

VPATH=.:src:include
DEPFILE = .depends

build-all : libsnap.so




clean:
	rm -f src/*.o **/*~ correct1 fork ssl_snap_server ssl_snap_client correct_shared switcher switcher_pthread nonshared

depend:
	truncate -s 0 $(DEPFILE)
	makedepend -Y -f $(DEPFILE) -- -Iinclude -- src/*.c src/*.cpp


.PHONY: depend build-all

sinclude $(DEPFILE)
