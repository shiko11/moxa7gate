/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010                 
***************************************/

///*********************** МОДУЛЬ ИНТЕРФЕЙСОВ ШЛЮЗА ****************************
///*** МЕХАНИЗМ ОПРОСА MODBUS RTU MASTER
///*** МЕХАНИЗМ ОПРОСА MODBUS TCP MASTER
///*** МЕХАНИЗМ ОБСЛУЖИВАНИЯ MODBUS RTU SLAVE
///*** МЕХАНИЗМ ПЕРЕНАПРАВЛЕНИЯ GATEWAY_SIMPLE

#ifndef INTERFACES_H
#define INTERFACES_H

#define   MAX_MOXA_PORTS 8

#include "forwarding.h"
#include "clients.h"

///=== INTERFACES_H constants

// конфигурационные константы времени компиляции

/// MOXA7GATE ASSETS

#define GATEWAY_ASSETS 64

#define GATEWAY_P1	0
#define GATEWAY_P2	1
#define GATEWAY_P3	2
#define GATEWAY_P4	3
#define GATEWAY_P5	4
#define GATEWAY_P6	5
#define GATEWAY_P7	6
#define GATEWAY_P8	7

#define GATEWAY_P9        8
#define GATEWAY_SYSTEM    9
#define GATEWAY_MOXAGATE  10
#define GATEWAY_LANTCP    11
#define GATEWAY_SECURITY  12
#define GATEWAY_FRWD      13
#define GATEWAY_RESERVE   14
#define GATEWAY_HMI       15

#define GATEWAY_T01 16
#define GATEWAY_T02 17
#define GATEWAY_T03 18
#define GATEWAY_T04 19
#define GATEWAY_T05 20
#define GATEWAY_T06 21
#define GATEWAY_T07 22
#define GATEWAY_T08 23

#define GATEWAY_T09 24
#define GATEWAY_T10 25
#define GATEWAY_T11 26
#define GATEWAY_T12 27
#define GATEWAY_T13 28
#define GATEWAY_T14 29
#define GATEWAY_T15 30
#define GATEWAY_T16 31

#define GATEWAY_T17 32
#define GATEWAY_T18 33
#define GATEWAY_T19 34
#define GATEWAY_T20 35
#define GATEWAY_T21 36
#define GATEWAY_T22 37
#define GATEWAY_T23 38
#define GATEWAY_T24 39

#define GATEWAY_T25 40
#define GATEWAY_T26 41
#define GATEWAY_T27 42
#define GATEWAY_T28 43
#define GATEWAY_T29 44
#define GATEWAY_T30 45
#define GATEWAY_T31 46
#define GATEWAY_T32 47

#define GATEWAY_NONE 63

#define IFACETCP_MASK 0x30

/// коды ошибок верификации конфигурации
#define IFACE_MBMODE 73

#define IFACE_RTUPHYSPROT 13
#define IFACE_RTUSPEED 14
#define IFACE_RTUPARITY 15
#define IFACE_RTUTIMEOUT 16
#define IFACE_RTUTCPPORT 17

#define IFACE_TCPIP1 20
#define IFACE_TCPPORT1 21
#define IFACE_TCPUNITID 22
#define IFACE_TCPOFFSET 23
//#define IFACE_TCPMBADDR 24
#define IFACE_TCPTIMEOUT 24
#define IFACE_TCPIP2 25
#define IFACE_TCPPORT2 26
#define IFACE_TCPIPEQUAL 27

/// коды ошибок инициализации

#define IFACE_TTYINIT 89
#define IFACE_THREAD_INIT 93
#define IFACE_THREAD_STARTED 94
#define IFACE_THREAD_STOPPED 95
#define PROGRAM_TERMINATED 96

// интерфейсы шлюза

#define IFACE_TCPSERVER			6
#define IFACE_RTUMASTER			13
#define IFACE_RTUSLAVE			14

// значение связано с семафором
#define IFACE_MOXAGATE			0x0A

#define IFACE_TCPMASTER			15

// возможные другие состояния интерфейсов шлюза
#define IFACE_ERROR					16
#define IFACE_OFF						17
#define IFACE_MARGINAL			18

#define   NAME_MOXA_PORT           "PORT"
#define   NAME_MOXA_PORT_DEV  "/dev/ttyM"

#define TIMEOUT_MIN 100000
#define TIMEOUT_MAX 10000000

/// клиентское соединение шлюза и устройства TCP MASTER

