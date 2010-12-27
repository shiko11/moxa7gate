#! /bin/sh
#
# MOXA7GATE MODBUS GATEWAY SOFTWARE VERSION 1.1
# SEM-ENGINEERING, BRYANSK 2010
#
# THIS CONFIGURATION FILE GENERATED AUTOMATICALLY
# AT 10:29:40 28.12.2010
#
# Object: Нефтебаза СМНП Козьмино
# Location: Кроссовая 214 
#
# File Version: 1.16
#
# Network Name: moxa_test
# Network Address: 10.0.6.88
#
 
/root/moxa7gate \
--tcp_port 502 \
--modbus_address 1 \
--status_info 1 \
--Object "Нефтебаза СМНП Козьмино" \
--Location "Кроссовая 214 " \
--confVersion "1.16" \
--NetworkName "moxa_test" \
--NetworkAddress "10.0.6.88" \
--exception 2 4 \
PORT3 RS485_2W 9600 NONE 200 GATEWAY_PROXY --desc "Тестовый шлейф" \
PROXY_TABLE 2 \
2 1 33 P3 8 HOLDING_REGISTER 0 2 --desc "ИПЭС №8" \
2 1 34 P3 10 HOLDING_REGISTER 0 2 --desc "ИПЭС №10" \
&
