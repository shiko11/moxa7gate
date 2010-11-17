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

#include "queue.h"
#include "clients.h"

///=== INTERFACES_H constants

#define   MAX_MOXA_PORTS		8

#define   NAME_MOXA_PORT           "PORT"
#define   NAME_MOXA_PORT_DEV  "/dev/ttyM"
#define SERIAL_P1	0
#define SERIAL_P2	1
#define SERIAL_P3	2
#define SERIAL_P4	3
#define SERIAL_P5	4
#define SERIAL_P6	5
#define SERIAL_P7	6
#define SERIAL_P8	7

#define PROXY_TCP	8

#define SERIAL_STUB 0xff

// режим применим только для serial-интерфейса шлюза
// ввиду его простоты и надежности он остается прежним
#define GATEWAY_SIMPLE				6
#define IFACE_GATEWAY_SIMPLE	6

///!!! эти определения режимов портов устарели, они сохраняют свой смысл для реализации механизма перенаправления
#define GATEWAY_ATM					7
#define GATEWAY_RTM					8
#define GATEWAY_PROXY				9
// вместо трех предыдущих добавляем один:
#define IFACE_RTUMASTER			13

// на уровне ATM процесса перенаправления находится устройство MOXA
// оно предназначено для реализации в первую очередь HMI-функций шлюза
// то есть взаимодействия с человеком
#define IFACE_MOXAGATE			0x0A
//#define EVENT_SRC_MOXAMB	0x0A
// это число исп. также для идентификации семафора!
#define   MOXA_MB_DEVICE		0x0A

///!!! эти определения режимов портов устарели
#define BRIDGE_TCP					11
#define BRIDGE_PROXY				12
// вместо двух предыдущих добавляем один:
#define IFACE_RTUSLAVE			14

// добавляем также новый тип интерфейса шлюза: TCP-интерфейс
#define IFACE_TCPMASTER			15

// возможные другие состояния интерфейсов шлюза
#define MODBUS_PORT_ERROR		4
#define MODBUS_PORT_OFF			5
#define IFACE_ERROR					16
#define IFACE_OFF						17

#define MAX_TCP_SERVERS 		32

///=== INTERFACES_H data types

typedef struct { // параметры последовательного порта
	int				fd;						// идентификатор
	unsigned char        p_num;				// номер порта MOXA
	char      p_name[12];		// имя порта (устройства в системе)
	char      p_mode[12];		// режим 232/485_2W/422/485_4W
	char			speed[12];		// скорость обмена
	char			parity[12];		// контроль четности
	int		timeout;					// таймаут связи

	int ch_interval_timeout;
	} GW_SerialLine;

typedef struct {
	GW_SerialLine serial;

	unsigned short				tcp_port;				// номер TCP-порта для приема входящих клиентских соединений
	int				ssd;						// TCP-порт для приема входящих клиентских соединений
	int current_client;				// номер свободного соединения
	char bridge_status[4];		// состояние шлюза
	int modbus_mode;  				// режим шлюза

	GW_Client	clients[MAX_TCP_CLIENTS_PER_PORT];
  GW_StaticData stat;
	time_t start_time;  // время последней инициализации порта

  unsigned int accepted_connections_number;
  unsigned int  current_connections_number;
  unsigned int rejected_connections_number;
  
  pthread_mutex_t serial_mutex;

//	u8			queue_adu[MAX_GATEWAY_QUEUE_LENGTH][MB_TCP_MAX_ADU_LENGTH];// TCP ADU
//	u16			queue_adu_len[MAX_GATEWAY_QUEUE_LENGTH];
//	u16			queue_clients[MAX_GATEWAY_QUEUE_LENGTH];
//	u16			queue_slaves[MAX_GATEWAY_QUEUE_LENGTH];
//	int			queue_start, queue_len;

	GW_Queue queue;

	char description[DEVICE_NAME_LENGTH]; // Описание шлейфа (сети ModBus)
	} input_cfg;

typedef struct { // запись таблицы TCP_SERVERS
	unsigned char        mb_slave;					// адрес modbus-устройства для перенаправления запросов (ATM)
	unsigned int ip;						// сетевой адрес
	unsigned int port;					// TCP порт
	int address_shift;					// смещение адресного пространства
	unsigned char        p_num;						// obsolete
	char device_name[DEVICE_NAME_LENGTH]; // наименование устройства
	} GW_TCP_Server;

///=== INTERFACES_H public variables

input_cfg iDATA[MAX_MOXA_PORTS];     // данные и параметры интерфейсов RTU
input_cfg iDATAtcp[MAX_TCP_SERVERS]; // данные и параметры интерфейсов TCP
GW_TCP_Server tcp_servers[MAX_TCP_SERVERS];

///=== INTERFACES_H public functions

void *srvr_tcp_child(void *arg); /// Потоковая функция режима GATEWAY_SIMPLE
void *srvr_tcp_child2(void *arg); /// Потоковая функция режимов GATEWAY_ATM, GATEWAY_RTM
void *srvr_tcp_bridge(void *arg); /// Потоковая функция режима BRIDGE_TCP
void *gateway_proxy_thread(void *arg); /// Потоковая функция режима GATEWAY_PROXY
void *bridge_proxy_thread(void *arg); /// Потоковая функция режима BRIDGE_PROXY

#endif  /* INTERFACES_H */
