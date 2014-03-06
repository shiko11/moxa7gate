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

#include "kltm/kltm.h"

///=== CLI_H constants

// конфигурационные константы времени компиляции

// семь констант по количеству типов структур данных										

#define MAX_COMMON_KEYS 16        // максимальное количество общих параметров конфигурации шлюза (с префиксом "--")
#define SERIAL_PARAMETERS 6       // количество параметров конфигурации последовательного интерфейса
#define LANTCP_PARAMETERS 6       // количество параметров конфигурации логического TCP интерфейса
#define ADDRESSMAP_PARAMETERS 32  // количество записей таблицы назначения адресов в конфигурационном блоке данных
#define RTM_TABLE_PARAMETERS 7    // количество параметров конфигурации в одной записи таблицы назначения регистров
#define PROXY_TABLE_PARAMETERS 10 // количество параметров конфигурации в одной записи таблицы опроса
#define EXCEPTION_PARAMETERS 7    // количество параметров конфигурации в одной записи таблицы исключений

// ошибки анализа командной строки

#define COMMAND_LINE_OK 0
#define COMMAND_LINE_ERROR 1
#define COMMAND_LINE_INFO 2
#define COMMAND_LINE_ARGC 3
#define COMMAND_LINE_UNDEFINED 4

#define SECURITY_CONF_STRUCT 5
#define SECURITY_CONF_DUPLICATE 6
#define SECURITY_CONF_SPELLING 7
#define SECURITY_KLTM_CONF_IFACE 7

#define IFACE_CONF_RTUDUPLICATE 10
#define IFACE_CONF_RTUSTRUCT 11
#define IFACE_CONF_GWMODE 12

#define IFACE_CONF_TCPDUPLICATE 18
#define IFACE_CONF_TCPSTRUCT 19

#define ATM_CONF_SPELLING 28
#define ATM_CONF_STRUCT 29

#define VSLAVE_CONF_OVERFLOW 32
#define VSLAVE_CONF_STRUCT 33
#define VSLAVE_CONF_IFACE 34

#define PQUERY_CONF_OVERFLOW 40
#define PQUERY_CONF_STRUCT 41
#define PQUERY_CONF_IFACE 42

#define EXPT_CONF_OVERFLOW 52
#define EXPT_CONF_STRUCT 53
#define EXPT_CONF_STAGE 54

/// коды ошибок верификации конфигурации в целом

#define CONFIG_TCPPORT_CONFLICT 77
#define CONFIG_ADDRMAP 78
#define CONFIG_FORWARDING 79
#define CONFIG_GATEWAY 80
#define CONFIG_ADDRMAP_INTEGRITY 81
#define CONFIG_VSLAVES_INTEGRITY_IFACE 82
#define CONFIG_VSLAVES_INTEGRITY_DIAP 83
#define CONFIG_PQUERIES_INTEGRITY_IFACE 84
#define CONFIG_PQUERIES_INTEGRITY_DIAP 85

///=== CLI_H public variables

///=== CLI_H public functions

int get_command_line(int argc, char *argv[]);
int get_ip_from_string(char *str, unsigned int *ip, unsigned int *port);

int check_GatewayTCPPorts();
int check_GatewayAddressMap();
int check_GatewayIfaces_ex();    // кроме проверки корректности конфигурации эта функция заполняет массивы индесов
int check_GatewayConf();
int check_IntegrityAddressMap();
int check_IntegrityVSlaves();
int check_IntegrityPQueries();

//void sigpipe_handler();
//void sigio_handler();

#endif  /* CLI_H */
