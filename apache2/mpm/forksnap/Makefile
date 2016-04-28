APACHECTL=/opt/apache2/bin/apachectl
APXS=/opt/apache2/bin/apxs
LWCDIR=/home/litton/snap
LWCLIB=$(LWCDIR)/libs/include/


.libs/forksnap.so: forksnap.c $(LWCDIR)/libs/liblwc.a
	$(APXS) -i -I $(LWCLIB)  -c forksnap.c $(LWCDIR)/libs/liblwc.a

clean:
	rm -rf *.la *~ *.o *.slo .libs 

.PHONY: clean 
