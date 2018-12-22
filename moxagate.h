/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010                 
***************************************/

#ifndef MOXAGATE_H
#define MOXAGATE_H

///*********************** ������ ���������� MODBUS MOXA UC-7410 ***************
///*** �������� ���������� ������
///*** ��������� ������ ��� ���������� ������� HMI-��������������

#include "queue.h"
#include "clients.h"

///=== MOXAGATE_H constants

#define	BASIC_STAT_GATEWAY_INFO	24
#define	GATE_STATUS_BLOCK_LENGTH	BASIC_STAT_GATEWAY_INFO+MAX_QUERY_ENTRIES/16

#define MOXA_DIAPASON_UNDEFINED		0
#define MOXA_DIAPASON_INSIDE			1
#define MOXA_DIAPASON_OUTSIDE			2
#define MOXA_DIAPASON_OVERLAPPED	3

#define MAX_VIRTUAL_SLAVES 128
#define MAX_QUERY_ENTRIES 128

#define EXCEPTION_DIOGEN 0x00000001

///=== MOXAGATE_H data types

typedef struct {
	int				ssd;						// TCP-���� ��� ������ �������� ���������� ����������
	int current_client;				// ����� ���������� ����������

unsigned		tcp_port;	// ����� TCP-����� ��� ������ � ����������������� ������������ ����� � ������� ATM, RTM
u8					modbus_address; // ����������� Modbus-����� ����� ��� ����������� � ���������� ���������� HMI ������
unsigned		status_info; // ��������� ����� ����� ����������� ��������� ����� (�� ��������� ���������� � �������)

char object[DEVICE_NAME_LENGTH];			// ������������ ������� �������������
char location[DEVICE_NAME_LENGTH];		// ����� ��������� ���������� MOXA UC-7410
char version[DEVICE_NAME_LENGTH];			// ������ ����������������� �����
char networkName[DEVICE_NAME_LENGTH];	// ������� ��� ����������
unsigned int IPAddress;								// ������� �����

u8 show_data_flow;		// ���������� ������ ������ � ������ Telnet
u8 show_sys_messages;	// ���������� ��������� ��������� � ������ Telnet
u8 watchdog_timer;		// ������������ Watchdog-������
u8 use_buzzer;				// ����������� ������ ��� ������������ ������
u8 back_light;				// ��������� ��������� �������

  pthread_mutex_t moxa_mutex;

	GW_Queue queue;

  unsigned int accepted_connections_number;
  unsigned int  current_connections_number;
  unsigned int rejected_connections_number;
  
	GW_Client	clients[MAX_MOXA_PORTS*MAX_TCP_CLIENTS_PER_PORT];
  GW_StaticData stat;

	time_t start_time;  // ����� ������� ���������
	time_t timestamp;  // ����� ���������� ���������� ������ ��� web-����������
	unsigned app_log_current_entry, app_log_entries_total;
	unsigned msg_filter;
	char halt;

	// ������ ����� ���������� ��������� Moxa (��������)
	unsigned offset1xStatus, offset2xStatus, offset3xRegisters, offset4xRegisters;
	// ���������� ��������� ��� ������ �� ������
	unsigned amount1xStatus, amount2xStatus, amount3xRegisters, amount4xRegisters;
	// ��������� �� ������� ������
	u8	*wData1x; //������ ����������� 1x ��������� MOXA (coil status)
	u8	*wData2x; //������ ����������� 2x ��������� MOXA (input status)
	u16	*wData3x; //������ ����������� 3x ��������� MOXA (input register)
	u16	*wData4x; //������ ����������� 4x ��������� MOXA (holding register)
	
	} input_cfg_502;

/// ���� ��������� ���� ������ ������� ��������� � Holding-��������� modbus � ������� 06
typedef struct {			// ���� ��������� ����������� ��������� ������������ �����
	unsigned start;			// ��������� ������� ��������� ��������� ������������ �����
	unsigned length;		// ���������� ��������� �������� ��������� ������������ �����
	unsigned port;			// ���������������� ���� ����� ��� ��������������� �������
	unsigned device;		// ����� ���������� � ���� modbus ��� ��������������� �������

  unsigned modbus_table; // ���� �� 4-� ����������� ������ ��������� MODBUS (��. ���� modbus_rtu.h)
  unsigned address_shift;  // �������� � �������� ������������ ���������� modbus (����� ������ �������� �� � ����)
	char device_name[DEVICE_NAME_LENGTH]; // ������������ ����������
  } RT_Table_Entry;

typedef struct {			// modbus-������ ��� ������������ ������ �������� ���������� � ������ PROXY
	unsigned start;			// ��������� ������� ��������� ��������� ������������ �������� ����������
	unsigned length;		// ���������� ��������� � ��������� ������� �������� ����������
  unsigned offset;  // ����� �������� (��������) � �������� ������������ �����, � �������� ���������� ���������� ������
	unsigned port;			// ���������������� ���� ����� ��� ������ �������� ����������
	unsigned device;		// ����� �������� ���������� � ���� modbus

  unsigned mbf; 			// ���� �� ����������� ������� ��������� MODBUS (��. ���� modbus_rtu.h)
  unsigned delay;  // �������� ����� ��������� ������� ��� ������ � ������ PROXY � ������������
	unsigned critical;			// ���������� ������ �� ��������� ���� ������� �����
  unsigned err_counter;  // ������� �������� ���������� ������
  unsigned status_bit;  // ��� ������� ����� (������ � ����������)
	char device_name[DEVICE_NAME_LENGTH]; // ������������ ����������
  } Query_Table_Entry;

///=== MOXAGATE_H public variables

input_cfg_502 gate502;								 // ������ � ��������� ���������� MOXAGATE

RT_Table_Entry vslave[MAX_VIRTUAL_SLAVES];
Query_Table_Entry query_table[MAX_QUERY_ENTRIES];

// ������ �������������� �������� ������ ��� ���������� ������� ��� ������, ��������� �������������
// �������� ��������� modbus-slave. �� �������� ����� ������, ���������� ������������ ��������� �
// ������������ ���������.

// ���������� ��� ���-07, �������� - ������� ������ ������� ���������������� ������, � ������� ���������� �������
unsigned int exceptions; // ������ �� 16 ������
unsigned int except_prm[16]; // �������� ����������

///=== MOXAGATE_H public functions

void *moxa_mb_thread(void *arg); /// ��������� ������� ��������� �������� � MOXA

int checkDiapason(int function, int start_address, int length);
int process_moxamb_request(int client_id, u8 *adu, u16 adu_len, u8 *memory_adu, u16 *memory_adu_len);

#endif  /* MOXAGATE_H */
