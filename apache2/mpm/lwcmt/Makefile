APACHECTL=/opt/apache2/bin/apachectl
APXS=/opt/apache2/bin/apxs
LWCDIR=/home/litton/snap
LWCLIB=$(LWCDIR)/libs/include/


.libs/lwcmt.so: lwcmt.c $(LWCDIR)/libs/liblwc.a
	$(APXS) -n mpm_lwcmt_module -i -I $(LWCLIB)  -c $(LWCDIR)/libs/liblwc.a lwcmt.c fdqueue.c

clean:
	rm -rf *.la *~ *.o *.slo .libs 

.PHONY: clean 
