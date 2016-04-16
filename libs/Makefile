include ../Makefile.inc

VPATH=.:src:include
DEPFILE = .depends

CFLAGS+=-fpic

all: libsnap.so libsnap.a

LIBSNAP_DEPS=src/snapper.o src/shared_malloc.o src/shared_ht.o src/shared_llist.o
libsnap.so: $(LIBSNAP_DEPS)
	$(CC) -shared $(LIBSNAP_DEPS) -o libsnap.so

libsnap.a: $(LIBSNAP_DEPS)
	ar -rc libsnap.a $(LIBSNAP_DEPS)

depend:
	truncate -s 0 $(DEPFILE)
	makedepend -Y -f $(DEPFILE) -- -Iinclude -- src/*.c 

clean:
	rm -f *~ *.o **/*~ **/*.o *.a *.so

.PHONY: all clean depend

sinclude $(DEPFILE)
