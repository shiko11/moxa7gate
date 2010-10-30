#! /bin/sh
#
# MOXA7GATE MODBUS GATEWAY SOFTWARE VERSION 1.1
# SEM-ENGINEERING, BRYANSK 2010
#
# THIS CONFIGURATION FILE GENERATED AUTOMATICALLY
# AT 15:46:30 12.10.2010
#
# Object: ЛПДС Кротовка
# Location: УСО ЦПУ-П A3
#
# File Version: 1.1
#
# Network Name: moxa_cpup_a3
# Network Address: 192.0.0.15
#
 
./moxa7gate \
--tcp_port 502 \
--modbus_address 1 \
--status_info 1 \
--Object "ЛПДС Кротовка" \
--Location "УСО ЦПУ-П A3" \
--confVersion "1.1" \
--NetworkName "moxa_cpup_a3" \
--NetworkAddress "192.0.0.15" \
PORT1 RS485_2W 2400 NONE 1000 BRIDGE_SIMPLE --desc "Панель сигнализации" \
TCP_SERVERS 1 \
192.0.0.252:502 1 0 P1 --desc "QUANTUM CPU 671 60" \
&
