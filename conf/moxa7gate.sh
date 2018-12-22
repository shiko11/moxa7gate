#! /bin/sh
#
# MOXA7GATE MODBUS GATEWAY SOFTWARE VERSION 1.1
# SEM-ENGINEERING, BRYANSK 2010
#
# THIS CONFIGURATION FILE GENERATED AUTOMATICALLY
# AT 13:16:30 17.09.2010
#
# Object: ���. �����
# Location: ���� 
#
# File Version: 1.1
#
# Network Name: moxa
# Network Address: 192.0.0.12
#
 
./moxa7gate \
--tcp_port 502 \
--modbus_address 247 \
--status_info 200 \
--show_sys_messages \
--Object "���. �����" \
--Location "���� " \
--confVersion "1.1" \
--NetworkName "moxa" \
--NetworkAddress "192.0.0.12" \
PORT1 RS485_2W 9600 NONE 400 GATEWAY_PROXY --desc "��� ���������� ModSim �� ��������� ��" \
PORT5 RS232 9600 NONE 400 BRIDGE_PROXY --desc "���������� ModScan �� ��������� ��" \
PROXY_TABLE 5 \
1 8 1 P1 2 HOLDING_REGISTER 0 0 --desc "�������� ���� 1" \
9 8 9 P1 2 HOLDING_REGISTER 0 1 --desc "�������� ���� 2" \
17 16 17 P1 2 HOLDING_REGISTER 0 2 --desc "�������� ���� 3" \
1 16 33 P1 3 HOLDING_REGISTER 0 3 --desc "�������� ���� 4" \
17 16 49 P1 3 HOLDING_REGISTER 0 4 --desc "�������� ���� 5" \
&
