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

#define   MAX_GATEWAY_QUEUE_LENGTH 16

#define MOXA_DIAPASON_UNDEFINED		0
#define MOXA_DIAPASON_INSIDE			1
#define MOXA_DIAPASON_OUTSIDE			2
#define MOXA_DIAPASON_OVERLAPPED	3

#define QT_ACCESS_READONLY  1
#define QT_ACCESS_READWRITE 2
#define QT_ACCESS_DISABLED  3

#define MAX_VIRTUAL_SLAVES 128
#define MAX_QUERY_ENTRIES 128

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

/// пока определия этих блоков адресов относится к Holding-регистрам modbus и функции 06
typedef struct {			// блок регистров внутреннего адресного пространства шлюза
	unsigned short start;			// начальный регистр диапазона адресного пространства шлюза
	unsigned short length;		// количество регистров дипазона адресного пространства шлюза
	unsigned char iface;			// последовательный порт шлюза для перенаправления запроса
	unsigned device;		// адрес устройства в сети modbus для перенаправления запроса

  unsigned char modbus_table; // одна из 4-х стандартных таблиц протокола MODBUS (см. файл modbus_rtu.h)
  unsigned short offset;  // смещение в адресном пространстве устройства modbus (чтобы читать регистры не с нуля)
	char device_name[DEVICE_NAME_LENGTH]; // наименование устройства
  } RT_Table_Entry; // GW_RTM_Table_Entry

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
  } Query_Table_Entry; // GW_Query_Table_Entry

///=== FRWD_QUEUE_H public variables

// Набор семафоров. Используется для синхронизации работы потоков с очередями.
int semaphore_id;

unsigned char atm_map[MODBUS_ADDRESS_MAX+1]; // iface type enumeration
RT_Table_Entry vslave[MAX_VIRTUAL_SLAVES];
Query_Table_Entry query_table[MAX_QUERY_ENTRIES];

///=== FRWD_QUEUE_H public functions

int init_queue();
int enqueue_query_ex(GW_Queue *queue, int client_id, int device_id, u8 *adu, u16 adu_len);
int get_query_from_queue(GW_Queue *queue, int *client_id, int *device_id, u8 *adu, u16 *adu_len);

int checkDiapason(int function, int start_address, int length);
int process_moxamb_request(int client_id, u8 *adu, u16 adu_len, u8 *memory_adu, u16 *memory_adu_len);

#endif  /* FRWD_QUEUE_H */
