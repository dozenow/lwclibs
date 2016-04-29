include ../Makefile.inc

VPATH=.:src:include
DEPFILE = .depends

CFLAGS+=-fPIC

all: liblwc.a

src/lwc_asm.o : src/lwc.S
	$(CC) -c src/lwc.S -o src/lwc_asm.o

# Note that if you want to use the shared library, you still need to
# link in src/lwc_asm.o. It is already included in the static library
LIBLWC_DEPS=src/lwc.o src/shared_malloc.o src/shared_ht.o src/shared_llist.o
liblwc.so: $(LIBLWC_DEPS) src/lwc_asm.o
	$(CC) -fPIC -shared $(LIBLWC_DEPS) src/lwc_asm.o -o liblwc.so

liblwc.a: $(LIBLWC_DEPS) src/lwc_asm.o
	ar -rc liblwc.a $(LIBLWC_DEPS) src/lwc_asm.o

depend:
	truncate -s 0 $(DEPFILE)
	makedepend -Y -f $(DEPFILE) -- -Iinclude -- src/*.c 

clean:
	rm -f *~ *.o **/*~ **/*.o *.a *.so

.PHONY: all clean depend

sinclude $(DEPFILE)
