APACHECTL=/opt/apache2/bin/apachectl
APXS=/opt/apache2/bin/apxs
SNAPDIR=/home/litton/snap
SNAPLIB=$(SNAPDIR)/libs/include/


.libs/forksnap.so: forksnap.c $(SNAPDIR)/libs/libsnap.a
	$(APXS) -i -I $(SNAPLIB)  -c forksnap.c $(SNAPDIR)/libs/libsnap.a

clean:
	rm -rf *.la *~ *.o *.slo .libs 

.PHONY: clean 
