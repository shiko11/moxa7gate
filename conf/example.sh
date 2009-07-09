#! /bin/sh
#
# Description:	
#
# Version:	
# 
# MOXA:		
#  
# USO:		
#
./moxa7gate \
PORT1 RS485_2w 9600 none 1400000 GATEWAY 502 \
PORT3 RS232 	 9600 none 1600000 BRIDGE \
	10.0.6.240:502 1 --address_shift 2 \
	192.0.0.252:502 2 \
	127.0.0.1:6502 3 \
PORT4 RS485_2w 9600 none 1600000 MASTER  6502 1 \
	1 0x01 1 2 1 \ modbus exception response
	1 0x02 3 2 3 \ write function can be realized too
	1 0x03 6 3 6 \
	1 0x04 9 3 9 \
PORT6 RS232 	 9600 none 1600000 GATEWAY 5502 \
PORT7 RS485_2w 9600 none 1600000 GATEWAY 6502 \
PORT8 RS485_2w 9600 none 2000000 GATEWAY 7502 \
&