APACHECTL=/opt/apache2/bin/apachectl
APXS=/opt/apache2/bin/apxs
SNAPLIB=../../../tests/include/

.libs/forksnap.so: forksnap.c ../../../tests/src/snapper.c
	$(APXS) -i -I $(SNAPLIB)  -c forksnap.c ../../../tests/src/snapper.c

clean:
	rm -rf *.la *~ *.o *.slo .libs 

.PHONY: clean 
