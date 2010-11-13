#! /bin/sh
#
# MOXA7GATE MODBUS GATEWAY SOFTWARE VERSION 1.2
# SEM-ENGINEERING, BRYANSK 2010
#
# THIS CONFIGURATION FILE GENERATED AUTOMATICALLY
# AT 14:24:39 13.11.2010
#
# Object: Тестовый стенд
# Location: Кроссовая (214) A4
#
# File Version: 1.13
#
# Network Name: moxa_test_a4
# Network Address: 10.0.6.37
#
 
/root/moxa7gate \
--tcp_port 502 \
--modbus_address 1 \
--status_info 1 \
--show_sys_messages \
--Object "Тестовый стенд" \
--Location "Кроссовая (214) A4" \
--confVersion "1.13" \
--NetworkName "moxa_test_a4" \
--NetworkAddress "10.0.6.37" \
PORT2 RS485_2W 9600 NONE 1000 GATEWAY_PROXY --desc "Тестовый шлейф из восьми пожарных датчиков" \
PORT8 RS485_2W 9600 NONE 1000 BRIDGE_PROXY --desc "Устройство modbus-мастер на стороне RTU" \
PROXY_TABLE 9 \
1 8 33 T1 1 HOLDING_REGISTER 100 2 --desc "Тестовый блок регистров" \
2 1 41 P2 1 HOLDING_REGISTER 0 2 --desc "ИПЭС №1" \
2 1 42 P2 3 HOLDING_REGISTER 0 2 --desc "ИПЭС №3" \
2 1 43 P2 5 HOLDING_REGISTER 0 2 --desc "ИПЭС №5" \
2 1 44 P2 7 HOLDING_REGISTER 0 2 --desc "ИПЭС №7" \
2 1 45 P2 9 HOLDING_REGISTER 0 2 --desc "ИПЭС №9" \
2 1 46 P2 11 HOLDING_REGISTER 0 2 --desc "ИПЭС №11" \
2 1 47 P2 13 HOLDING_REGISTER 0 2 --desc "ИПЭС №13" \
2 1 48 P2 15 HOLDING_REGISTER 0 2 --desc "ИПЭС №15" \
TCP_SERVERS 1 \
10.0.1.252:502 2 0 P1 --desc "MODSIM на локальном ПК" \
&
