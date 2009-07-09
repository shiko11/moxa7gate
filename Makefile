PREFIXPATH=/usr/local/mxscaleb/bin
CC=$(PREFIXPATH)/mxscaleb-gcc
STRIP=$(PREFIXPATH)/mxscaleb-strip

moxa:
	$(CC) -o moxa7gate \
main.c global.c modbus_rtu.c modbus_tcp.c mx_keypad_lcm.c mxshm.c \
mxlib/mxkeypad_lx.c mxlib/mxlcm_lx.c mxlib/mxwdg_lx.c mxlib/mxbuzzer_lx.c \
-lpthread
	$(STRIP) -s moxa7gate

clean:
	rm -f moxa7gate
