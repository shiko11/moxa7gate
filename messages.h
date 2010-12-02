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

#define EVENT_LOG_LENGTH 100
#define EVENT_MESSAGE_LENGTH 81
#define EVENT_TEMPLATE_AMOUNT 0x100

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
#define EVENT_TYPE_RESERV     0x00
#define EVENT_TYPE_MASK       0x30

#define EVENT_SRC_MASK        0x0F

/*//!!! Глобальные идентификаторы объектов moxa7gate:

ИНТЕРФЕЙСОВ
  - 9 физических последовательных, возможны 3 режима работы:
     - IFACE_GATEWAY_SIMPLE
     - IFACE_RTU_MASTER
     - IFACE_RTU_SLAVE
  - 2 физических сетевых Ethernet
  - 32 логических IFACE_TCP_MASTER

МОДУЛЕЙ

  - CLI_H
  - MODBUS_H
  - MAIN_H

* - CLIENTS_H
* - FRWD_QUEUE_H
  - INTERFACES_H
* - MOXAGATE_H

  - MESSAGES_H
  - STATISTICS_H
  - HMI_KEYPAD_LCM_H
  - HMI_WEB_H

КЛИЕНТОВ
  - 32 логических клиента шлюза, возможны следующие варианты:
    - GW_CLIENT_TCP_GWS
    - GW_CLIENT_TCP_502
    - GW_CLIENT_RTU_SLV

*/

#define TRAFFIC_RTU_RECV	182
#define TRAFFIC_RTU_SEND	183
#define TRAFFIC_TCP_RECV	184
#define TRAFFIC_TCP_SEND	185
#define TRAFFIC_QUEUE_IN	186
#define TRAFFIC_QUEUE_OUT	187

///=== MESSAGES_H data types

typedef struct {				// ЗАПИСЬ ЖУРНАЛА СОБЫТИЙ ШЛЮЗА
	time_t	time;					// время возникновения события

//	unsigned int source;  // источник (аппаратное устройство или объект программы)
//	unsigned int code;		// дополнительная информация о событии (например, код ошибки)
//	char desc[EVENT_MESSAGE_LENGTH]; // proverit' dlinu soobscheniya functsii perror()

	unsigned char msgtype;
	unsigned char msgcode;
	unsigned int prm[4];
	} GW_EventLog;	// GW_Event	 // текст сообщения

///=== MESSAGES_H public variables

  GW_EventLog *app_log; ///!!! Нужно оформить эту переменную отдельной структурой

	unsigned app_log_current_entry, app_log_entries_total;
	unsigned msg_filter;

///=== MESSAGES_H public functions

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
