/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010                 
***************************************/

#ifndef FRWD_QUEUE_H
#define FRWD_QUEUE_H

///*********  МОДУЛЬ ФУНКЦИЙ ПЕРЕНАПРАВЛЕНИЯ, ОЧЕРЕДЕЙ ЗАПРОСОВ MODBUS *********

#include <sys/sem.h>

#include "modbus.h"

///=== FRWD_QUEUE_H constants

// конфигурационные константы времени компиляции

#define   MAX_GATEWAY_QUEUE_LENGTH 16

#define MOXA_DIAPASON_UNDEFINED		0
#define MOXA_DIAPASON_INSIDE			1
#define MOXA_DIAPASON_OUTSIDE			2
#define MOXA_DIAPASON_OVERLAPPED	3

#define QT_ACCESS_DISABLED  0
#define QT_ACCESS_READONLY  1
#define QT_ACCESS_READWRITE 2

#define QT_DELAY_RTU_MIN 0
#define QT_DELAY_RTU_MAX 10000
#define QT_DELAY_TCP_MIN 100
#define QT_DELAY_TCP_MAX 10000

#define QT_CRITICAL_MAX 128

#define MAX_VIRTUAL_SLAVES 128
#define MAX_QUERY_ENTRIES 128
#define MOXAGATE_EXCEPTIONS_NUMBER 32

#define EXPT_STAGE_UNDEFINED         0
#define EXPT_STAGE_QUERY_RECV_RAW    1
#define EXPT_STAGE_QUERY_RECV        2
#define EXPT_STAGE_QUERY_FRWD        3
#define EXPT_STAGE_RESPONSE_RECV_RAW 4
#define EXPT_STAGE_RESPONSE_RECV     5
#define EXPT_STAGE_RESPONSE_SEND     6

#define EXPT_ACT_NONE 0x00
#define EXPT_ACT_SKS07_DIOGEN 0x01

/// коды ошибок верификации конфигурации
#define ATM_IFACE 30
#define ATM_MBADDR 31

#define VSLAVE_IFACE 76
#define VSLAVE_MBADDR 35
#define VSLAVE_MBTABL 36
#define VSLAVE_BEGDIAP 37
#define VSLAVE_ENDDIAP 38
#define VSLAVE_LENDIAP 39

#define PQUERY_IFACE 74
#define PQUERY_MBADDR 43
#define PQUERY_MBTABL 44
#define PQUERY_ACCESS 45
#define PQUERY_ENDREGREAD 46
#define PQUERY_LENPACKET 47
#define PQUERY_ENDREGWRITE 48
#define PQUERY_DELAYMIN 49
#define PQUERY_DELAYMAX 50
#define PQUERY_ERRCNTR 51

#define EXPT_STAGE 60
#define EXPT_ACTION 55
#define EXPT_PRM1 56
#define EXPT_PRM2 57
#define EXPT_PRM3 58
#define EXPT_PRM4 59

///=== FRWD_QUEUE_H data types

typedef struct { // очередь на семафорах
	int port_id;

	unsigned char			queue_adu[MAX_GATEWAY_QUEUE_LENGTH][MB_TCP_MAX_ADU_LENGTH];// TCP ADU
	unsigned short		queue_adu_len[MAX_GATEWAY_QUEUE_LENGTH];
	unsigned short		queue_clients[MAX_GATEWAY_QUEUE_LENGTH];
	unsigned short		queue_slaves[MAX_GATEWAY_QUEUE_LENGTH];
	int			queue_start, queue_len;

  pthread_mutex_t queue_mutex;
	struct sembuf operations[1];
	} GW_Queue;

typedef struct {
  unsigned char iface;
  unsigned char address;
  } GW_AddressMap_Entry;

