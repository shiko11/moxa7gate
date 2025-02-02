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

// ���������������� ��������� ������� ����������

#define   MAX_GATEWAY_QUEUE_LENGTH 16

// ������ ������ ��������� ���������������

#define FRWD_RESULT_OK					0x00
#define FRWD_RESULT_UNDEFINED		0x01
#define FRWD_RESULT_OVERLAPPED	0x02
#define FRWD_RESULT_QUEUE_FAIL	0x03
#define FRWD_RESULT_UNSUP_FUNC	0x04

#define FRWD_TYPE_PROXY					0x10
#define FRWD_TYPE_REGISTER			0x20
#define FRWD_TYPE_ADDRESS				0x30

#define FRWD_RESULT_MASK				0x0F
#define FRWD_TYPE_MASK					0xF0

// ���� �������� �� ������� ����������� ������������ �������� ����������

#define QT_ACCESS_DISABLED  0
#define QT_ACCESS_READONLY  1
#define QT_ACCESS_WRITEONLY 2
#define QT_ACCESS_READWRITE 3

#define QT_DELAY_RTU_MIN 0
#define QT_DELAY_RTU_MAX 10000
#define QT_DELAY_TCP_MIN 20
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

/// ���� ������ ����������� ������������
#define ATM_IFACE 30
#define ATM_MBADDR 31

#define VSLAVE_IFACE 76
#define VSLAVE_MBADDR 35
#define VSLAVE_MBTABL 36
#define VSLAVE_BEGDIAP 37
#define VSLAVE_ENDDIAP 38
#define VSLAVE_LENDIAP 39

#define PQUERY_IFACE 74
#define PQUERY_MBADDR 43
#define PQUERY_MBTABL 44
#define PQUERY_ACCESS 45
#define PQUERY_ENDREGREAD 46
#define PQUERY_LENPACKET 47
#define PQUERY_ENDREGWRITE 48
#define PQUERY_DELAYMIN 49
#define PQUERY_DELAYMAX 50
#define PQUERY_ERRCNTR 51

#define EXPT_STAGE 60
#define EXPT_ACTION 55
#define EXPT_PRM1 56
#define EXPT_PRM2 57
#define EXPT_PRM3 58
#define EXPT_PRM4 59

/// ���� ������ �������������

#define SEMAPHORE_SET_EXISTS 102

///=== FRWD_QUEUE_H data types

typedef struct { // ������� �� ���������
	int port_id;

	unsigned char			queue_adu[MAX_GATEWAY_QUEUE_LENGTH][MB_TCP_MAX_ADU_LENGTH];// TCP ADU
	unsigned short		queue_adu_len[MAX_GATEWAY_QUEUE_LENGTH];
	unsigned short		queue_clients[MAX_GATEWAY_QUEUE_LENGTH];
	unsigned short		queue_slaves[MAX_GATEWAY_QUEUE_LENGTH];
	int			queue_start, queue_len;

  pthread_mutex_t queue_mutex;
	struct sembuf operations[1]; // ������������ ������ ��� �������� �������� sem_flg
	} GW_Queue;

typedef struct {
  unsigned char iface;
  unsigned char address;
  } GW_AddressMap_Entry;

/// ���� ��������� ���� ������ ������� ��������� � Holding-��������� modbus � ������� 06
typedef struct {			// ���� ��������� ����������� ��������� ������������ �����
	unsigned char iface;			// ���������������� ���� ����� ��� ��������������� �������
	unsigned device;		// ����� ���������� � ���� modbus ��� ��������������� �������
  ///??? ��� unsigned char �������� ����� run-time level ������, ������� ���������� unsigned
  unsigned modbus_table; // ���� �� 4-� ����������� ������ ��������� MODBUS (��. ���� modbus_rtu.h)
  unsigned short offset;  // �������� � �������� ������������ ���������� modbus (����� ������ �������� �� � ����)

	unsigned short start;			// ��������� ������� ��������� ��������� ������������ �����
	unsigned short length;		// ���������� ��������� �������� ��������� ������������ �����

	unsigned critical;			// ���������� ������ �� ��������� ���� ������� �����
  unsigned err_counter;  // ������� �������� ���������� ������
  unsigned status_bit;  // ��� ������� ����� (������ � ����������)

	char device_name[DEVICE_NAME_LENGTH]; // ������������ ����������
  } GW_VSlave_Entry;

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
  } GW_ProxyQuery_Entry;

typedef struct {
  ///!!! ���������� ��������� ����� ��� ��������� ���������� �� �� � ����� �������� �� ����������� ������
	unsigned char stage; // ������ ����������� �������

  // �������� - ��� ����������� ��������,
  // ��������������� ��� ��������� ����� ����������� ��������.
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
extern int semaphore_id;

extern GW_AddressMap_Entry AddressMap[MODBUS_ADDRESS_MAX+1]; // ��������� � �������
extern GW_VSlave_Entry      VSlave[MAX_VIRTUAL_SLAVES];
extern GW_ProxyQuery_Entry   PQuery[MAX_QUERY_ENTRIES];
extern GW_Exception        Exception[MOXAGATE_EXCEPTIONS_NUMBER];

extern unsigned short vsmem_offset1xStatus, vsmem_offset2xStatus, vsmem_offset3xRegisters, vsmem_offset4xRegisters;
extern unsigned short vsmem_amount1xStatus, vsmem_amount2xStatus, vsmem_amount3xRegisters, vsmem_amount4xRegisters;
extern unsigned short vsmem_used1xStatus, vsmem_used2xStatus, vsmem_used3xRegisters, vsmem_used4xRegisters;

// ������ �������������� �������� ������ ��� ���������� ������� ��� ������, ��������� �������������
// �������� ��������� modbus-slave. �� �������� ����� ������, ���������� ������������ ��������� �
// ������������ ���������.

// obsolete
// ���������� ��� ���-07, �������� - ������� ������ ������� ���������������� ������, � ������� ���������� �������
extern unsigned int exceptions; // obsolete // ������ �� 16 ������
extern unsigned int except_prm[16]; // obsolete // �������� ����������

///=== FRWD_QUEUE_H public functions

int init_frwd_queue_h(); // ������������� ����������

int init_AddressMap_Entry(int index);
int check_AddressMap_Entry(int index);

int init_Vslave_Entry(int index);
int check_Vslave_Entry(int index);

int init_ProxyQuery_Entry(int index);
int check_ProxyQuery_Entry(int index);

int init_Exception(int index);
int check_Exception(int index);

int init_sem_set(); // ������������� ���������
int init_queue(GW_Queue *queue, int port); // ������������� �������� ��������
int enqueue_query_ex(GW_Queue *queue, int client_id, int context, u8 *adu, u16 adu_len);
int get_query_from_queue(GW_Queue *queue, int *client_id, int *context, u8 *adu, u16 *adu_len);

int checkDiapason(int function, int start_address, int length);
int process_moxamb_request(int client_id, u8 *adu, u16 adu_len, u8 *memory_adu, u16 *memory_adu_len);
int forward_query(int client_id, u8 *tcp_adu, u16 tcp_adu_len);

// ���������� ������� � ������ � ������������ � ��������� ���������������
void prepare_request (int context, u8 *tcp_adu, u16 tcp_adu_len);
void prepare_response(int context, u8 *tcp_adu, u16 tcp_adu_len);

int translateRegisters(int start_address, int length, int *port_id, int *device_id);
int translateProxyDevice(int start_address, int length, int *port_id, int *device_id);

#endif  /* FRWD_QUEUE_H */
