#! /bin/sh
#
# MOXA7GATE MODBUS GATEWAY SOFTWARE VERSION 1.1
# SEM-ENGINEERING, BRYANSK 2011
#
# THIS CONFIGURATION FILE GENERATED AUTOMATICALLY
# AT 10:15:53 28.11.2011
#
# Object: моя -3
# Location: сян жо A6 A10
#
# File Version: 1.1
#
# Network Name: moxa_a6
# Network Address: 192.0.0.13
#
 
/root/moxa7gate \
--tcp_port 502 \
--modbus_address 1 \
--status_info 2000 \
--show_sys_messages \
--watchdog_timer \
--Object "моя -3" \
--Location "сян жо A6 A10" \
--confVersion "1.1" \
--NetworkName "moxa_a6" \
--NetworkAddress "192.0.0.13" \
PORT1 RS485_2W 9600 NONE 1000 GATEWAY_PROXY --desc "PLC Schneider Momentum" \
PROXY_TABLE 2 \
1 28 11 P1 11 HOLDING_REGISTER 0 2 \
1 28 39 P1 12 HOLDING_REGISTER 0 2 \
&
