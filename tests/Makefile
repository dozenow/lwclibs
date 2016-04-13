include ../Makefile.inc

VPATH=.:src:include
DEPFILE = .depends

CFLAGS += -I../libsnap/include
CXXFLAGS += -I../libsnap/include

LIBDIRS = -L../libsnap
LIBS = -lsnap

build-all : correct1 fork ssl_snap_server ssl_snap_client switcher switcher_pthread nonshared

ssl_snap_server: src/ssl_snap_server.o src/snap_ssl.o src/shared_malloc.o
	$(CXX) $(CXXFLAGS) $> -o $@ -lssl -lcrypto $(LIBDIRS) $(LIBS)

ssl_snap_client: src/ssl_snap_client.o src/snap_ssl.o src/shared_malloc.o 
	$(CXX) $(CXXFLAGS) $> -o $@ -lssl -lcrypto

fork: src/fork.c
	$(CC) $(CFLAGS)  $(LIBDIRS) $(LIBS) $> -o $@

correct_shared : src/correct_shared.o
	$(CC) $(CFLAGS)  $(LIBDIRS) $(LIBS) $> -o $@

switcher_pthread : src/switcher_pthread.o
	$(CXX) $(CXXFLAGS)  $(LIBDIRS) $(LIBS) -lpthread $> -o $@

switcher : src/switcher.o
	$(CXX) $(CXXFLAGS)  $(LIBDIRS) $(LIBS) $> -o $@

nonshared : src/nonshared.c
	$(CC) $(CFLAGS)  $(LIBDIRS) $(LIBS) $> -o $@

correct1 : src/correct1.o
	$(CC) $(CFLAGS)  $(LIBDIRS) $(LIBS) $> -o $@

clean:
	rm -f src/*.o **/*~ correct1 fork ssl_snap_server ssl_snap_client correct_shared switcher switcher_pthread nonshared

depend:
	truncate -s 0 $(DEPFILE)
	makedepend -Y -f $(DEPFILE) -- -Iinclude -- src/*.c src/*.cpp


.PHONY: depend build-all

sinclude $(DEPFILE)
