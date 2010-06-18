#! /bin/sh
#
# MOXA7GATE MODBUS GATEWAY SOFTWARE VERSION 1.1
# THIS CONFIGURATION FILE GENERATED AUTOMATICALLY
# AT 11:01:08 17.06.2010
# SEM-ENGINEERING, BRYANSK 2010
#
# Object: БТС-2 НПС-3
#
# Location: УСО 1.0 А2
#
# File Version: 1.17
#
# Network Name: moxa_nps3_10a2
#
# Network Address: 192.0.0.12
#
 
./moxa7gate \
--tcp_port 502 \
--modbus_address 247 \
--status_info 1024 \
--show_sys_messages \
--Object "БТС-2 НПС-3" \
--Location "УСО 1.0 А2" \
--confVersion "1.17" \
--NetworkName "moxa_nps3_10a2" \
--NetworkAddress "192.0.0.12" \
PORT1 RS485_2W 9600 NONE 400 GATEWAY_SIMPLE 1502 --desc "Пожарные датчики резервуара РВСПК 102.1" \
PORT2 RS485_2W 9600 NONE 200 GATEWAY_ATM --desc "Подсистемы контроля загазованности УПЭС в УСО 5.5.2" \
PORT3 RS485_2W 9600 NONE 200 GATEWAY_ATM --desc "Приборы контроля вибрации А1, А2, А3 в УСО 5.5.1" \
PORT4 RS485_2W 9600 NONE 200 GATEWAY_RTM --desc "Пожарные датчики резервуара РВСПК 102.4" \
PORT5 RS232 9600 NONE 200 GATEWAY_RTM \
PORT6 RS485_2W 9600 NONE 200 GATEWAY_PROXY --desc "Пожарные датчики резервуара РВСПК 102.6" \
PORT7 RS485_2W 9600 NONE 1000 BRIDGE_PROXY --desc "Modicon Quantum 140 CPU 671 60" \
PORT8 RS485_2W 9600 NONE 1000 GATEWAY_SIMPLE  --desc "Удаленная система контроля вибрации на основе прибора Аргус-М" \
PROXY_TABLE 27 \
1024 13 123 P1 1 HOLDING_REGISTER 0 4 --desc "Задвижка 004.1.1.1" \
1037 12 136 P1 1 HOLDING_REGISTER 0 4 --desc "Задвижка 004.1.1.1" \
1 20 150 P1 2 HOLDING_REGISTER 0 4 --desc "Задвижка 004.1.1.2" \
1 20 170 P1 3 HOLDING_REGISTER 0 4 --desc "Задвижка 004.1.2.2" \
1024 13 223 P2 1 HOLDING_REGISTER 0 4 \
1037 12 236 P2 1 HOLDING_REGISTER 0 4 \
1 20 250 P2 2 HOLDING_REGISTER 0 4 \
1 20 270 P2 3 HOLDING_REGISTER 0 4 \
1 20 300 P3 1 HOLDING_REGISTER 0 4 \
1 20 320 P3 2 HOLDING_REGISTER 0 4 \
1 20 340 P3 3 HOLDING_REGISTER 0 4 \
1 25 400 P4 1 HOLDING_REGISTER 0 4 --desc "Задвижка 007.4.1" \
1 25 425 P4 2 HOLDING_REGISTER 0 4 --desc "Задвижка 007.4.2" \
1 25 450 P4 3 HOLDING_REGISTER 0 4 --desc "Задвижка 007.4.3" \
1 20 500 P5 1 HOLDING_REGISTER 0 4 \
1 20 520 P5 2 HOLDING_REGISTER 0 4 \
1 20 540 P5 3 HOLDING_REGISTER 0 4 \
1 20 600 P6 1 HOLDING_REGISTER 0 4 \
1 20 600 P6 1 COIL_STATUS 0 4 \
1 20 600 P6 1 INPUT_STATUS 0 4 \
1 20 700 P7 1 HOLDING_REGISTER 0 4 \
1 20 720 P7 2 HOLDING_REGISTER 0 4 \
1 20 740 P7 3 HOLDING_REGISTER 0 4 \
1 20 700 PT 1 HOLDING_REGISTER 0 4 \
1 20 720 PT 2 HOLDING_REGISTER 0 4 \
1 20 740 PT 3 HOLDING_REGISTER 0 4 \
1 20 600 P6 1 INPUT_REGISTER 0 4 \
RTM_TABLE 7 \
1001 200 P4 1 HOLDING_REGISTER --desc "Задвижка 004.1.1.1" \
1201 200 P4 2 HOLDING_REGISTER --desc "Задвижка 004.1.1.1" \
1401 200 P5 1 HOLDING_REGISTER --address_shift 2 --desc "Задвижка 004.1.1.2" \
1601 200 P5 2 HOLDING_REGISTER --desc "Задвижка 004.1.2.2" \
1201 200 P4 2 COIL_STATUS --desc "Задвижка 004.1.1.3" \
1201 200 P4 2 INPUT_STATUS --desc "Задвижка 004.1.1.4" \
1201 200 P4 2 INPUT_REGISTER --desc "Задвижка 004.1.1.5" \
TCP_SERVERS 3 \
10.0.6.240:502 1 0 P7 --desc "Система виброконтроля в УСО 1.1" \
192.0.0.252:502 2 0 P7 --desc "Система виброконтроля в УСО 1.2" \
127.0.0.1:6502 3 2 PT --desc "Система контроля загазованности" \
&
