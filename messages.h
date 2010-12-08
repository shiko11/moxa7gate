/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010                 
***************************************/

#ifndef MESSAGES_H
#define MESSAGES_H

///******************* МОДУЛЬ ЖУРНАЛА СООБЩЕНИЙ ********************************

#include <sys/time.h>

#include "modbus.h"

///=== MESSAGES_H constants

#define EVENT_LOG_LENGTH 100        // длина журнала сообщений
#define EVENT_MESSAGE_LENGTH 65     // в связи с размерами экрана LCM выбираем такое значение
#define EVENT_TEMPLATE_AMOUNT 0x100 // количество сообщений по каждому из объектов

/// Типы событий, возникающих в процессе работы:
//    - внешние (monitor)
//    - внутренние (debug)

#define EVENT_CAT_MONITOR     0x40
#define EVENT_CAT_DEBUG       0x80
#define EVENT_CAT_RESERVE1    0xC0
#define EVENT_CAT_RESERVE2    0x00
#define EVENT_CAT_MASK        0xC0

#define EVENT_TYPE_INF        0x10
#define EVENT_TYPE_WRN        0x20
#define EVENT_TYPE_ERR        0x30
#define EVENT_TYPE_FTL        0x00
#define EVENT_TYPE_MASK       0x30

#define EVENT_SRC_MASK        0x0F

///!!!
#define TRAFFIC_RTU_RECV	182
#define TRAFFIC_RTU_SEND	183
#define TRAFFIC_TCP_RECV	184
#define TRAFFIC_TCP_SEND	185
#define TRAFFIC_QUEUE_IN	186
#define TRAFFIC_QUEUE_OUT	187

/// типы сообщений по комбинациям параметров
#define EVENT_TPL_DEFAULT 0

#define EVENT_TPL_000D 0x01
#define EVENT_TPL_00DD 0x03
#define EVENT_TPL_000S 0x10

#define EVENT_TPL_D1_MASK 0x01
#define EVENT_TPL_D2_MASK 0x02
#define EVENT_TPL_D3_MASK 0x04
#define EVENT_TPL_D4_MASK 0x08

#define EVENT_TPL_S1_MASK 0x10
#define EVENT_TPL_S2_MASK 0x20
#define EVENT_TPL_S3_MASK 0x40
#define EVENT_TPL_S4_MASK 0x80

#define EVENT_TPL_NOPARAM 0x100

///=== MESSAGES_H data types

typedef struct {				// ЗАПИСЬ ЖУРНАЛА СОБЫТИЙ ШЛЮЗА
	time_t	time;					// время возникновения события

//	unsigned int source;  // источник (аппаратное устройство или объект программы)
//	unsigned int code;		// дополнительная информация о событии (например, код ошибки)
//	char desc[EVENT_MESSAGE_LENGTH]; // proverit' dlinu soobscheniya functsii perror()

	unsigned char msgtype;
	unsigned char msgcode;
	unsigned int prm[4];
	} GW_EventLog;	// GW_Event

///=== MESSAGES_H public variables

  GW_EventLog *app_log;

	unsigned int app_log_current_entry, app_log_entries_total;
	unsigned int msg_filter;

///=== MESSAGES_H public functions

// условно конструктор
int init_messages_h();

void sysmsg_ex(unsigned char msgtype, unsigned char msgcode,
								unsigned int prm1,
								unsigned int prm2,
								unsigned int prm3,
								unsigned int prm4);

void show_traffic(int traffic, int port_id, int client_id, u8 *adu, u16 adu_len);

void make_msgstr(	unsigned char msgcode, char *str,
									unsigned int prm1,
									unsigned int prm2,
									unsigned int prm3,
									unsigned int prm4);

///*************************************************************************************

#endif  /* MESSAGES_H */
