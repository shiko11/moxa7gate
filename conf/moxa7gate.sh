#! /bin/sh
#
# MOXA7GATE MODBUS GATEWAY SOFTWARE VERSION 1.2
# SEM-ENGINEERING, BRYANSK 2013
#
# OBJECT: ПКУ ЛТМ
# LOCATION: УСО 1
# LABEL: A4
#
# THIS CONFIGURATION FILE GENERATED AUTOMATICALLY
# AT 19.12.2013 09:20:36
# VERSION 1.8
#
# Device Model: EM1240
# Network Name: EM1240
# LAN1 Address: 192.168.3.127
# LAN2 Address: 192.168.4.127
#
 
/home/km400 \
--Object "ПКУ ЛТМ" \
--Location "УСО 1" \
--Label "A4" \
--NetworkName "EM1240" \
--LAN1Address "192.168.3.127" \
--LAN2Address "192.168.4.127" \
--VersionNumber "1.8" \
--VersionTime "19.12.2013 09:20:36" \
--Model "EM1240" \
--tcp_port 502 \
--status_info 1 \
--show_sys_messages \
--show_data_flow \
PORT1 RS232 19200 EVEN 400 RTU_MASTER --desc "Контроллер линейной телемеханики" \
AT1 2560 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
RT P1 1 HOLDING_REGISTER 1659 1659 59 --desc "КЛТМ, переменная CM" \
QT P1 1 HOLDING_REGISTER R 1559 59 1559 0 2 --desc "КЛТМ, переменная PLC" \
&
