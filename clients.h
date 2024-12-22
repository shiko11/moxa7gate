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

// конфигурационные константы времени компиляции

///!!! эта константа принадлежит модулю интерфейсов
#define MAX_TCP_SERVERS 32

#define MAX_TCP_CLIENTS_PER_PORT 4
#define MOXAGATE_CLIENTS_NUMBER 32

#define MAX_CLIENT_ACTIVITY_TIMEOUT 30

//#define DEFAULT_CLIENT 0
//#define MB_SLAVE_NOT_DEFINED			0xff

#define TCP_PORT_MIN 1
#define TCP_PORT_MAX 0xFFFF

/// коды ошибок верификации конфигурации
#define SECURITY_TCPPORT 75
#define MOXAGATE_MBADDR 8
#define MOXAGATE_STATINFO 9

#define MOXAGATE_STATINFO_OVERLAPS 86
#define MOXAGATE_MBTABLES_OVERLAPS 87

/// коды ошибок инициализации
#define MOXAGATE_MBTABLE_ALLOCATED 88

#define TCPCON_INITIALIZED 90
#define TCPCON_ACCEPTED 91
#define TCPCON_REJECTED 92

#define TCPCON_CLOSED_LINK	105
#define TCPCON_CLOSED_TIME	106
#define TCPCON_CLOSED_KPALV 122
#define TCPCON_CLOSED_REMSD 123
#define TCPCON_CLOSED_TMOUT 124
#define TCPCON_ESTABLISHED	120
#define TCPCON_FAILED				121

#define CLIENT_NOTAVAIL 101

/*--- Всего возможны 4 различных типа клиентов шлюза:
	1. Клиент на стороне TCP, подключенный к порту IFACE_TCPSERVER;
	2. Клиент на стороне TCP, подключенный к шлюзу через основной TCP порт (обычно 502);
	3. Клиент на стороне RTU, подключенный к порту IFACE_RTUSLAVE;
	4. HMI клиент (пользователь, выдает управляющие команды на изменение конфигурации);
*/

// состояние клиентского соединения
#define GW_CLIENT_IDLE				1
#define GW_CLIENT_ERROR				2
#define GW_CLIENT_TCP_GWS			3
#define GW_CLIENT_TCP_502			4
#define GW_CLIENT_RTU_SLV			5
#define GW_CLIENT_MOXAGATE		6 // используется при обработке PROXY-запросов
#define GW_CLIENT_KM400				7

// параметры, предназначенные для контроля и управления шлюзом
typedef struct {

	time_t start_time;  // время запуска шлюза

  char Object[DEVICE_NAME_LENGTH];        // наименование объекта автоматизации
  char Location[DEVICE_NAME_LENGTH];      // Место установки устройства MOXA UC-7410
  char Label[DEVICE_NAME_LENGTH];         //Обозначение по спецификации
  char NetworkName[DEVICE_NAME_LENGTH];   // Сетевое имя устройства
  unsigned int LAN1Address;               // сетевой адрес основной
  unsigned int LAN2Address;               // сетевой адрес резервный
  char VersionNumber[DEVICE_NAME_LENGTH]; // Версия конфигурационного файла
  char VersionTime[DEVICE_NAME_LENGTH];   // Время создания конфигурационного файла
  char Model[DEVICE_NAME_LENGTH];         // Модель устройства

  unsigned char show_data_flow;     // Показывать пакеты данных в сессии Telnet
  unsigned char show_sys_messages;	// Выводить в журнал отладочные сообщения
  unsigned char watchdog_timer;     // Использовать Watchdog-таймер
  unsigned char use_buzzer;         // Испльзовать зуммер для сигнализации

  ///!!!
	time_t timestamp;  // время последнего обновления данных для web-интерфейса
 	unsigned char halt; // флаг принудительного останова программы по команде
  unsigned int back_light; // флаг включения подсветки дисплея устройства MOXA UC-7410

  unsigned int tcp_port;	// номер TCP-порта для приема входящих клиентских соединений

  unsigned int accepted_connections_number;
  unsigned int current_connections_number;
  unsigned int rejected_connections_number;

  GW_StaticData stat; // подсчет статистики при приеме и перенаправлении от клиентов GW_CLIENT_TCP_502

  // массив индексов сконфигурированных TCP-интерфейсов
  unsigned char TCPIndex[MAX_TCP_SERVERS+1];
  // массив индексов сконфигурированных интерфейсов типа IFACE_TCPSERVER
  unsigned char TCPSRVIndex[MAX_MOXA_PORTS+1];

	// счетчик циклов сканирования главного потока
	unsigned int scan_counter;

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

///=== CLIENTS_H public variables

extern GW_Security Security;
extern GW_Client   Client[MOXAGATE_CLIENTS_NUMBER];

///=== CLIENTS_H public functions

  int init_clients();     // условно конструктор
  int close_clients();    // условно деструктор

  int check_Security();
  int init_main_socket();
	int clear_client(int client);

  int gateway_common_processing();
  int get_current_client();

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

  // Блок переменных SNMP, RFC1213-MIB
  char sysDescr[64];     //.0	Uninitialized
  char sysObjectID[64];  //.0	liebertGlobalProducts
  char sysUpTime[64];    //.0	56 minutes, 25 seconds
    char sysContact[64]; //.0	Uninitialized
  char sysName[256];     //.0	nb214_ibp1_liebert_bottom
  char sysLocation[64];  //.0	Uninitialized
    char sysServices[64];//.0	72

*******************************************************************************/
