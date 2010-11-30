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

#include "frwd_queue.h"
#include "clients.h"

///=== MOXAGATE_H constants

#define	BASIC_STAT_GATEWAY_INFO 24
#define	GATE_STATUS_BLOCK_LENGTH BASIC_STAT_GATEWAY_INFO+MAX_QUERY_ENTRIES/16

///=== MOXAGATE_H data types

typedef struct {

	time_t start_time;  // время запуска
  unsigned char modbus_address; // собственный Modbus-адрес шлюза для диагностики и управления средствами HMI систем
  ///!!! реализовать блок данных диагностики отдельным массивом памяти, не связанным с указателем wData4x
  unsigned short status_info; // стартовый адрес блока собственных регистров шлюза (по умолчанию начинается с первого)

	GW_Queue queue;
  GW_StaticData stat;

  unsigned char map_2x_to_4x;			// Режим отображения таблицы дискретных входов на таблицу holding-регистров

  // мьютекс используется для синхронизации потоков при работе с памятью
  pthread_mutex_t moxa_mutex;
	// начало блока внутренних регистров Moxa (смещение)
	unsigned short offset1xStatus, offset2xStatus, offset3xRegisters, offset4xRegisters;
	// количество элементов в каждой из таблиц MODBUS
	unsigned short amount1xStatus, amount2xStatus, amount3xRegisters, amount4xRegisters;
	// указатели на массивы памяти
	unsigned char *wData1x; //массив виртуальных 1x регистров MOXA (coil status)
	unsigned char *wData2x; //массив виртуальных 2x регистров MOXA (input status)
	unsigned short *wData3x; //массив виртуальных 3x регистров MOXA (input register)
	unsigned short *wData4x; //массив виртуальных 4x регистров MOXA (holding register)
	
	} GW_MoxaDevice;

///=== MOXAGATE_H public variables

  GW_MoxaDevice MoxaDevice; // данные и параметры устройства MOXAGATE

///=== MOXAGATE_H public functions

int init_moxagate_h();

void *moxa_device(void *arg); /// Потоковая функция обработки запросов к MOXA

#endif  /* MOXAGATE_H */
