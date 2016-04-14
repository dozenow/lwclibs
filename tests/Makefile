include ../Makefile.inc

VPATH=.:src:include
DEPFILE = .depends
TARGETS=correct1 fork ssl_snap_server ssl_snap_client switcher switcher_pthread nonshared

LIBDIRS = -L../libs/
LIBS = -lsnap


build-all : $(TARGETS)

../libs/libsnap.a: ../libs/src/*.c ../libs/include/*.h
	$(MAKE) -C ../libs

ssl_snap_server: src/ssl_snap_server.o src/snap_ssl.o 
	$(CXX) $(CXXFLAGS) $> -o $@ -lssl -lcrypto $(LIBDIRS) $(LIBS)

ssl_snap_client: src/ssl_snap_client.o src/snap_ssl.o 
	$(CXX) $(CXXFLAGS) $> -o $@ -lssl -lcrypto $(LIBDIRS) $(LIBS)

fork: src/fork.c
	$(CC) $(CFLAGS)  $(LIBDIRS) $(LIBS) $> -o $@

correct_shared : src/correct_shared.o
	$(CC) $(CFLAGS)  $(LIBDIRS) $(LIBS) $> -o $@

switcher_pthread : src/switcher_pthread.o
	$(CXX) $(CXXFLAGS)  $(LIBDIRS) $(LIBS) -lpthread $> -o $@

switcher : src/switcher.o
	$(CXX) $(CXXFLAGS) $> $(LIBDIRS) $(LIBS) -o $@

nonshared : src/nonshared.c
	$(CC) $(CFLAGS) $> $(LIBDIRS) $(LIBS) -o $@

correct1 : src/correct1.o
	$(CC) $(CFLAGS) $> $(LIBDIRS) $(LIBS)  -o $@

clean:
	rm -f src/*.o **/*~ $(TARGETS) *.core

depend:
	truncate -s 0 $(DEPFILE)
	makedepend -Y -f $(DEPFILE) -- -Iinclude -I../libs/include -- src/*.c src/*.cpp


.PHONY: depend build-all

sinclude $(DEPFILE)
