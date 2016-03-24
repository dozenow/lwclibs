include ../Makefile.inc

VPATH=.:src:include
DEPFILE = .depends


build-all : correct1 fork ssl_snap_server ssl_snap_client switcher switcher_pthread

ssl_snap_server: src/ssl_snap_server.o src/snap_ssl.o src/shared_malloc.o src/snapper.o
	$(CXX) $(CXXFLAGS) $> -o $@ -lssl -lcrypto

ssl_snap_client: src/ssl_snap_client.o src/snap_ssl.o src/shared_malloc.o 
	$(CXX) $(CXXFLAGS) $> -o $@ -lssl -lcrypto  

fork: src/snapper.o src/fork.c
	$(CC) $(CFLAGS) $> -o $@

correct_shared : src/snapper.o src/correct_shared.o
	$(CC) $(CFLAGS) $> -o $@

switcher_pthread : src/snapper.o src/switcher_pthread.o
	$(CXX) $(CXXFLAGS) -lpthread $> -o $@

switcher : src/snapper.o src/switcher.o
	$(CXX) $(CXXFLAGS) $> -o $@

correct1 : src/snapper.o src/correct1.o
	$(CC) $(CFLAGS) $> -o $@

clean:
	rm -f src/*.o **/*~ correct1 fork ssl_snap_server ssl_snap_client correct_shared switcher switcher_pthread

depend:
	truncate -s 0 $(DEPFILE)
	makedepend -Y -f $(DEPFILE) -- $(CFLAGS) -- $(SRCS)


.PHONY: depend build-all

sinclude $(DEPFILE)
