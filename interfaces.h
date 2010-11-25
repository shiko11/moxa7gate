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

#include "frwd_queue.h"
#include "clients.h"

///=== INTERFACES_H constants

#define   MAX_MOXA_PORTS 8

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

#define MAX_TCP_SERVERS 32

#define EXCEPTION_DIOGEN 0x00000001

///=== INTERFACES_H data types

// параметры последовательного порта шлюза
///!!! не хватает четырех параметров: количества стоповых битов,
///    битов данных, метода контроля потока, режима RTU/ASCII
typedef struct {
	int fd;                // системный идентификатор
	char p_name[12];  // имя порта (устройства в системе)
	char p_mode[12];  // режим работы 232/485_2W/422/485_4W
	char speed[12];   // скорость обмена
	char parity[12];  // контроль четности

	int timeout;      // таймаут связи
	unsigned int ch_interval_timeout; ///!!!
	} GW_SerialIface;

// параметры TCP-порта шлюза
typedef struct {
	unsigned char mb_slave; // адрес modbus-устройства для перенаправления запросов (ATM)
	unsigned short offset;      /// стартовый регистр внутри целевого устройства
	unsigned int ip;        // сетевой адрес
	unsigned int port;      // номер TCP-порта
	unsigned int ip2;       // резервный сетевой адрес
	unsigned int port2;     // резервный номер TCP-порта
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

  ///=== условно частные переменные (private members)

  // переменные режима GATEWAY_SIMPLE
	int ssd;                  // TCP-порт для приема входящих клиентских соединений
  pthread_mutex_t serial_mutex; // мьютекс - системная переменная для синхронизации потоков

  // переменные режима IFACE_RTUMASTER, IFACE_RTUSLAVE
	int rc;          // результат функции создания ассоциированного потока
	pthread_t tid_srvr; // системный идентификатор ассоциированного потока

	} GW_Iface;

///=== INTERFACES_H public variables

GW_Iface IfaceRTU[MAX_MOXA_PORTS];  // данные и параметры интерфейсов RTU
GW_Iface IfaceTCP[MAX_TCP_SERVERS]; // данные и параметры интерфейсов TCP

//GW_TCPIface tcp_servers[MAX_TCP_SERVERS]; ///!!!

// массив исключительных ситуаций служит для устранения проблем при обмене, вызванных особенностями
// конечных устройств modbus-slave. он содержит набор флагов, включающих определенные алгоритмы в
// определенных ситуациях.

///=== INTERFACES_H public functions

void *iface_rtu_gws(void *arg); /// Потоковая функция режима GATEWAY_SIMPLE
void *srvr_tcp_child2(void *arg); ///!!! Потоковая функция режимов GATEWAY_ATM, GATEWAY_RTM
void *iface_tcp_master(void *arg); /// Потоковая функция режима IFACE_TCPMASTER
void *iface_rtu_master(void *arg); /// Потоковая функция режима IFACE_RTUMASTER
void *iface_rtu_slave(void *arg); /// Потоковая функция режима IFACE_RTUSLAVE

#endif  /* INTERFACES_H */
