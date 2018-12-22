/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010                 
***************************************/

#ifndef FRWD_QUEUE_H
#define FRWD_QUEUE_H

///*********  ������ ������� ���������������, �������� �������� MODBUS *********

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

typedef struct { // ������� �� ���������
	int port_id;

	unsigned char			queue_adu[MAX_GATEWAY_QUEUE_LENGTH][MB_TCP_MAX_ADU_LENGTH];// TCP ADU
	unsigned short		queue_adu_len[MAX_GATEWAY_QUEUE_LENGTH];
	unsigned short		queue_clients[MAX_GATEWAY_QUEUE_LENGTH];
	unsigned short		queue_slaves[MAX_GATEWAY_QUEUE_LENGTH];
	int			queue_start, queue_len;

  pthread_mutex_t queue_mutex;
	struct sembuf operations[1];
	} GW_Queue;

/// ���� ��������� ���� ������ ������� ��������� � Holding-��������� modbus � ������� 06
typedef struct {			// ���� ��������� ����������� ��������� ������������ �����
	unsigned short start;			// ��������� ������� ��������� ��������� ������������ �����
	unsigned short length;		// ���������� ��������� �������� ��������� ������������ �����
	unsigned char iface;			// ���������������� ���� ����� ��� ��������������� �������
	unsigned device;		// ����� ���������� � ���� modbus ��� ��������������� �������

  unsigned char modbus_table; // ���� �� 4-� ����������� ������ ��������� MODBUS (��. ���� modbus_rtu.h)
  unsigned short offset;  // �������� � �������� ������������ ���������� modbus (����� ������ �������� �� � ����)
	char device_name[DEVICE_NAME_LENGTH]; // ������������ ����������
  } RT_Table_Entry; // GW_RTM_Table_Entry

typedef struct {			// modbus-������ ��� ������������ ������ �������� ���������� � ������ PROXY
	unsigned char iface;		// ��������� ����� ��� ������ �������� ����������
	unsigned char device;		// ����� �������� ���������� � ���� modbus
  unsigned char mbf; 			// ���� �� ����������� ������� ��������� MODBUS (��. ���� modbus_rtu.h)
  unsigned char access;  // ����� ������� � ����� ������: ������ ������, ������/������, ���������

	unsigned short  start;			// ��������� ������� ��������� ��������� ������������ �������� ����������
	unsigned short length;		// ���������� ��������� � ��������� ������� �������� ����������
  unsigned short offset;  // ����� �������� (��������) � �������� ������������ �����, � �������� ���������� ���������� ������

  unsigned delay;  // �������� ����� ��������� ������� ��� ������ � ������ PROXY � ������������
	unsigned critical;			// ���������� ������ �� ��������� ���� ������� �����
	char device_name[DEVICE_NAME_LENGTH]; // ������������ ����������

  unsigned err_counter;  // ������� �������� ���������� ������
  unsigned status_bit;  // ��� ������� ����� (������ � ����������)
  } Query_Table_Entry; // GW_Query_Table_Entry

///=== FRWD_QUEUE_H public variables

// ����� ���������. ������������ ��� ������������� ������ ������� � ���������.
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
