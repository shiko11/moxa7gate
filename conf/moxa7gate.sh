#! /bin/sh
#
# MOXA7GATE MODBUS GATEWAY SOFTWARE VERSION 1.1
# SEM-ENGINEERING, BRYANSK 2011
#
# THIS CONFIGURATION FILE GENERATED AUTOMATICALLY
# AT 11:40:11 13.04.2011
#
# Object: Линейная телемеханика
# Location: Шкаф ЛТМ №1 A10
#
# File Version: 1.1
#
# Network Name: UNO2171
# Network Address: 10.0.3.69
#
 
./moxa7gate_i386 \
--tcp_port 9502 \
--modbus_address 1 \
--status_info 1 \
--show_sys_messages \
--Object "Линейная телемеханика" \
--Location "Шкаф ЛТМ №1 A10" \
--confVersion "1.1" \
--NetworkName "UNO2171" \
--NetworkAddress "10.0.3.69" \
PORT1 RS232 19200 NONE 1000 GATEWAY_SIMPLE 502 --desc "PLC Schneider Momentum" \
&
