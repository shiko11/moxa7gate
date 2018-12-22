#! /bin/sh
#
# MOXA7GATE MODBUS GATEWAY SOFTWARE VERSION 1.2
# SEM-ENGINEERING, BRYANSK 2010
#
# THIS CONFIGURATION FILE GENERATED AUTOMATICALLY
# AT 19:49:11 11.12.2010
#
# Object: ���. ����
# Location: ��������� (214) A4
#
# File Version: 1.2
#
# Network Name: moxa_rp
# Network Address: 192.168.5.126
 
/root/moxa7gate \
--tcp_port 502 \
--modbus_address 1 \
--status_info 2000 \
--show_sys_messages \
--Object "���. ����" \
--Location "��������� (214) A4" \
--confVersion "1.2" \
--NetworkName "moxa_rp" \
--NetworkAddress "192.168.5.126" \
PORT1 RS485_2W 19200 EVEN 1000 GATEWAY_PROXY --desc "����" \
PORT3 RS485_2W 19200 EVEN 1000 BRIDGE_PROXY --desc "���������� ������������ 2" \
PORT4 RS485_2W 19200 NONE 1000 BRIDGE_PROXY --desc "���������� ������������ 1" \
PROXY_TABLE 13 \
1 50 1 P1 1 HOLDING_REGISTER 1000 3 \
51 50 51 P1 1 HOLDING_REGISTER 1000 3 \
101 57 101 P1 1 HOLDING_REGISTER 500 3 \
201 50 201 P1 1 HOLDING_REGISTER 500 3 \
251 50 251 P1 1 HOLDING_REGISTER 500 3 \
301 50 301 P1 1 HOLDING_REGISTER 500 3 \
351 50 351 P1 1 HOLDING_REGISTER 500 3 \
401 51 401 P1 1 HOLDING_REGISTER 500 3 \
501 51 501 P1 1 HOLDING_REGISTER 500 3 \
601 51 601 P1 1 HOLDING_REGISTER 500 3 \
701 51 701 P1 1 HOLDING_REGISTER 500 3 \
1562 40 1562 T1 1 HOLDING_REGISTER 100 4 --desc "���� ��������� ��" \
1662 70 1662 T1 1 HOLDING_REGISTER 100 4 --desc "���� ��������� ��" \
TCP_SERVERS 1 \
192.168.5.121:502 1 0 P1 --desc "���������� ���. �����" \
&
