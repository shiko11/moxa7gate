#! /bin/sh
#
# MOXA7GATE MODBUS GATEWAY SOFTWARE VERSION 1.2
# SEM-ENGINEERING, BRYANSK 2011
#
# OBJECT: БиПС СМНП Козьмино
# LOCATION: Кроссовая 689 УСО 9.0
# LABEL: A4
#
# THIS CONFIGURATION FILE GENERATED AUTOMATICALLY
# AT 16.11.2011 11:26:50
# VERSION 1.128
#
# Device Model: MOXA UC-7410
# Network Name: moxa_ms689_90a4
# LAN1 Address: 192.168.5.126
# LAN2 Address: 192.168.6.126
#
 
/root/moxa7gate \
--Object "БиПС СМНП Козьмино" \
--Location "Кроссовая 689 УСО 9.0" \
--Label "A4" \
--NetworkName "moxa_ms689_90a4" \
--LAN1Address "192.168.5.126" \
--LAN2Address "192.168.6.126" \
--VersionNumber "1.128" \
--VersionTime "16.11.2011 11:26:50" \
--Model "MOXA UC-7410" \
--tcp_port 502 \
--status_info 1 \
--show_sys_messages \
--use_buzzer \
PORT1 RS485_2W 9600 NONE 1000 TCP_SERVER 1502 --desc "Программа ModSim на локальном ПК" \
PORT2 RS485_2W 9600 NONE 100 RTU_SLAVE --desc "Устройство modbus-мастер на стороне RTU" \
PORT8 RS485_2W 9600 NONE 1000 RTU_MASTER --desc "Тестовый шлейф из восьми пожарных датчиков" \
AT1 0 0 1844 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
AT4 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2560 0 0 \
&