/// пока определия этих блоков адресов относится к Holding-регистрам modbus и функции 06
typedef struct {			// блок регистров внутреннего адресного пространства шлюза
	unsigned char iface;			// последовательный порт шлюза для перенаправления запроса
	unsigned device;		// адрес устройства в сети modbus для перенаправления запроса
  ///??? тип unsigned char вызывает здесь run-time level ошибки, поэтому используем unsigned
  unsigned modbus_table; // одна из 4-х стандартных таблиц протокола MODBUS (см. файл modbus_rtu.h)
  unsigned short offset;  // смещение в адресном пространстве устройства modbus (чтобы читать регистры не с нуля)

	unsigned short start;			// начальный регистр диапазона адресного пространства шлюза
	unsigned short length;		// количество регистров дипазона адресного пространства шлюза

	char device_name[DEVICE_NAME_LENGTH]; // наименование устройства
  } RT_Table_Entry; // GW_VSlave_Entry

typedef struct {			// modbus-запрос для циклического опроса ведомого устройства в режиме PROXY
	unsigned char iface;		// интерфейс шлюза для опроса ведомого устройства
	unsigned char device;		// адрес ведомого устройства в сети modbus
  unsigned char mbf; 			// одна из стандартных функций протокола MODBUS (см. файл modbus_rtu.h)
  unsigned char access;  // режим доступа к блоку данных: только чтение, чтение/запись, отключено

	unsigned short  start;			// начальный регистр диапазона адресного пространства ведомого устройства
	unsigned short length;		// количество регистров в диапазоне адресов ведомого устройства
  unsigned short offset;  // номер регистра (смещение) в адресном пространстве шлюза, с которого запивывать полученные данные

  unsigned delay;  // задержка перед отправкой запроса при работе в режиме PROXY в милисекундах
	unsigned critical;			// количество ошибок до изменения бита статуса связи
	char device_name[DEVICE_NAME_LENGTH]; // наименование устройства

  unsigned err_counter;  // счетчик текущего количества ошибок
  unsigned status_bit;  // бит статуса связи (первый в переменной)
  } Query_Table_Entry; // GW_ProxyQuery_Entry

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

///=== FRWD_QUEUE_H public variables

// Набор семафоров. Используется для синхронизации работы потоков с очередями.
int semaphore_id;

GW_AddressMap_Entry AddressMap[MODBUS_ADDRESS_MAX+1]; // нумерация с единицы
RT_Table_Entry      VSlave[MAX_VIRTUAL_SLAVES];
Query_Table_Entry   PQuery[MAX_QUERY_ENTRIES];
GW_Exception        Exception[MOXAGATE_EXCEPTIONS_NUMBER];

unsigned short vsmem_offset1xStatus, vsmem_offset2xStatus, vsmem_offset3xRegisters, vsmem_offset4xRegisters;
unsigned short vsmem_amount1xStatus, vsmem_amount2xStatus, vsmem_amount3xRegisters, vsmem_amount4xRegisters;
unsigned short vsmem_used1xStatus, vsmem_used2xStatus, vsmem_used3xRegisters, vsmem_used4xRegisters;

// массив исключительных ситуаций служит для устранения проблем при обмене, вызванных особенностями
// конечных устройств modbus-slave. он содержит набор флагов, включающих определенные алгоритмы в
// определенных ситуациях.

// obsolete
// исключение для СКС-07, параметр - битовый массив номеров последовательных портов, к которым подключены диогены
unsigned int exceptions; // obsolete // массив из 16 флагов
unsigned int except_prm[16]; // obsolete // параметр исключения

///=== FRWD_QUEUE_H public functions

int init_frwd_queue_h(); // инициализация переменных

int init_AddressMap_Entry(int index);
int check_AddressMap_Entry(int index);

int init_Vslave_Entry(int index);
int check_Vslave_Entry(int index);

int init_ProxyQuery_Entry(int index);
int check_ProxyQuery_Entry(int index);

int init_Exception(int index);
int check_Exception(int index);

int init_queue(); // инициализация семафоров
int enqueue_query_ex(GW_Queue *queue, int client_id, int device_id, u8 *adu, u16 adu_len);
int get_query_from_queue(GW_Queue *queue, int *client_id, int *device_id, u8 *adu, u16 *adu_len);

int checkDiapason(int function, int start_address, int length);
int process_moxamb_request(int client_id, u8 *adu, u16 adu_len, u8 *memory_adu, u16 *memory_adu_len);

#endif  /* FRWD_QUEUE_H */
