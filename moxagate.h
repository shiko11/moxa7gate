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

#include "forwarding.h"
#include "clients.h"

///=== MOXAGATE_H constants

// ���������������� ��������� ������� ����������

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

// ���� ������ �������������
#define MOXAGATE_WATCHDOG_STARTED 130

///=== MOXAGATE_H data types

typedef struct {

	time_t start_time;  // ����� �������
  unsigned char modbus_address; // ����������� Modbus-����� ����� ��� ����������� � ���������� ���������� HMI ������
  unsigned short status_info; // ��������� ����� ����� ����������� ��������� ����� (�� ��������� ���������� � �������)

	GW_Queue queue;
  GW_StaticData stat;

  unsigned char map2Xto4X;			// ����� ����������� ������� ���������� ������ �� ������� holding-���������

  // ������� ������������ ��� ������������� ������� ��� ������ � �������
  pthread_mutex_t moxa_mutex;
	// ������ ����� ���������� ��������� Moxa (��������)
	unsigned short offset1xStatus, offset2xStatus, offset3xRegisters, offset4xRegisters;
	// ���������� ��������� � ������ �� ������ MODBUS
	unsigned short amount1xStatus, amount2xStatus, amount3xRegisters, amount4xRegisters;
  // �������� ������� ������������ ������ ����� �������
  unsigned short used1xStatus, used2xStatus, used3xRegisters, used4xRegisters;
	// ��������� �� ������� ������
	unsigned char *wData1x; //������ ����������� 1x ��������� MOXA (coil status)
	unsigned char *wData2x; //������ ����������� 2x ��������� MOXA (input status)
	unsigned short *wData3x; //������ ����������� 3x ��������� MOXA (input register)
	unsigned short *wData4x; //������ ����������� 4x ��������� MOXA (holding register)
	
	int mxwdt_handle;

	} GW_MoxaDevice;

///=== MOXAGATE_H public variables

extern GW_MoxaDevice MoxaDevice; // ������ � ��������� ���������� MOXAGATE

///=== MOXAGATE_H public functions

int init_moxagate_h();
int init_moxagate_memory();

void *moxa_device(void *arg); /// ��������� ������� ��������� �������� � MOXA

int refresh_status_info(); // ���������� ������������ ������ � ����� ����������� �����

void create_proxy_request(int index, u8 *tcp_adu, u16 *tcp_adu_len);  // ������������ ������� �� ������ ������ �� ������� ������
void process_proxy_response(int index, u8 *tcp_adu, u16 tcp_adu_len); // ��������� ������ �� ������ �� ������� ������
void make_tcp_adu(u8 *tcp_adu, int length); // ���������� ����� TCP ADU ��� �������� �� TCP ������

#endif  /* MOXAGATE_H */
