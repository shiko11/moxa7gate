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

// ���������������� ��������� ������� ����������

#define EVENT_LOG_LENGTH 100        // ����� ������� ���������
#define EVENT_TEMPLATE_AMOUNT 0x100 // ���������� ��������� � ������� ��������
#define EVENT_MESSAGE_LENGTH 63     // � ����� � ��������� ������ LCM �������� ����� ��������
#define EVENT_MESSAGE_PREFIX 32     // ������� �������� �� ������ 2-� ����� LCM

/// ���� �������, ����������� � �������� ������:
//    - ������� (monitor)
//    - ���������� (debug)

#define EVENT_CAT_MONITOR     0x40  // ������� � ��, ��������� �������� ��������� (�������������)
#define EVENT_CAT_DEBUG       0x80  // ������� � ��, ��������� ����������� ��������� ��� ����������
#define EVENT_CAT_DATAFLOW    0xC0  // �������, ��������� ������������� � ��������� ����������� ������
#define EVENT_CAT_USERCMD     0x00  // ������� ����� ���� ��������� ��� ��������� ����������� ������ �� HMI
#define EVENT_CAT_MASK        0xC0

#define EVENT_TYPE_INF        0x10  // �������������� ���������
#define EVENT_TYPE_WRN        0x20  // ����������������� ��������� � ����� �� ��������� ��������
#define EVENT_TYPE_ERR        0x30  // ��������� �� ������, �������������� ������� ���������������� �����
#define EVENT_TYPE_FTL        0x00  // ��������� ������, ���� ��������� ���� ������
#define EVENT_TYPE_MASK       0x30

#define EVENT_SRC_MASK        0x0F

// ����������� ���� � ���������� �����
#define EVENT_TYPE_ORD(type) (((type) >> 4)&0x03)

// ����������� ��������� � ���������� �����
#define EVENT_CAT_ORD(cat) ((cat) >> 6)

// ����������� ������� ����� ������������� ����� � ������ ��������
#define A2D(str_addr) (str_addr*EVENT_MESSAGE_LENGTH)

/// ���� ���������� ��������� ���� EVENT_CAT_DEBUG

#define POLL_RTU_RECV 97
#define POLL_TCP_RECV 99

#define POLL_WRMBFUNC				107

#define FRWD_TRANS_ADDRESS	108
#define FRWD_TRANS_OVERLAP	109
#define FRWD_TRANS_PQUERY		110
#define FRWD_TRANS_VSLAVE		111

#define POLL_QUEUE_EMPTY 103
#define POLL_QUEUE_OVERL 104

#define POLL_RTU_SEND 98
#define POLL_TCP_SEND 100

/// ���� ���������� ������� ������, ��������� � ������� ������ telnet
#define TRAFFIC_RTU_RECV	182
#define TRAFFIC_RTU_SEND	183
#define TRAFFIC_TCP_RECV	184
#define TRAFFIC_TCP_SEND	185
#define TRAFFIC_QUEUE_IN	186
#define TRAFFIC_QUEUE_OUT	187

/// ��� ��������� ���������, ���������� ����� ������
#define EVENT_PRM_NO   0x00
#define EVENT_PRM_DGT  0x01
#define EVENT_PRM_STR  0x02
#define EVENT_PRM_MASK 0x03

#define EVENT_STR_IFACE  0x00
#define EVENT_STR_CLIENT 0x04
#define EVENT_STR_IOFUNC 0x08
#define EVENT_STR_IPADDR 0x0C
#define EVENT_STR_MASK   0xFC

///=== MESSAGES_H data types

typedef struct {				// ������ ������� ������� �����
	time_t	time;					// ����� ������������� �������

//	unsigned int source;  // �������� (���������� ���������� ��� ������ ���������)
//	unsigned int code;		// �������������� ���������� � ������� (��������, ��� ������)
//	char desc[EVENT_MESSAGE_LENGTH]; // proverit' dlinu soobscheniya functsii perror()

	unsigned char msgtype;
	unsigned char msgcode;
	unsigned int prm[4];
	} GW_Event;

typedef struct { // ������ ������� �����
  GW_Event *app_log;

	unsigned int app_log_current_entry;
	unsigned int app_log_entries_total;

  // ���������� ����� �� ���������� ���������� ���������
	unsigned int msg_filter;

  // �������� ��������� ������� ����
  unsigned int  cat_msgs_amount[4];
  unsigned int type_msgs_amount[4];

  // ������ �������� ���������, ������ ������ � ������ �������� ��������� ������� � ��:
  //char message_template[EVENT_TEMPLATE_AMOUNT][EVENT_MESSAGE_LENGTH];
  char *msg_tpl;

  // ������ ����� ��������
  // ������������ ������������� ����� ������� �������� ��������
  unsigned int *msg_index;

	} GW_EventLog;

///=== MESSAGES_H public variables

extern GW_EventLog EventLog;

///=== MESSAGES_H public functions

// ������� �����������
int init_messages_h();

int init_message_templates();

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

/// ���������� �������������� ����������� ��������� ���������
void get_msgtype_str(unsigned char msgtype, char *str);			 
/// ���������� �������������� ����������� ��������� ���������
void get_msgsrc_str(unsigned char msgtype, unsigned int prm4, char *str);

void iface2str(char *str, int prm);
void iofunc2str(char *str, int prm);
void ipaddr2str(char *str, int prm);

///*************************************************************************************

#endif  /* MESSAGES_H */
