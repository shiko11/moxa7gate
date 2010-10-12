#! /bin/sh
#
# MOXA7GATE MODBUS GATEWAY SOFTWARE VERSION 1.1
# SEM-ENGINEERING, BRYANSK 2010
#
# THIS CONFIGURATION FILE GENERATED AUTOMATICALLY
# AT 9:06:38 12.10.2010
#
# Object: ЛПДС Кротовка
# Location: УСО ЦПУ-П A2
#
# File Version: 1.1
#
# Network Name: moxa_cpup_a2
# Network Address: 192.0.0.14
#
 
./moxa7gate_plus \
--tcp_port 502 \
--modbus_address 247 \
--show_sys_messages \
--status_info 30 \
--Object "ЛПДС Кротовка" \
--Location "УСО ЦПУ-П A2" \
--confVersion "1.1" \
--NetworkName "moxa_cpup_a2" \
--NetworkAddress "192.0.0.14" \
PORT1 RS485_2W 9600 NONE 200 GATEWAY_PROXY --desc "Пожарный шлейф 6 датчиков" \
PORT2 RS485_2W 9600 NONE 200 GATEWAY_PROXY --desc "Пожарный шлейф 8 датчиков" \
PORT3 RS485_2W 9600 NONE 200 GATEWAY_PROXY --desc "Пожарный шлейф 8 датчиков" \
PORT4 RS485_2W 9600 NONE 200 GATEWAY_PROXY --desc "Пожарный шлейф 7 датчиков" \
PROXY_TABLE 29 \
1 1 1 P1 1 HOLDING_REGISTER 0 4 --desc "Датчик №1" \
1 1 2 P1 2 HOLDING_REGISTER 0 4 --desc "Датчик №2" \
1 1 3 P1 3 HOLDING_REGISTER 0 4 --desc "Датчик №3" \
1 1 4 P1 4 HOLDING_REGISTER 0 4 --desc "Датчик №4" \
1 1 5 P1 5 HOLDING_REGISTER 0 4 --desc "Датчик №5" \
1 1 6 P1 6 HOLDING_REGISTER 0 4 --desc "Датчик №6" \
1 1 7 P2 1 HOLDING_REGISTER 0 4 --desc "Датчик №1" \
1 1 8 P2 2 HOLDING_REGISTER 0 4 --desc "Датчик №2" \
1 1 9 P2 3 HOLDING_REGISTER 0 4 --desc "Датчик №3" \
1 1 10 P2 4 HOLDING_REGISTER 0 4 --desc "Датчик №4" \
1 1 11 P2 5 HOLDING_REGISTER 0 4 --desc "Датчик №5" \
1 1 12 P2 6 HOLDING_REGISTER 0 4 --desc "Датчик №6" \
1 1 13 P2 7 HOLDING_REGISTER 0 4 --desc "Датчик №7" \
1 1 14 P2 8 HOLDING_REGISTER 0 4 --desc "Датчик №8" \
1 1 15 P3 1 HOLDING_REGISTER 0 4 --desc "Датчик №1" \
1 1 16 P3 2 HOLDING_REGISTER 0 4 --desc "Датчик №2" \
1 1 17 P3 3 HOLDING_REGISTER 0 4 --desc "Датчик №3" \
1 1 18 P3 4 HOLDING_REGISTER 0 4 --desc "Датчик №4" \
1 1 19 P3 5 HOLDING_REGISTER 0 4 --desc "Датчик №5" \
1 1 20 P3 6 HOLDING_REGISTER 0 4 --desc "Датчик №6" \
1 1 21 P3 7 HOLDING_REGISTER 0 4 --desc "Датчик №7" \
1 1 22 P3 8 HOLDING_REGISTER 0 4 --desc "Датчик №8" \
1 1 23 P4 1 HOLDING_REGISTER 0 4 --desc "Датчик №1" \
1 1 24 P4 2 HOLDING_REGISTER 0 4 --desc "Датчик №2" \
1 1 25 P4 3 HOLDING_REGISTER 0 4 --desc "Датчик №3" \
1 1 26 P4 4 HOLDING_REGISTER 0 4 --desc "Датчик №4" \
1 1 27 P4 5 HOLDING_REGISTER 0 4 --desc "Датчик №5" \
1 1 28 P4 6 HOLDING_REGISTER 0 4 --desc "Датчик №6" \
1 1 29 P4 7 HOLDING_REGISTER 0 4 --desc "Датчик №7" \
&
