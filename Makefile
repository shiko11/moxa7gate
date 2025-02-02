# Defined for UC-7420/7410/7408/7402-LX/DA-660-8/16-LX
# PLATFORM = UC7400_AND_DA660

# Defined for UC-7420/7410/7408/7402-LX Plus and /DA-661/662/663-16-LX
#PLATFORM = UC7400PLUS_AND_DA66X

# Defined for UC-8410/8416/8418-LX
# PLATFORM = UC8400

ifeq ($(PLATFORM), UC7400_AND_DA660)
  PREFIX=mxscaleb-
else
  ifeq ($(PLATFORM),UC7400PLUS_AND_DA66X)
    PREFIX=xscale_be-
  else
    ifeq ($(PLATFORM),UC8400)
      PREFIX=xscale-linux-
    else
      PREFIX=
    endif
  endif
endif  

CC=$(PREFIX)gcc
STRIP=$(PREFIX)strip
NAME=moxa7gate_i386

all: release

clean:
	rm -f $(NAME)

release:
	$(CC) -o $(NAME) \
-DMOXA7GATE_WITHOUT_HMI_KLB \
-DARCHITECTURE_I386 \
main.c cli.c moxagate.c modbus_rtu.c modbus_tcp.c forwarding.c clients.c \
iface_tcpserver.c iface_rtumaster.c iface_rtuslave.c iface_tcpmaster.c \
iface_tcpslave.c interfaces.c hmi_web.c messages.c statistics.c \
custom.c \
-lpthread
#	$(STRIP) -s $(NAME)

