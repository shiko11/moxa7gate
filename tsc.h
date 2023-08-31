/***********   XXXXXXXXX   *************
        MODBUS XXXXXXX SOFTWARE         
                    VERSION X.X         
        AO NPO SPETSELECTROMEHANIKA     
               BRYANSK 2015                 
***************************************/

#ifndef TSC_H
#define TSC_H

///******************* ������ TSC ********************************

#include <sys/time.h>
#include <time.h>

#include "modbus.h"

///=== TSC_H constants

// ���������������� ��������� ������� ����������

#define HAGENT_HEADER_LEN   8 // ����� ��������� ������ ������� � ���������
#define HAGENT_PARAMS_MAX  32 // ������������ ���������� ���������� � ������ �������
#define HAGENT_BUFEIP_MAX 256 // ������������ ������ ������ ������� � ������ ���
#define HAGENT_METRIC_LEN  16 // ����� ��������� ������ � �������� ��������� �������
#define HAGENT_TAGS_LEN    32 // ����� ��������� ������ � ������   ��������� �������
///=== TSC_H data types

typedef struct { // ��������� ������ �������
  unsigned int header; // ��������� �� ������ ��������� ���� ���
  unsigned int cycles; // ���������� ������ ���������� ������
  
  unsigned int min  ;  // ����� �������, ������
  unsigned int hour ;  // ����� �������, ���
  unsigned int day  ;  // ����� �������, ����
  unsigned int month;  // ����� �������, �����
  unsigned int year ;  // ����� �������, ���
  
  unsigned int prmnum; // ������ ������ �������, ���������� ���������� � ������
  unsigned int msprec; // ������ ������ �������, �������� (0 - ���; 1 - ��)
  
  unsigned long fault; // ����� ������ ������������� ����������
  unsigned long group; // ����� ����������� ����������

  struct tm      tm;   // ����� ������� � ������� Linux
  struct timeval tv;   // ����� ������� � ������� Linux
  
	} HB_Header;

typedef struct { // �������������� ��������� �������
  char         metric[HAGENT_METRIC_LEN]; // ������� ��� ������ � openTSDB
  char         tags  [HAGENT_TAGS_LEN  ]; // ����    ��� ������ � openTSDB
  unsigned int adc_min; // ������ ��������� ��������� � ����� ���
  unsigned int adc_max; // �����  ��������� ��������� � ����� ���
  float        eng_min; // ������ ��������� ��������� � ���������� ��������
  float        eng_max; // �����  ��������� ��������� � ���������� ��������
	} HB_Parameter;

typedef struct { // ���� ���������� ��������� ���������� (���)
  unsigned int validity;               // ������� ������������ ����� �������
  unsigned int time_mark;              // ����� ������� � ������� HAGENT
  unsigned int  ms;                    // ����� �������, �����������
  unsigned int sec;                    // ����� �������, �������
  unsigned int min;                    // ����� �������, ������
  struct timeval tv;                   // ����� ������� � ������� Linux
  unsigned int prm[HAGENT_PARAMS_MAX]; // ������ ��������� ���������
	} HB_EIP_Block;

///=== TSC_H public variables

// �������� ������ ������ �������, ����������� �������������� � ������ T01..T04
extern HB_Parameter HB_Param[4][HAGENT_PARAMS_MAX]; // ������ ������������� ���������� �������

///=== TSC_H public functions

// ������� �����������
int init_tsc_h(int port);

int  check_header(u8 *rsp_adu, HB_Header *hdr);
void  copy_header(HB_Header *src, HB_Header *dst);
void  cross_params(int port, unsigned long group);

///*************************************************************************************

#endif  /* TSC_H */
