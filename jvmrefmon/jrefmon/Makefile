
SOURCES=lwcni
OBJECTS=$(SOURCES:=.o)
HEADERS=$(SOURCES:=.h)
INCLUDE_PATHS=-I../../libs/include/ -I/usr/lib/jvm/java-8-oracle/include/ -I/usr/lib/jvm/java-8-oracle/include/linux 
LIB_PATHS=
LIBS= -lpthread
ARCHIVE=../../libs/liblwc.a
CFLAGS=-Wall -g -D_GNU_SOURCE -fpic -Wno-unused-variable -Wno-unused-but-set-variable #-DLINUX_BUILD
CC=gcc $(CFLAGS) $(INCLUDE_PATHS)

all: $(OBJECTS) libtestiojni.so ctest loader.so

libtestiojni.so: $(OBJECTS)
	$(CC) -shared -fpic -o $@ $(OBJECTS) $(ARCHIVE) $(LIB_PATHS) $(LIBS)

ctest: $(OBJECTS) ctest.c
	$(CC) ctest.c $(OBJECTS) $(ARCHIVE) $(LIB_PATHS) $(LIBS) -o ctest

loader.so: $(OBJECTS) loader.o
	$(CC) -shared -fpic -o $@ $(OBJECTS) loader.o $(ARCHIVE) $(LIB_PATHS) $(LIBS)

clean:
	rm -f *.o *.d
	rm -f libtestiojni.so ctest loader.so
