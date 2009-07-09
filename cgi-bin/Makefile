PREFIXPATH=/usr/local/mxscaleb/bin
CC=$(PREFIXPATH)/mxscaleb-gcc
STRIP=$(PREFIXPATH)/mxscaleb-strip

moxa:
	$(CC) -o control control.c
	$(STRIP) -s control

clean:
	rm -f control
