include ../Makefile.inc

VPATH=.:src:include
DEPFILE = .depends


CLFAGS += -fPIC

build-all : libsnap.so

libsnap.so: src/snapper.o
	$(CC) -shared $(REQ_LIBS) -o $@ $^ #$>

clean:
	rm -f src/*.o **/*~ 

depend:
	truncate -s 0 $(DEPFILE)
	makedepend -Y -f $(DEPFILE) -- -Iinclude -- src/*.c src/*.cpp


.PHONY: depend build-all

sinclude $(DEPFILE)
