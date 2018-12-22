#! /bin/sh
#
# MOXA7GATE MODBUS GATEWAY SOFTWARE VERSION 1.2
# SEM-ENGINEERING, BRYANSK 2010
#
# OBJECT: ���� ���� ��������
# LOCATION: ��������� 689 ��� 9.0
# LABEL: A4
#
# THIS CONFIGURATION FILE GENERATED AUTOMATICALLY
# AT 26.11.2010 17:22:29
# VERSION 1.31
#
# Device Model: MOXA UC-7410
# Network Name: moxa_bips689_90a4
# LAN1 Address: 10.0.6.37
# LAN2 Address: 192.168.4.127
#
 
/root/moxa7gate \
--Object "���� ���� ��������" \
--Location "��������� 689 ��� 9.0" \
--Label "A4" \
--NetworkName "moxa_bips689_90a4" \
--LAN1Address "10.0.6.37" \
--LAN2Address "192.168.4.127" \
--VersionNumber "1.31" \
--VersionTime "26.11.2010 17:22:29" \
--Model "MOXA UC-7410" \
--tcp_port 502 \
--modbus_address 1 \
--status_info 1 \
--show_sys_messages \
--map2Xto4X \
--watchdog_timer \
--show_data_flow \
--use_buzzer \
PORT1 RS485_2W 9600 NONE 1000 TCP_SERVER --desc "��������� ModSim �� ��������� ��" \
PORT2 RS485_2W 9600 NONE 1000 RTU_MASTER --desc "�������� ����� �� ������ �������� ��������" \
PORT8 RS485_2W 9600 NONE 1000 RTU_SLAVE --desc "���������� modbus-������ �� ������� RTU" \
TCP01 10.0.1.252:502 1 0 2 10.0.2.252:502 --desc "MODSIM �� ��������� ��" \
PROXY_TABLE 9 \
T01 1 HOLDING_REGISTER R 1 8 33 100 2 --desc "�������� ���� ���������" \
P2 1 HOLDING_REGISTER R 2 1 41 0 2 --desc "���� �1" \
P2 3 HOLDING_REGISTER R 2 1 42 0 2 --desc "���� �3" \
P2 5 HOLDING_REGISTER R 2 1 43 0 2 --desc "���� �5" \
P2 7 HOLDING_REGISTER R 2 1 44 0 2 --desc "���� �7" \
P2 9 HOLDING_REGISTER R 2 1 45 0 2 --desc "���� �9" \
P2 11 HOLDING_REGISTER R 2 1 46 0 2 --desc "���� �11" \
P2 13 HOLDING_REGISTER R 2 1 47 0 2 --desc "���� �13" \
P2 15 HOLDING_REGISTER R 2 1 48 0 2 --desc "���� �15" \
RTM_TABLE 2 \
P2 1 HOLDING_REGISTER 0 1 200 --desc "Virtual Device 1" \
T1 1 HOLDING_REGISTER 0 1 200 --desc "Virtual Device 2" \
EXCEPTIONS 1 \
RESPONSE_RECV_RAW EXCP_SKS07_DIOGEN 3 3 6 0 --desc "����������� ������������� ������ �� ������� ����������." \
ATM_TABLE \
0 0 257 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
0 0 259 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
0 0 261 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
0 0 263 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
0 0 265 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
0 0 267 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
0 0 269 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
0 0 271 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
&
