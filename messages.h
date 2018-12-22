/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010                 
***************************************/

#ifndef MESSAGES_H
#define MESSAGES_H

///******************* ������ ������� ��������� ********************************

#include <sys/time.h>

#include "modbus.h"

///=== MESSAGES_H constants

#define EVENT_LOG_LENGTH 100
#define EVENT_MESSAGE_LENGTH 81
#define EVENT_TEMPLATE_AMOUNT 0x100

/// ���� �������, ����������� � �������� ������:
//    - ������� (monitor)
//    - ���������� (debug)

#define EVENT_CAT_MONITOR			0x40
#define EVENT_CAT_DEBUG				0x80
#define EVENT_CAT_TRAFFIC			0xC0
#define EVENT_CAT_RESERV			0x00
#define EVENT_CAT_MASK				0xC0

#define EVENT_TYPE_INF				0x10
#define EVENT_TYPE_WRN				0x20
#define EVENT_TYPE_ERR				0x30
#define EVENT_TYPE_RESERV			0x00
#define EVENT_TYPE_MASK				0x30

/*//!!! ���������� �������������� �������� moxa7gate:

�����������
  - 9 ���������� ����������������, �������� 3 ������ ������:
     - IFACE_GATEWAY_SIMPLE
     - IFACE_RTU_MASTER
     - IFACE_RTU_SLAVE
  - 2 ���������� ������� Ethernet
  - 32 ���������� IFACE_TCP_MASTER

�������

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

��������
  - 32 ���������� ������� �����, �������� ��������� ��������:
    - GW_CLIENT_TCP_GWS
    - GW_CLIENT_TCP_502
    - GW_CLIENT_RTU_SLV

*/
#define EVENT_SRC_P1					0x00
#define EVENT_SRC_P2					0x01
#define EVENT_SRC_P3					0x02
#define EVENT_SRC_P4					0x03
#define EVENT_SRC_P5					0x04
#define EVENT_SRC_P6					0x05
#define EVENT_SRC_P7					0x06
#define EVENT_SRC_P8					0x07
#define EVENT_SRC_SYSTEM			0x08
#define EVENT_SRC_GATE502			0x09
#define EVENT_SRC_MOXAMB			0x0A
//#define   MOXA_MB_DEVICE		0x0A
#define EVENT_SRC_MOXATCP			0x0B
#define EVENT_SRC_TCPBRIDGE		0x0C
#define EVENT_SRC_RESERV1			0x0D
#define EVENT_SRC_RESERV2			0x0E
#define EVENT_SRC_RESERV3			0x0F
#define EVENT_SRC_MASK				0x0F

#define TRAFFIC_RTU_RECV	182
#define TRAFFIC_RTU_SEND	183
#define TRAFFIC_TCP_RECV	184
#define TRAFFIC_TCP_SEND	185
#define TRAFFIC_QUEUE_IN	186
#define TRAFFIC_QUEUE_OUT	187

///=== MESSAGES_H data types

typedef struct {				// ������ ������� ������� �����
	time_t	time;					// ����� ������������� �������

//	unsigned int source;  // �������� (���������� ���������� ��� ������ ���������)
//	unsigned int code;		// �������������� ���������� � ������� (��������, ��� ������)
//	char desc[EVENT_MESSAGE_LENGTH]; // proverit' dlinu soobscheniya functsii perror()

	unsigned char msgtype;
	unsigned char msgcode;
	unsigned int prm[4];
	} GW_EventLog;	// GW_Event	 // ����� ���������

///=== MESSAGES_H public variables

  GW_EventLog *app_log; ///!!! ����� �������� ��� ���������� ��������� ����������

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
