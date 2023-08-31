/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010                 
***************************************/

#ifndef MOXAGATE_H
#define MOXAGATE_H

///*********************** МОДУЛЬ УСТРОЙСТВА MODBUS MOXA UC-7410 ***************
///*** МЕХАНИЗМ ВНУТРЕННЕЙ ПАМЯТИ
///*** СТРУКТУРЫ ДАННЫХ ДЛЯ РЕАЛИЗАЦИИ ФУНКЦИЙ HMI-ВЗАИМОДЕЙСТВИЯ

#include "forwarding.h"
#include "clients.h"

///=== MOXAGATE_H constants

// конфигурационные константы времени компиляции

#define GWINF_STATE_IFACERTU 1-1
#define GWINF_STATE_IFACETCP 9-1
#define GWINF_STATE_CLIENTS  41-1

#define GWINF_PROXY_STATUS   73-1

#define GWINF_SCAN_COUNTER   81-1

#define GWINF_CONSTAT_M7G    82-1
#define GWINF_CONSTAT_RTU    85-1

#define GWINF_STATDETAILS_1  109-1
#define GWINF_STATDETAILS_2  173-1
#define GWINF_STATDETAILS_3  237-1
#define GWINF_STATDETAILS_4  301-1

#define GWINF_STAT_RTU       365-1
#define GWINF_STAT_TCP       461-1
#define GWINF_STAT_CLIENTS   845-1
#define GWINF_STAT_MOXAGATE  1229-1

#define	GATE_STATUS_BLOCK_LENGTH 1241

#define MOXAGATE_MAX_MEMORY_LEAK 1024

#define MOXAGATE_WATCHDOG_PERIOD 60

// коды ошибок инициализации
#define MOXAGATE_WATCHDOG_STARTED 130

///=== MOXAGATE_H data types

typedef struct {

	time_t start_time;  // время запуска
  unsigned char modbus_address; // собственный Modbus-адрес шлюза для диагностики и управления средствами HMI систем
  unsigned short status_info; // стартовый адрес блока собственных регистров шлюза (по умолчанию начинается с первого)

	GW_Queue queue;
  GW_StaticData stat;

  unsigned char map2Xto4X;			// Режим отображения таблицы дискретных входов на таблицу holding-регистров

  // мьютекс используется для синхронизации потоков при работе с памятью
  pthread_mutex_t moxa_mutex;
	// начало блока внутренних регистров Moxa (смещение)
	unsigned short offset1xStatus, offset2xStatus, offset3xRegisters, offset4xRegisters;
	// количество элементов в каждой из таблиц MODBUS
	unsigned short amount1xStatus, amount2xStatus, amount3xRegisters, amount4xRegisters;
  // счетчики реально используемой памяти всеми блоками
  unsigned short used1xStatus, used2xStatus, used3xRegisters, used4xRegisters;
	// указатели на массивы памяти
	unsigned char *wData1x; //массив виртуальных 1x регистров MOXA (coil status)
	unsigned char *wData2x; //массив виртуальных 2x регистров MOXA (input status)
	unsigned short *wData3x; //массив виртуальных 3x регистров MOXA (input register)
	unsigned short *wData4x; //массив виртуальных 4x регистров MOXA (holding register)
	
	int mxwdt_handle;

	} GW_MoxaDevice;

///=== MOXAGATE_H public variables

extern GW_MoxaDevice MoxaDevice; // данные и параметры устройства MOXAGATE

///=== MOXAGATE_H public functions

int init_moxagate_h();
int init_moxagate_memory();

void *moxa_device(void *arg); /// Потоковая функция обработки запросов к MOXA

int refresh_status_info(); // обновление динамических данных в блоке диагностики шлюза

void create_proxy_request(int index, u8 *tcp_adu, u16 *tcp_adu_len);  // формирование запроса на основе записи из таблицы опроса
void process_proxy_response(int index, u8 *tcp_adu, u16 tcp_adu_len); // обработка ответа на запрос из таблицы опроса
void make_tcp_adu(u8 *tcp_adu, int length); // подготовка новой TCP ADU для отправки на TCP сервер

#endif  /* MOXAGATE_H */
