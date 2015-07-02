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
# AT 30.06.2015 14:23:37
# VERSION 1.3
#
# Device Model: MOXA UC-7410
# Network Name: tsreg
# LAN1 Address: 10.0.2.15
# LAN2 Address: 
#
 
/root/moxa7gate_UC7410 \
--Object "tsreg host" \
--Location "stend" \
--Label "A" \
--NetworkName "tsreg" \
--LAN1Address "10.0.2.15" \
--LAN2Address "" \
--VersionNumber "1.3" \
--VersionTime "30.06.2015 14:23:37" \
--Model "MOXA UC-7410" \
--tcp_port 502 \
--status_info 200 \
TCP01 192.168.56.1:502 1 0 1000 0.0.0.0:502 --desc "MODSIM на локальном ПК" \
TCP02 192.168.56.1:502 1 0 1000 0.0.0.0:502 --desc "MODSIM на локальном ПК" \
AT1 2560 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
QT T01 1 HOLDING_REGISTER R 26188 36 1 400 1 --desc "Буфер истории H_IPs15" \
QT T02 1 HOLDING_REGISTER R 26112 76 40 800 1 --desc "Буфер истории H_IPs1" \
&
