#! /bin/sh
#
# MOXA7GATE MODBUS GATEWAY SOFTWARE VERSION 1.2
# SEM-ENGINEERING, BRYANSK 2013
#
# OBJECT: ПК ЛТМ
# LOCATION: Блок-бокс
# LABEL: A4
#
# THIS CONFIGURATION FILE GENERATED AUTOMATICALLY
# AT 29.11.2013 16:24:29
# VERSION 3.136
#
# Device Model: MOXA UC-7410
# Network Name: EM1240
# LAN1 Address: 192.168.5.125
# LAN2 Address: 192.168.6.125
#
 
/root/moxa7gate_v12 \
--Object "ПК ЛТМ" \
--Location "Блок-бокс" \
--Label "A4" \
--NetworkName "EM1240" \
--LAN1Address "192.168.5.125" \
--LAN2Address "192.168.6.125" \
--VersionNumber "3.136" \
--VersionTime "29.11.2013 16:24:29" \
--Model "MOXA UC-7410" \
--tcp_port 502 \
--status_info 1 \
PORT1 RS232 9600 NONE 100 RTU_SLAVE --desc "MODSCAN PC" \
PORT2 RS232 9600 NONE 1000 RTU_MASTER --desc "MODSIM PC" \
TCP01 192.168.5.252:502 52 0 1000 192.168.6.252:502 --desc "MODSIM на локальном ПК" \
AT1 2560 0 0 0 0 0 0 0 0 0 0 258 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
QT P2 2 HOLDING_REGISTER RW 1 40 3001 0 2 --desc "MODSIM PC" \
QT T01 52 HOLDING_REGISTER RW 1 40 3041 100 2 --desc "MODSIM PC" \
&
