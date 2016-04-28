include ../Makefile.inc

VPATH=.:src:include
DEPFILE = .depends

CFLAGS+=-fpic

all: liblwc.so liblwc.a

src/lwc_asm.o : src/lwc.S
	$(CC) -c src/lwc.S -o src/lwc_asm.o

LIBLWC_DEPS=src/lwc.o src/shared_malloc.o src/shared_ht.o src/shared_llist.o src/lwc_asm.o
liblwc.so: $(LIBLWC_DEPS)
	$(CC) -shared $(LIBLWC_DEPS) -o liblwc.so

liblwc.a: $(LIBLWC_DEPS)
	ar -rc liblwc.a $(LIBLWC_DEPS)

depend:
	truncate -s 0 $(DEPFILE)
	makedepend -Y -f $(DEPFILE) -- -Iinclude -- src/*.c 

clean:
	rm -f *~ *.o **/*~ **/*.o *.a *.so

.PHONY: all clean depend

sinclude $(DEPFILE)
