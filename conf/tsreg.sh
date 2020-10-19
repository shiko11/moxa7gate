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
# AT 20.07.2015 22:02:34
# VERSION 1.6
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
--VersionNumber "1.6" \
--VersionTime "20.07.2015 22:02:34" \
--Model "MOXA UC-7410" \
--tcp_port 502 \
--status_info 600 \
TCP01 192.168.81.1:502 1 0 1000 127.0.0.1:4242 --desc "IPs15" \
TCP02 192.168.81.1:502 1 0 1000 127.0.0.1:4242 --desc "IPs1" \
TCP03 192.168.81.1:502 1 0 1000 127.0.0.1:4242 --desc "PIDm100" \
TCP04 192.168.81.1:502 1 0 1000 127.0.0.1:4242 --desc "TPm100" \
AT1 2560 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
QT T01 1 HOLDING_REGISTER R 5533 76   1 1000 1 --desc "IPs15" \
QT T02 1 HOLDING_REGISTER R 5609 76  77 1000 1 --desc "IPs1" \
QT T03 1 HOLDING_REGISTER R 5685 76 153 1000 1 --desc "PIDm100" \
QT T04 1 HOLDING_REGISTER R 5761 76 229 1000 1 --desc "TPm100" \
&
