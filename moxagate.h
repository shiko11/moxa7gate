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

#include "frwd_queue.h"
#include "clients.h"

///=== MOXAGATE_H constants

#define	BASIC_STAT_GATEWAY_INFO 24
#define	GATE_STATUS_BLOCK_LENGTH BASIC_STAT_GATEWAY_INFO+MAX_QUERY_ENTRIES/16

///=== MOXAGATE_H data types

typedef struct {

	time_t start_time;  // ����� �������
  unsigned char modbus_address; // ����������� Modbus-����� ����� ��� ����������� � ���������� ���������� HMI ������
  ///!!! ����������� ���� ������ ����������� ��������� �������� ������, �� ��������� � ���������� wData4x
  unsigned short status_info; // ��������� ����� ����� ����������� ��������� ����� (�� ��������� ���������� � �������)

	GW_Queue queue;
  GW_StaticData stat;

  unsigned char map_2x_to_4x;			// ����� ����������� ������� ���������� ������ �� ������� holding-���������

  // ������� ������������ ��� ������������� ������� ��� ������ � �������
  pthread_mutex_t moxa_mutex;
	// ������ ����� ���������� ��������� Moxa (��������)
	unsigned short offset1xStatus, offset2xStatus, offset3xRegisters, offset4xRegisters;
	// ���������� ��������� � ������ �� ������ MODBUS
	unsigned short amount1xStatus, amount2xStatus, amount3xRegisters, amount4xRegisters;
	// ��������� �� ������� ������
	unsigned char *wData1x; //������ ����������� 1x ��������� MOXA (coil status)
	unsigned char *wData2x; //������ ����������� 2x ��������� MOXA (input status)
	unsigned short *wData3x; //������ ����������� 3x ��������� MOXA (input register)
	unsigned short *wData4x; //������ ����������� 4x ��������� MOXA (holding register)
	
	} GW_MoxaDevice;

///=== MOXAGATE_H public variables

  GW_MoxaDevice MoxaDevice; // ������ � ��������� ���������� MOXAGATE

///=== MOXAGATE_H public functions

int init_moxagate_h();

void *moxa_device(void *arg); /// ��������� ������� ��������� �������� � MOXA

#endif  /* MOXAGATE_H */
