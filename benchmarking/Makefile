include ../Makefile.inc

VPATH=.:src:include
DEPFILE = .depends
TARGETS=switcher switcher_pthread switcher_pth switcher_ipc switcher_pthr_ipc

STATICLWC=../libs/liblwc.a
LIBDIRS = -L../libs/ 
LIBS = $(STATICLWC)
CXXFLAGS += -I/usr/local/include

build-all : $(LIBS) $(TARGETS)

$(LIBS) : ../libs/src/*.c ../libs/src/*.S
	$(MAKE) -C ../libs

switcher_ipc : src/switcher_ipc.o src/bench.o
	$(CXX) $(CXXFLAGS) src/switcher_ipc.o src/bench.o $(LIBDIRS) -o $@

switcher_pthr_ipc : src/switcher_pthr_ipc.o src/bench.o
	$(CXX) $(CXXFLAGS) src/switcher_pthr_ipc.o src/bench.o $(LIBDIRS) -lpthread -o $@

switcher_pthread : src/switcher_pthread.o src/bench.o
	$(CXX) $(CXXFLAGS) src/switcher_pthread.o src/bench.o $(LIBDIRS) -lpthread -o $@

switcher_pth : src/switcher_pth.o src/bench.o
	$(CXX) $(CXXFLAGS) src/switcher_pth.o src/bench.o -L/usr/local/lib/pth $(LIBDIRS) -o $@ -lpth

switcher : src/switcher.o src/bench.o $(LIBS)
	$(CXX) $(CXXFLAGS) src/switcher.o src/bench.o $(LIBDIRS) $(LIBS) -o $@

clean:
	rm -f src/*.o **/*~ $(TARGETS) *.core

depend:
	truncate -s 0 $(DEPFILE)
	makedepend -Y -f $(DEPFILE) -- -Iinclude -I../libs/include -- src/*.c src/*.cpp


.PHONY: depend build-all

sinclude $(DEPFILE)
