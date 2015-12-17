#! /bin/sh
#
# MOXA7GATE MODBUS GATEWAY SOFTWARE VERSION 1.2
# AO NPO SEM, BRYANSK 2015
#
# OBJECT: Стенд имитационный
# LOCATION: АО НПО СЭМ
# LABEL: A1
#
# THIS CONFIGURATION FILE GENERATED AUTOMATICALLY
# AT 17.12.2015 17:18:18
# VERSION 1.7
#
# Device Model: MOXA UC-8410
# Network Name: mxdev
# LAN1 Address: 192.168.0.11
# LAN2 Address: 192.168.1.11
#
 
/home/root/moxa7gate_UC8410 \
--Object "Стенд имитационный" \
--Location "АО НПО СЭМ" \
--Label "A1" \
--NetworkName "mxdev" \
--LAN1Address "192.168.0.11" \
--LAN2Address "192.168.1.11" \
--VersionNumber "1.7" \
--VersionTime "17.12.2015 17:18:18" \
--Model "MOXA UC-8410" \
--tcp_port 502 \
--status_info 1000 \
--map3Xto4X \
PORT1 RS485_2W 9600 NONE 1000 RTU_MASTER --desc "Шлейф задвижек AUMA" \
AT1 2560 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
QT P1 1 INPUT_REGISTER R 1000 8 1 100 2 --desc "Стенд имитационный" \
&
