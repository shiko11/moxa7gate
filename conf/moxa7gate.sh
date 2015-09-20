#! /bin/sh
#
# MOXA7GATE MODBUS GATEWAY SOFTWARE VERSION 1.2
# SEM-ENGINEERING, BRYANSK 2015
#
# OBJECT: tsreg host
# LOCATION: stend
# LABEL: A
#
# THIS CONFIGURATION FILE GENERATED AUTOMATICALLY
# AT 10.09.2015 16:20:49
# VERSION 1.8
#
# Device Model: MOXA UC-7410
# Network Name: tsreg
# LAN1 Address: 10.0.2.15
# LAN2 Address: 
#
 
./moxa7gate_i386 \
--Object "tsreg host" \
--Location "stend" \
--Label "A" \
--NetworkName "tsreg" \
--LAN1Address "10.0.2.15" \
--LAN2Address "" \
--VersionNumber "1.8" \
--VersionTime "10.09.2015 16:20:49" \
--Model "MOXA UC-7410" \
--tcp_port 502 \
--status_info 400 \
TCP01 192.168.201.7:502 1 0 1000 127.0.0.1:4242 --desc "SAR" \
AT1 2560 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
QT T01 1 HOLDING_REGISTER R 3000 808 1 1000 1 --desc "SAR" \
&
