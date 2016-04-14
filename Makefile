
build-all:
#	$(MAKE) -C libref libref not committed
	$(MAKE) -C libs 
	$(MAKE) -C tests build-all

clean:
	rm -f *~ *.core
	$(MAKE) -C libs clean
	$(MAKE) -C tests clean


.PHONY: build-all clean
