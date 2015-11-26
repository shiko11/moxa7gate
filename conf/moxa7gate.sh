#! /bin/sh
#
# MOXA7GATE MODBUS GATEWAY SOFTWARE VERSION 1.2
# SEM-ENGINEERING, BRYANSK 2015
#
# OBJECT: Стенд имитационный
# LOCATION: АО НПО СЭМ
# LABEL: A1
#
# THIS CONFIGURATION FILE GENERATED AUTOMATICALLY
# AT 16.11.2015 13:46:56
# VERSION 1.3
#
# Device Model: i386
# Network Name: mxdev
# LAN1 Address: 192.168.0.11
# LAN2 Address: 192.168.1.11
#
 
./moxa7gate_cygwin.exe \
--Object "Стенд имитационный" \
--Location "АО НПО СЭМ" \
--Label "A1" \
--NetworkName "mxdev" \
--LAN1Address "192.168.0.11" \
--LAN2Address "192.168.1.11" \
--VersionNumber "1.3" \
--VersionTime "16.11.2015 13:46:56" \
--Model "i386" \
--tcp_port 1502 \
--status_info 1000 \
TCP01 192.168.0.3:502 1 0 1000 0.0.0.0:502 --desc "Стенд имитационный" \
TCP02 192.168.0.11:502 1 0 1000 0.0.0.0:502 --desc "КЦ АСУ" \
AT1 2560 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
QT T01 1 HOLDING_REGISTER R 3301 20 1 100 2 --desc "Стенд имитационный" \
QT T01 1 HOLDING_REGISTER W 3321 20 21 100 2 --desc "Стенд имитационный" \
QT T02 1 HOLDING_REGISTER R 4200 20 21 100 2 --desc "КЦ АСУ" \
QT T02 1 HOLDING_REGISTER W 4220 20 1 100 2 --desc "КЦ АСУ" \
&
