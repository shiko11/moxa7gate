#! /bin/sh
#
# MOXA7GATE MODBUS GATEWAY SOFTWARE VERSION 1.1
# SEM-ENGINEERING, BRYANSK 2013
#
# THIS CONFIGURATION FILE GENERATED AUTOMATICALLY
# AT 14:33:28 26.08.2013
#
# Object: Тест UC-8410
# Location: ОРПО NA
#
# File Version: 1.4
#
# Network Name: Moxa
# Network Address: 192.168.3.127
#
 
/home/moxa7gate_8410 \
--tcp_port 502 \
--modbus_address 1 \
--status_info 1 \
--show_sys_messages \
--Object "Тест UC-8410" \
--Location "ОРПО NA" \
--confVersion "1.4" \
--NetworkName "Moxa" \
--NetworkAddress "192.168.3.127" \
PORT8 RS485_2W 9600 NONE 1000 GATEWAY_PROXY \
PROXY_TABLE 1 \
1 8 101 P8 1 HOLDING_REGISTER 0 2 \
&
