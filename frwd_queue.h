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

typedef struct {
  unsigned char iface;
  unsigned char address;
  } GW_AddressMap_Entry;

/// ���� ��������� ���� ������ ������� ��������� � Holding-��������� modbus � ������� 06
typedef struct {			// ���� ��������� ����������� ��������� ������������ �����
	unsigned char iface;			// ���������������� ���� ����� ��� ��������������� �������
	unsigned device;		// ����� ���������� � ���� modbus ��� ��������������� �������
  unsigned char modbus_table; // ���� �� 4-� ����������� ������ ��������� MODBUS (��. ���� modbus_rtu.h)
  unsigned short offset;  // �������� � �������� ������������ ���������� modbus (����� ������ �������� �� � ����)

	unsigned short start;			// ��������� ������� ��������� ��������� ������������ �����
	unsigned short length;		// ���������� ��������� �������� ��������� ������������ �����

	char device_name[DEVICE_NAME_LENGTH]; // ������������ ����������
  } RT_Table_Entry; // GW_Vslave_Entry

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
  } Query_Table_Entry; // GW_ProxyQuery_Entry

typedef struct {
  ///!!! ���������� ��������� ����� ��� ��������� ���������� �� �� � ����� �������� �� ����������� ������
	unsigned char stage; // ������ ����������� �������

  // �������� - ��� ����������� ��������,\
     ��������������� ��� ��������� ����� ����������� ��������.
	unsigned char action;

  // �������� - ��� ������ � ����� �������, ���������� � ����� �
  // 32-���������� ����� ��� ��������� ������������.
  unsigned int prm1;
  unsigned int prm2;
  unsigned int prm3;
  unsigned int prm4;

	char comment[DEVICE_NAME_LENGTH]; // �����������
  } GW_Exception;

///=== FRWD_QUEUE_H public variables

// ����� ���������. ������������ ��� ������������� ������ ������� � ���������.
int semaphore_id;

GW_AddressMap_Entry AddressMap[MODBUS_ADDRESS_MAX+1]; // ��������� � �������
RT_Table_Entry vslave[MAX_VIRTUAL_SLAVES];
Query_Table_Entry query_table[MAX_QUERY_ENTRIES];
GW_Exception Exception[MOXAGATE_EXCEPTIONS_NUMBER];

// obsolete
// ���������� ��� ���-07, �������� - ������� ������ ������� ���������������� ������, � ������� ���������� �������
unsigned int exceptions; // obsolete // ������ �� 16 ������
unsigned int except_prm[16]; // obsolete // �������� ����������

///=== FRWD_QUEUE_H public functions

int init_frwd_queue_h();

int init_AddressMap_Entry(int index);
int check_AddressMap_Entry(int index);

int init_Vslave_Entry(int index);
int check_Vslave_Entry(int index);

int init_ProxyQuery_Entry(int index);
int check_ProxyQuery_Entry(int index);

int init_Exception(int index);
int check_Exception(int index);

int init_queue();
int enqueue_query_ex(GW_Queue *queue, int client_id, int device_id, u8 *adu, u16 adu_len);
int get_query_from_queue(GW_Queue *queue, int *client_id, int *device_id, u8 *adu, u16 *adu_len);

int checkDiapason(int function, int start_address, int length);
int process_moxamb_request(int client_id, u8 *adu, u16 adu_len, u8 *memory_adu, u16 *memory_adu_len);

#endif  /* FRWD_QUEUE_H */
