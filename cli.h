/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

#ifndef CLI_H
#define CLI_H

///**************** МОДУЛЬ РАЗБОРА ПАРАМЕТРОВ СТАРТОВОЙ КОНФИГУРАЦИИ ***********

#include "interfaces.h"
#include "moxagate.h"

///=== CLI_H constants

#define MAX_COMMON_KEYS 16 // максимальное количество общих параметров конфигурации шлюза (с префиксом "--")
#define SERIAL_PARAMETERS 6 // количество параметров конфигурации последовательного интерфейса
#define LANTCP_PARAMETERS 5 // количество параметров конфигурации логического TCP интерфейса
#define ADDRESSMAP_PARAMETERS 32 // количество записей таблицы назначения адресов в конфигурационном блоке данных
#define RTM_TABLE_PARAMETERS 7 // количество параметров конфигурации в одной записи таблицы назначения регистров
#define PROXY_TABLE_PARAMETERS 10 // количество параметров конфигурации в одной записи таблицы опроса
#define EXCEPTION_PARAMETERS 7 // количество параметров конфигурации в одной записи таблицы исключений

// ошибки анализа командной строки
// и верификации полученных структур данных

#define COMMAND_LINE_OK 0
#define COMMAND_LINE_ERROR 1
#define COMMAND_LINE_INFO 2
#define COMMAND_LINE_ARGC 3

#define SECURITY_CONF_STRUCT 4
#define SECURITY_CONF_DUPLICATE 5
#define SECURITY_CONF_SPELLING 6

#define SECURITY_CONF_TCPPORT 7
#define SECURITY_CONF_MBADDR 8
#define SECURITY_CONF_STATINFO 9

#define IFACE_CONF_RTUDUPLICATE 10
#define IFACE_CONF_RTUSTRUCT 11

#define IFACE_CONF_MBMODE 12
#define IFACE_CONF_RTUPHYSPROT 13
#define IFACE_CONF_RTUSPEED 14
#define IFACE_CONF_RTUPARITY 15
#define IFACE_CONF_RTUTIMEOUT 16
#define IFACE_CONF_RTUTCPPORT 17

#define IFACE_CONF_TCPDUPLICATE 18
#define IFACE_CONF_TCPSTRUCT 19

#define IFACE_CONF_TCPIP1 20
#define IFACE_CONF_TCPPORT1 21
#define IFACE_CONF_TCPUNITID 22
#define IFACE_CONF_TCPOFFSET 23
#define IFACE_CONF_TCPMBADDR 24
#define IFACE_CONF_TCPIP2 25
#define IFACE_CONF_TCPPORT2 26
#define IFACE_CONF_TCPIPEQUAL 27

#define ATM_CONF_SPELLING 28
#define ATM_CONF_STRUCT 29

#define ATM_CONF_IFACE 30
#define ATM_CONF_MBADDR 31

#define VSLAVE_CONF_OVERFLOW 32
#define VSLAVE_CONF_STRUCT 33

#define VSLAVE_CONF_IFACE 34
#define VSLAVE_CONF_MBADDR 35
#define VSLAVE_CONF_MBTABL 36
#define VSLAVE_CONF_BEGDIAP 37
#define VSLAVE_CONF_ENDDIAP 38
#define VSLAVE_CONF_LENDIAP 39

#define PQUERY_CONF_OVERFLOW 40
#define PQUERY_CONF_STRUCT 41

#define PQUERY_CONF_IFACE 42
#define PQUERY_CONF_MBADDR 43
#define PQUERY_CONF_MBTABL 44
#define PQUERY_CONF_ACCESS 45
#define PQUERY_CONF_ENDREGREAD 46
#define PQUERY_CONF_LENPACKET 47
#define PQUERY_CONF_ENDREGWRITE 48
#define PQUERY_CONF_DELAYMIN 49
#define PQUERY_CONF_DELAYMAX 50
#define PQUERY_CONF_ERRCNTR 51

#define EXPT_CONF_OVERFLOW 52
#define EXPT_CONF_STRUCT 53

#define EXPT_CONF_STAGE 54
#define EXPT_CONF_ACTION 55
#define EXPT_CONF_PRM1 56
#define EXPT_CONF_PRM2 57
#define EXPT_CONF_PRM3 58
#define EXPT_CONF_PRM4 59

///=== CLI_H public variables

///=== CLI_H public functions

int get_command_line (int 	argc, char	*argv[]);
int get_ip_from_string(char *str, unsigned int *ip, unsigned int *port);

int check_GatewayTCPPorts();
int check_GatewayAddressMap();
int check_GatewayIfaces();
int check_GatewayConf();
int check_IntegrityAddressMap();
int check_IntegrityVSlaves();
int check_IntegrityPQueries();

//void sigpipe_handler();
//void sigio_handler();

#endif  /* CLI_H */
