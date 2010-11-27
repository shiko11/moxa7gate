/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010                 
***************************************/

#ifndef CLIENTS_H
#define CLIENTS_H

///****************** РАБОТА С КЛИЕНТСКИМИ СОЕДИНЕНИЯМИ ************************

#include <sys/time.h>
#include <pthread.h>

#include "modbus.h"

///=== CLIENTS_H constants

#define MAX_TCP_CLIENTS_PER_PORT 4
#define MOXAGATE_CLIENTS_NUMBER 32
#define MOXAGATE_EXCEPTIONS_NUMBER 32

#define MAX_CLIENT_ACTIVITY_TIMEOUT 30

//#define DEFAULT_CLIENT 0
//#define MB_SLAVE_NOT_DEFINED			0xff

#define EXPT_STAGE_QUERY_RECV_RAW    1
#define EXPT_STAGE_QUERY_RECV        2
#define EXPT_STAGE_QUERY_FRWD        3
#define EXPT_STAGE_RESPONSE_RECV     4
#define EXPT_STAGE_RESPONSE_SEND     5
#define EXPT_STAGE_RESPONSE_RECV_RAW 6
#define EXPT_STAGE_UNDEFINED         7

///!!! эти константы не в том модуле:
#define MB_ADDRESS_NO_SHIFT				0
#define MB_SCAN_RATE_INFINITE			100000

/*--- Всего возможны 4 различных типа клиентов шлюза:
	1. Клиент на стороне TCP, подключенный к порту GATEWAY_SIMPLE;
	2. Клиент на стороне TCP, подключенный к шлюзу через общий TCP порт (обычно 502);
	3. Клиент на стороне RTU, подключенный к порту BRIDGE_PROXY;
	4. HMI клиент (выдает управляющие команды на запись внутренних регистров);
*/

// состояние клиентского соединения
#define GW_CLIENT_CLOSED			1
#define GW_CLIENT_ERROR				2
#define GW_CLIENT_TCP_GWS			3
#define GW_CLIENT_TCP_502			4
#define GW_CLIENT_RTU_SLV			5

// параметры, предназначенные для контроля и управления шлюзом
typedef struct {

	time_t start_time;  // время запуска

  char Object[DEVICE_NAME_LENGTH];        // наименование объекта автоматизации
  char Location[DEVICE_NAME_LENGTH];      // Место установки устройства MOXA UC-7410
  char Label[DEVICE_NAME_LENGTH];         //Обозначение по спецификации
  char NetworkName[DEVICE_NAME_LENGTH];   // Сетевое имя устройства
  unsigned int LAN1Address;               // сетевой адрес основной
  unsigned int LAN2Address;               // сетевой адрес резервный
  char VersionNumber[DEVICE_NAME_LENGTH]; // Версия конфигурационного файла
  char VersionTime[DEVICE_NAME_LENGTH];   // Время создания конфигурационного файла
  char Model[DEVICE_NAME_LENGTH];         // Модель устройства

  // Блок переменных SNMP, RFC1213-MIB
//  char sysDescr[64];     //.0	Uninitialized
//  char sysObjectID[64];  //.0	liebertGlobalProducts
//  char sysUpTime[64];    //.0	56 minutes, 25 seconds
//    char sysContact[64]; //.0	Uninitialized
//  char sysName[256];     //.0	nb214_ibp1_liebert_bottom
//  char sysLocation[64];  //.0	Uninitialized
//    char sysServices[64];//.0	72

  unsigned char show_data_flow;     // Показывать пакеты данных в сессии Telnet
  unsigned char show_sys_messages;	// Выводить в журнал отладочные сообщения
  unsigned char watchdog_timer;     // Использовать Watchdog-таймер
  unsigned char use_buzzer;         // Испльзовать зуммер для сигнализации

 	unsigned char halt; // флаг принудительного останова программы по команде

  unsigned int tcp_port;	// номер TCP-порта для приема входящих клиентских соединений

  unsigned int accepted_connections_number;
  unsigned int current_connections_number;
  unsigned int rejected_connections_number;

	} GW_Security;

