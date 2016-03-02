include ../Makefile.inc

VPATH=.:src:include
SRCS = src/correct1.c src/snapper.c src/fork.c
DEPFILE = .depends

build-all : correct1 fork

fork: src/snapper.o src/fork.c
	$(CC) $(CFLAGS) $> -o $@

correct1 : src/snapper.o src/correct1.o
	$(CC) $(CFLAGS) $> -o $@

clean:
	rm -f src/*.o **/*~ fork correct1

depend:
	truncate -s 0 $(DEPFILE)
	makedepend -Y -f $(DEPFILE) -- $(CFLAGS) -- $(SRCS)


.PHONY: depend build-all

sinclude $(DEPFILE)
