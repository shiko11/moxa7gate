#! /bin/sh
#
# MOXA7GATE MODBUS GATEWAY SOFTWARE VERSION 1.2
# SEM-ENGINEERING, BRYANSK 2014
#
# OBJECT: ойс крл
# LOCATION: сян 1
# LABEL: A4
#
# THIS CONFIGURATION FILE GENERATED AUTOMATICALLY
# AT 17.01.2014 15:27:06
# VERSION 1.10
#
# Device Model: EM1240
# Network Name: EM1240
# LAN1 Address: 192.168.3.127
# LAN2 Address: 192.168.4.127
#
 
/home/km400 \
--Object "ойс крл" \
--Location "сян 1" \
--Label "A4" \
--NetworkName "EM1240" \
--LAN1Address "192.168.3.127" \
--LAN2Address "192.168.4.127" \
--VersionNumber "1.10" \
--VersionTime "17.01.2014 15:27:06" \
--Model "EM1240" \
--tcp_port 502 \
--status_info 1 \
--show_sys_messages \
PORT1 RS232 115200 EVEN 200 RTU_MASTER --desc "M340" \
TCP01 192.168.3.252:502 1 0 200 0.0.0.0:502 --desc "M340" \
AT1 2560 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
RT P1 1 HOLDING_REGISTER 1660 1680 59 --desc "CM" \
RT T01 1 HOLDING_REGISTER 1660 1740 59 --desc "CM" \
QT P1 1 HOLDING_REGISTER R 1560 59 1560 0 2 --desc "PLC" \
QT T01 1 HOLDING_REGISTER R 1560 59 1620 20 2 --desc "PLC" \
&