#define TCP_RECONN_INTVL 4000000

#define TCPMSTCON_NOTMADE			0
#define TCPMSTCON_ESTABLISHED	1
#define TCPMSTCON_INPROGRESS	2
#define TCPMSTCON_NOTUSED			3

///=== INTERFACES_H data types

// параметры последовательного порта шлюза
///!!! не хватает четырех параметров: количества стоповых битов,
///    битов данных, метода контроля потока, режима RTU/ASCII
typedef struct {
	int fd;                // системный идентификатор
	char p_name[13];  // имя порта (устройства в системе)
	char p_mode[12];  // режим работы 232/485_2W/422/485_4W
	char speed[12];   // скорость обмена
	char parity[12];  // контроль четности

	unsigned int timeout;      // таймаут связи
	unsigned int ch_interval_timeout; ///!!!
	} GW_SerialIface;

// параметры TCP-порта шлюза
typedef struct {
	unsigned int ip;        // сетевой адрес
	unsigned int port;      // номер TCP-порта
  ///??? тип unsigned char вызывает здесь ошибки процесса выполнения (runtime):
  ///??? используем просто unsigned
	unsigned unit_id;  // адрес на который отвечает целевое устройство
	unsigned short offset;  /// стартовый регистр внутри целевого устройства
	unsigned mb_slave; // адрес modbus-устройства для перенаправления запросов (ATM) (вычисляемый)
	unsigned int timeout;      // таймаут связи
	unsigned int ip2;       // резервный сетевой адрес
	unsigned int port2;     // резервный номер TCP-порта

	int active_connection;
	int status;	// состояние клиентского соединения
	int status2;	// состояние клиентского соединения

	time_t	connection_time;		// время подключения
	time_t	connection_time2;
	time_t	disconnection_time;		// время отключения
	time_t	disconnection_time2;
	time_t	last_activity_time;		// время последнего запроса
	time_t	last_activity_time2;

	int csd;
	int csd2;

	struct timeval tv; // время последней попытки подключения

	} GW_TCPIface;

// параметры интерфейса шлюза (условно наследник GW_SerialIface и GW_SerialIface)
typedef struct {
  
  ///=== условно открытые переменные (public members)

  GW_Security Security;
	int modbus_mode;          // состояние интерфейса
	char bridge_status[4];    // обозначение состояния интерфейса для LCM
	char description[DEVICE_NAME_LENGTH]; // Описание шлейфа (сети ModBus)

	GW_SerialIface serial;
  GW_TCPIface ethernet;
						 
	GW_Queue queue;      // очередь запросов интерфейса
  GW_StaticData stat;  // структура данных со статистикой опроса

  // массив индексов по полю iface таблицы опроса
  unsigned char PQueryIndex[MAX_QUERY_ENTRIES+1];

  ///=== условно частные переменные (private members)

  // переменные режима GATEWAY_SIMPLE
	int ssd;                  // TCP-порт для приема входящих клиентских соединений
  pthread_mutex_t serial_mutex; // мьютекс - системная переменная для синхронизации потоков

  // переменные режима IFACE_RTUMASTER, IFACE_RTUSLAVE
	int rc;          // результат функции создания ассоциированного потока
	pthread_t tid_srvr; // системный идентификатор ассоциированного потока

	} GW_Iface;

///=== INTERFACES_H public variables

extern GW_Iface IfaceRTU[MAX_MOXA_PORTS];  // данные и параметры интерфейсов RTU
extern GW_Iface IfaceTCP[MAX_TCP_SERVERS]; // данные и параметры интерфейсов TCP

///=== INTERFACES_H public functions

int init_interfaces_h(); // условно конструктор

int check_Iface(GW_Iface *iface);

void *iface_tcp_server(void *arg); /// Потоковая функция режима GATEWAY_SIMPLE
void *iface_tcp_master(void *arg); /// Потоковая функция режима IFACE_TCPMASTER
void *iface_rtu_master(void *arg); /// Потоковая функция режима IFACE_RTUMASTER
void *iface_rtu_slave(void *arg); /// Потоковая функция режима IFACE_RTUSLAVE

int forward_response(int port_id, int client_id, u8 *req_adu, u16 req_adu_len, u8 *rsp_adu, u16 rsp_adu_len);

int reset_tcpmaster_conn(GW_Iface *tcp_master, int connum);
int check_tcpmaster_conn(GW_Iface *tcp_master, int connum);

#endif  /* INTERFACES_H */