typedef struct { // параметры клиентского устройства

	int status;	// состояние клиентского соединения
	unsigned char iface;		// номер интерфейса, соотнесенного с этой записью
	char device_name[DEVICE_NAME_LENGTH]; // наименование устройства (сетевое имя, DNS)

	unsigned int ip; 
	unsigned int port;

	time_t	connection_time;		// время подключения
	time_t	disconnection_time;		// время отключения
	time_t	last_activity_time;		// время последнего запроса

  // буферы для приема запросов и отправки ответов
  u8  inp_adu[MB_TCP_MAX_ADU_LENGTH];
  u16 inp_len;
  u8  out_adu[MB_TCP_MAX_ADU_LENGTH];
  u16 out_len;
  
	GW_StaticData stat;
	
  ///=== условно частные переменные (private members)
	int csd;								// TCP-сокет принятого
	int rc;									// результат функции создания потока (только режим GATEWAY_SIMPLE)
	pthread_t	tid_srvr; 		// идентификатор потока в системе (только режим GATEWAY_SIMPLE)

	} GW_Client;

typedef struct {
  ///!!! Необходимо разделять этапы при обработке исключений на до и после проверки на целостность пакета
	unsigned char stage; // Стадия прохождения запроса

  // Действие - это специальный алгоритм,\
     предназначенный для обработки очень специфичной ситуации.
	unsigned char action;

  // Параметр - это данные в любом формате, приводимые в итоге к
  // 32-разрядному числу при генерации конфигурации.
  unsigned int prm1;
  unsigned int prm2;
  unsigned int prm3;
  unsigned int prm4;

	char comment[DEVICE_NAME_LENGTH]; // комментарий
  } GW_Exception;

///=== CLIENTS_H public variables

	// переменная для контроля состояния объекта программы (модуля, механизма, потока)
  int clients_h_state;

  GW_Security Security;
	GW_Client	Client[MOXAGATE_CLIENTS_NUMBER];
  GW_Exception Exception[MOXAGATE_EXCEPTIONS_NUMBER];

  // obsolete
  // исключение для СКС-07, параметр - битовый массив номеров последовательных портов, к которым подключены диогены
  unsigned int exceptions; // obsolete // массив из 16 флагов
  unsigned int except_prm[16]; // obsolete // параметр исключения

///=== CLIENTS_H public functions
  int init_clients();     // условно конструктор
  int init_main_socket();
  int close_clients();    // условно деструктор
	int clear_client(int client);

  int gateway_common_processing();

#endif  /* CLIENTS_H */

/**** SNMP notes ***************************************************************
Name: Name to refer to the device. Your system administrator may use a specific naming convention. 
Note: The maximum length of the entry is 255 characters including spaces and punctuation. 
 
Contact: Person responsible for maintenance and operation of the device, which may be a network or facility administrator or the vendor from whom you purchased the device. 
Note: The maximum length of the entry is 64 characters including spaces and punctuation. 
 
Location: Description of the location of the device. 
Note: The maximum length of the entry is 64 characters including spaces and punctuation. 
 
Description: Other information useful for record keeping or quick reference. 
Note: The maximum length of the entry is 64 characters including spaces and punctuation but maybe longer depending on the device. 
 
Note: The values described above can be composed of printable characters with the exception of a double quote.  

********************************************************************************
	RFC1213-MIB	sysDescr.0	Uninitialized
RFC1213-MIB	sysObjectID.0	liebertGlobalProducts
RFC1213-MIB	sysUpTime.0	56 minutes, 25 seconds
	RFC1213-MIB	sysContact.0	Uninitialized
	RFC1213-MIB	sysName.0	nb214_ibp1_liebert_bottom
	RFC1213-MIB	sysLocation.0	Uninitialized
RFC1213-MIB	sysServices.0	72
*******************************************************************************/
