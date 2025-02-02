#!/bin/bash
#
# MOXA7GATE MODBUS GATEWAY SOFTWARE VERSION 1.2
# AO NPO SEM, BRYANSK 2024
#
# OBJECT: Стенд имитационный
# LOCATION: АО НА
# LABEL: A34
#
# THIS CONFIGURATION FILE GENERATED AUTOMATICALLY
# AT 30.12.2024 12:38:17
# VERSION 1.36
#
# Device Model: i386
# Network Name: KS_SIM
# LAN1 Address: 192.168.217.4
# LAN2 Address: 192.168.218.4
#
 
./moxa7gate_i386 \
--Object "Стенд имитационный" \
--Location "АО НА" \
--Label "A34" \
--NetworkName "KS_SIM" \
--LAN1Address "192.168.217.4" \
--LAN2Address "192.168.218.4" \
--VersionNumber "1.36" \
--VersionTime "30.12.2024 12:38:17" \
--Model "i386" \
--tcp_port 9502 \
--status_info 1 \
TCP01 192.168.217.4:1502 1 0 100 0.0.0.0:502 --desc "Имитатор КС А3 1РП" \
TCP02 192.168.217.4:2502 1 0 100 0.0.0.0:502 --desc "Имитатор КС А3 2РП" \
TCP03 192.168.217.4:3502 1 0 100 0.0.0.0:502 --desc "Имитатор КС А13 2РП" \
AT1 2560 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
QT T01 1 HOLDING_REGISTER RW 1 120 2001 100 0 --desc "Блок диагностики для передачи из КС А3" \
QT T01 1 HOLDING_REGISTER RW 121 125 2121 100 0 --desc "1РП.САР.1 (основной канал). Чтение" \
QT T01 1 HOLDING_REGISTER RW 246 125 2246 100 0 --desc "1РП.САР.2 (основной канал). Чтение" \
QT T01 1 HOLDING_REGISTER RW 371 125 2371 100 0 --desc "1РП.САР.3 (основной канал). Чтение" \
QT T01 1 HOLDING_REGISTER RW 496 125 2496 100 0 --desc "1РП.САР.4 (основной канал). Чтение" \
QT T01 1 HOLDING_REGISTER RW 621 125 2621 100 0 --desc "1РП.САР.5 (основной канал). Чтение" \
QT T01 1 HOLDING_REGISTER RW 746 125 2746 100 0 --desc "1РП.САР.6 (основной канал). Чтение" \
QT T01 1 HOLDING_REGISTER RW 871 125 2871 100 0 --desc "СА 3РП ПП «Грушовая» (основной канал). Чтение; СА ПТ" \
QT T01 1 HOLDING_REGISTER RW 996 125 2996 100 0 --desc "СА 2РП ПП «Шесхарис» (основной канал). Чтение 1" \
QT T01 1 HOLDING_REGISTER RW 1121 125 3121 100 0 --desc "СА 2РП ПП «Шесхарис» (основной канал). Чтение 2" \
QT T01 1 HOLDING_REGISTER RW 10001 123 4001 100 0 --desc "1РП.САР.1-4 (основной канал). Запись" \
QT T01 1 HOLDING_REGISTER RW 10124 123 4124 100 0 --desc "1РП.САР.5-6 (основной канал). Запись" \
QT T01 1 HOLDING_REGISTER RW 10247 123 4247 100 0 --desc "СА 3РП ПП «Грушовая» (основной канал). Запись" \
QT T01 1 HOLDING_REGISTER RW 10370 123 4370 100 0 --desc "СА 2РП ПП «Шесхарис» (основной канал). Запись 1" \
QT T01 1 HOLDING_REGISTER RW 10493 123 4493 100 0 --desc "СА 2РП ПП «Шесхарис» (основной канал). Запись 2" \
QT T02 1 HOLDING_REGISTER RW 1 108 5001 100 0 --desc "KSA3toKC_Diag" \
QT T02 1 HOLDING_REGISTER RW 109 125 5109 100 0 --desc "KSA3toKC_Block1" \
QT T02 1 HOLDING_REGISTER RW 234 125 5234 100 0 --desc "KSA3toKC_Block2" \
QT T02 1 HOLDING_REGISTER RW 359 125 5359 100 0 --desc "KSA3toKC_Block3" \
QT T02 1 HOLDING_REGISTER RW 484 125 5484 100 0 --desc "KSA3toKC_Block4" \
QT T02 1 HOLDING_REGISTER RW 609 125 5609 100 0 --desc "KSA3toKC_Block5" \
QT T02 1 HOLDING_REGISTER RW 734 125 5734 100 0 --desc "KSA3toKC_Block6" \
QT T02 1 HOLDING_REGISTER RW 859 125 5859 100 0 --desc "KSA3toKC_Block7" \
QT T02 1 HOLDING_REGISTER RW 984 125 5984 100 0 --desc "KSA3toKC_Block8" \
QT T02 1 HOLDING_REGISTER RW 1109 125 6109 100 0 --desc "KSA3toKC_Block9" \
QT T02 1 HOLDING_REGISTER RW 1234 125 6234 100 0 --desc "KSA3toKC_Block10" \
QT T02 1 HOLDING_REGISTER RW 10001 123 7001 100 0 --desc "KCtoKSA3_Block1" \
QT T02 1 HOLDING_REGISTER RW 10124 123 7124 100 0 --desc "KCtoKSA3_Block2" \
QT T02 1 HOLDING_REGISTER RW 10247 123 7247 100 0 --desc "KCtoKSA3_Block3" \
QT T02 1 HOLDING_REGISTER RW 10370 123 7370 100 0 --desc "KCtoKSA3_Block4" \
QT T02 1 HOLDING_REGISTER RW 10493 123 7493 100 0 --desc "KCtoKSA3_Block5" \
QT T02 1 HOLDING_REGISTER RW 10616 123 7616 100 0 --desc "KCtoKSA3_Block6" \
QT T02 1 HOLDING_REGISTER RW 10739 123 7739 100 0 --desc "KCtoKSA3_Block7" \
QT T03 1 HOLDING_REGISTER RW 1 108 8001 100 0 --desc "KSA13toKC_Diag" \
QT T03 1 HOLDING_REGISTER RW 109 125 8109 100 0 --desc "KSA13toKC_Block1" \
QT T03 1 HOLDING_REGISTER RW 234 125 8234 100 0 --desc "KSA13toKC_Block2" \
QT T03 1 HOLDING_REGISTER RW 359 125 8359 100 0 --desc "KSA13toKC_Block3" \
QT T03 1 HOLDING_REGISTER RW 484 125 8484 100 0 --desc "KSA13toKC_Block4" \
QT T03 1 HOLDING_REGISTER RW 609 125 8609 100 0 --desc "KSA13toKC_Block5" \
QT T03 1 HOLDING_REGISTER RW 734 125 8734 100 0 --desc "KSA13toKC_Block6" \
QT T03 1 HOLDING_REGISTER RW 859 125 8859 100 0 --desc "KSA13toKC_Block7" \
QT T03 1 HOLDING_REGISTER RW 984 125 8984 100 0 --desc "KSA13toKC_Block8" \
QT T03 1 HOLDING_REGISTER RW 1109 125 9109 100 0 --desc "KSA13toKC_Block9" \
QT T03 1 HOLDING_REGISTER RW 10001 123 10001 100 0 --desc "KCtoKSA13_Block1" \
QT T03 1 HOLDING_REGISTER RW 10124 123 10124 100 0 --desc "KCtoKSA13_Block2" \
&
