include ../Makefile.inc

VPATH=.:src:include
DEPFILE = .depends
TARGETS=correct1 fork nonshared correct_shared overlay_mem overlay_file overlay_cred correct3 correct2 syscall close

STATICLWC=../libs/liblwc.a
LIBDIRS = -L../libs/
LIBS = $(STATICLWC)


build-all : ../libs/liblwc.a $(TARGETS)

overlay_mem : src/overlay_mem.o $(LIBS)
	$(CC) $(CFLAGS) src/overlay_mem.o $(LIBDIRS) -o $@ $(LIBS)

overlay_file : src/overlay_file.o $(LIBS)
	$(CC) $(CFLAGS) src/overlay_file.o $(LIBDIRS) -o $@ $(LIBS)

overlay_cred : src/overlay_cred.o $(LIBS)
	$(CC) $(CFLAGS) src/overlay_cred.o $(LIBDIRS) -o $@ $(LIBS)

close : src/close.o $(LIBS)
	$(CC) $(CFLAGS) src/close.o $(LIBDIRS) -o $@ $(LIBS)

syscall : src/syscall.o $(LIBS)
	$(CC) $(CFLAGS) src/syscall.o $(LIBDIRS) -o $@ $(LIBS)

correct3 : src/correct3.o $(LIBS)
	$(CC) $(CFLAGS) src/correct3.o $(LIBDIRS) -o $@ $(LIBS)

correct2 : src/correct2.o $(LIBS)
	$(CC) $(CFLAGS) src/correct2.o $(LIBDIRS) -o $@ $(LIBS)

$(LIBS) : ../libs/src/*.c ../libs/src/*.S
	$(MAKE) -C ../libs

SSS_DEPS=src/ssl_snap_server.o src/snap_ssl.o 
ssl_snap_server: $(SSS_DEPS) $(LIBS)
	$(CXX) $(CXXFLAGS) $(SSS_DEPS) -o $@ -lssl -lcrypto $(LIBDIRS) $(LIBS)

SSC_DEPS=src/ssl_snap_client.o src/snap_ssl.o 
ssl_snap_client: $(SSC_DEPS) $(LIBS)
	$(CXX) $(CXXFLAGS) $(SSC_DEPS) -o $@ -lssl -lcrypto $(LIBDIRS) $(LIBS)

fork: src/fork.c $(LIBS)
	$(CC) $(CFLAGS) src/fork.c $(LIBDIRS) $(LIBS) -o $@

correct_shared : src/correct_shared.o $(LIBS)
	$(CC) $(CFLAGS) src/correct_shared.o $(LIBDIRS) $(LIBS) -o $@

switcher_pthread : src/switcher_pthread.o $(LIBS)
	$(CXX) $(CXXFLAGS) src/switcher_pthread.o $(LIBDIRS) $(LIBS) -lpthread -o $@

switcher : src/switcher.o $(LIBS)
	$(CXX) $(CXXFLAGS) src/switcher.o $(LIBDIRS) $(LIBS) -o $@

nonshared : src/nonshared.c $(LIBS)
	$(CC) $(CFLAGS) src/nonshared.c $(LIBDIRS) $(LIBS) -o $@

correct1 : src/correct1.o $(LIBS)
	$(CC) $(CFLAGS) src/correct1.o $(LIBDIRS) $(LIBS) -o $@

clean:
	rm -f src/*.o **/*~ $(TARGETS) *.core

depend:
	truncate -s 0 $(DEPFILE)
	makedepend -Y -f $(DEPFILE) -- -Iinclude -I../libs/include -- src/*.c src/*.cpp


.PHONY: depend build-all

sinclude $(DEPFILE)
