
SOURCES=IOTest
OBJECTS=$(SOURCES:=.o)
HEADERS=$(SOURCES:=.h)
INCLUDE_PATHS=-I../../libs/include/ -I/usr/lib/jvm/java-8-oracle/include/ -I/usr/lib/jvm/java-8-oracle/include/linux 
LIB_PATHS=
LIBS=
ARCHIVE=../../libs/liblwc.a
CFLAGS=-Wall -g -D_GNU_SOURCE -fpic -Wno-unused-variable -Wno-unused-but-set-variable
CC=gcc $(CFLAGS) $(INCLUDE_PATHS)

all: $(OBJECTS) libtestiojni.so

libtestiojni.so: $(OBJECTS)
	$(CC) -shared -fpic -o $@ $(OBJECTS) $(ARCHIVE) $(LIB_PATHS) $(LIBS)

clean:
	rm -f *.o *.d
	rm -f libtestiojni.so
