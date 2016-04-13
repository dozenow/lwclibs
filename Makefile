
build-all:
	make -C libsnap build-all
	make -C tests build-all

clean:
	make -C tests clean

.PHONY: build-all clean
