#! /bin/sh
#
# MOXA7GATE MODBUS GATEWAY SOFTWARE VERSION 1.1
# SEM-ENGINEERING, BRYANSK 2013
#
# THIS CONFIGURATION FILE GENERATED AUTOMATICALLY
# AT 10:29:06 09.12.2013
#
# Object: Линейная телемеханика
# Location: Шкаф ЛТМ №1 A10
#
# File Version: 1.2
#
# Network Name: EM1240
# Network Address: 192.168.5.125
#
 
/root/moxa7gate \
--tcp_port 502 \
--modbus_address 1 \
--status_info 1 \
--show_data_flow \
--show_sys_messages \
--Object "Линейная телемеханика" \
--Location "Шкаф ЛТМ №1 A10" \
--confVersion "1.2" \
--NetworkName "EM1240" \
--NetworkAddress "192.168.5.125" \
PORT1 RS232 9600 NONE 1000 BRIDGE_PROXY --desc "MODSCAN PC" \
PORT2 RS232 9600 NONE 1000 GATEWAY_PROXY --desc "MODSIM PC" \
PROXY_TABLE 1 \
1 4 101 P2 2 HOLDING_REGISTER 500 0 --desc "MODSIM PC" \
&
