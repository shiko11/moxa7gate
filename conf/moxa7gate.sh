#! /bin/sh
#
# MOXA7GATE MODBUS GATEWAY SOFTWARE VERSION 1.2
# SEM-ENGINEERING, BRYANSK 2018
#
# OBJECT: uport1150 host
# LOCATION: stend
# LABEL: A
#
# THIS CONFIGURATION FILE GENERATED AUTOMATICALLY
# AT 23.11.2018 17:03:54
# VERSION 1.8
#
# Device Model: MOXA UC-7410
# Network Name: tsreg
# LAN1 Address: 10.0.2.15
# LAN2 Address: 
#
 
./moxa7gate_i386 \
--Object "uport1150 host" \
--Location "stend" \
--Label "A" \
--NetworkName "tsreg" \
--LAN1Address "10.0.2.15" \
--LAN2Address "" \
--VersionNumber "1.8" \
--VersionTime "23.11.2018 17:03:54" \
--Model "MOXA UC-7410" \
--tcp_port 1502 \
--status_info 600 \
--show_sys_messages \
PORT5 RS485_2W 9600 NONE 400 RTU_MASTER --desc "DeltaV USO9_4 C04 P02" \
TCP01 192.168.127.1:502 1 0 1000 0.0.0.0:502 --desc "PLC Simulator" \
AT1 2560 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
QT P5  1 HOLDING_REGISTER R 1441 80  1   5 3 --desc "data_block_01" \
QT P5  1 HOLDING_REGISTER R 1521 80 81   5 3 --desc "data_block_02" \
QT T01 1 HOLDING_REGISTER W   21 80  1 185 3 --desc "data_block_01" \
QT T01 1 HOLDING_REGISTER W  101 80 81 185 3 --desc "data_block_02" \
&
