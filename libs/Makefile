include ../Makefile.inc

VPATH=.:src:include
DEPFILE = .depends

libsnap.a: src/snapper.o src/shared_malloc.o
	ar -rc libsnap.a src/snapper.o src/shared_malloc.o

depend:
	truncate -s 0 $(DEPFILE)
	makedepend -Y -f $(DEPFILE) -- -Iinclude -- src/*.c 

clean:
	rm -f *~ *.o **/*~ **/*.o *.a

.PHONY: clean depend

sinclude $(DEPFILE)
