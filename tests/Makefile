include ../Makefile.inc

VPATH=.:src:include
DEPFILE = .depends
TARGETS=correct1 fork ssl_snap_server ssl_snap_client switcher switcher_pthread nonshared

STATICSNAP=../libs/libsnap.a
LIBDIRS = -L../libs/
LIBS = $(STATICSNAP)


build-all : ../libs/libsnap.a $(TARGETS)

../libs/libsnap.a:
	$(MAKE) -C ../libs


SSS_DEPS=src/ssl_snap_server.o src/snap_ssl.o 
ssl_snap_server: $(SSS_DEPS)
	$(CXX) $(CXXFLAGS) $(SSS_DEPS) -o $@ -lssl -lcrypto $(LIBDIRS) $(LIBS)

SSC_DEPS=src/ssl_snap_client.o src/snap_ssl.o 
ssl_snap_client: $(SSC_DEPS)
	$(CXX) $(CXXFLAGS) $(SSC_DEPS) -o $@ -lssl -lcrypto $(LIBDIRS) $(LIBS)

fork: src/fork.c
	$(CC) $(CFLAGS) src/fork.c $(LIBDIRS) $(LIBS) -o $@

correct_shared : src/correct_shared.o
	$(CC) $(CFLAGS) src/correct_shared.o $(LIBDIRS) $(LIBS) -o $@

switcher_pthread : src/switcher_pthread.o
	$(CXX) $(CXXFLAGS) src/switcher_pthread.o $(LIBDIRS) $(LIBS) -lpthread -o $@

switcher : src/switcher.o
	$(CXX) $(CXXFLAGS) src/switcher.o $(LIBDIRS) $(LIBS) -o $@

nonshared : src/nonshared.c
	$(CC) $(CFLAGS) src/nonshared.c $(LIBDIRS) $(LIBS) -o $@

correct1 : src/correct1.o
	$(CC) $(CFLAGS) src/correct1.o $(LIBDIRS) $(LIBS) -o $@

clean:
	rm -f src/*.o **/*~ $(TARGETS) *.core

depend:
	truncate -s 0 $(DEPFILE)
	makedepend -Y -f $(DEPFILE) -- -Iinclude -I../libs/include -- src/*.c src/*.cpp


.PHONY: depend build-all

sinclude $(DEPFILE)
