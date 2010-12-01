#! /bin/sh
#
# MOXA7GATE MODBUS GATEWAY SOFTWARE VERSION 1.2
# SEM-ENGINEERING, BRYANSK 2010
#
# OBJECT: БиПС СМНП Козьмино
# LOCATION: Кроссовая 689 УСО 9.0
# LABEL: A4
#
# THIS CONFIGURATION FILE GENERATED AUTOMATICALLY
# AT 01.12.2010 14:37:04
# VERSION 1.40
#
# Device Model: MOXA UC-7410
# Network Name: moxa_bips689_90a4
# LAN1 Address: 10.0.6.37
# LAN2 Address: 192.168.4.127
#
 
/root/moxa7gate \
--Object "БиПС СМНП Козьмино" \
--Location "Кроссовая 689 УСО 9.0" \
--Label "A4" \
--NetworkName "moxa_bips689_90a4" \
--LAN1Address "10.0.6.37" \
--LAN2Address "192.168.4.127" \
--VersionNumber "1.40" \
--VersionTime "01.12.2010 14:37:04" \
--Model "MOXA UC-7410" \
--tcp_port 502 \
--status_info 1 \
--show_sys_messages \
--map2Xto4X \
--watchdog_timer \
--show_data_flow \
--use_buzzer \
PORT1 RS485_2W 9600 NONE 1000 TCP_SERVER 1502 --desc "Программа ModSim на локальном ПК" \
PORT2 RS485_2W 9600 NONE 1000 RTU_MASTER --desc "Тестовый шлейф из восьми пожарных датчиков" \
PORT8 RS485_2W 9600 NONE 1000 RTU_SLAVE --desc "Устройство modbus-мастер на стороне RTU" \
TCP01 10.0.1.252:502 1 0 10.0.2.252:502 --desc "MODSIM на локальном ПК" \
AT1 2560 4098 257 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
AT2 0 0 259 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
AT3 0 0 261 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
AT4 0 0 263 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
AT5 0 0 265 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
AT6 0 0 267 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
AT7 0 0 269 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
AT8 0 0 271 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
RT P2 1 HOLDING_REGISTER 0 1 200 --desc "Virtual Device 1" \
RT T01 2 HOLDING_REGISTER 0 1 200 --desc "Virtual Device 2" \
QT T01 2 HOLDING_REGISTER R 1 8 33 100 2 --desc "Тестовый блок регистров" \
QT P2 1 HOLDING_REGISTER R 2 1 41 0 2 --desc "ИПЭС №1" \
QT P2 3 HOLDING_REGISTER R 2 1 42 0 2 --desc "ИПЭС №3" \
QT P2 5 HOLDING_REGISTER R 2 1 43 0 2 --desc "ИПЭС №5" \
QT P2 7 HOLDING_REGISTER R 2 1 44 0 2 --desc "ИПЭС №7" \
QT P2 9 HOLDING_REGISTER R 2 1 45 0 2 --desc "ИПЭС №9" \
QT P2 11 HOLDING_REGISTER R 2 1 46 0 2 --desc "ИПЭС №11" \
QT P2 13 HOLDING_REGISTER R 2 1 47 0 2 --desc "ИПЭС №13" \
QT P2 15 HOLDING_REGISTER R 2 1 48 0 2 --desc "ИПЭС №15" \
EXPT RESPONSE_RECV_RAW SKS07_DIOGEN 3 3 0 0 --desc "Диоген №15" \
&
