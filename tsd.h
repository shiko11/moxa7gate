/***********   XXXXXXXXX   *************
        MODBUS XXXXXXX SOFTWARE         
                    VERSION X.X         
        AO NPO SPETSELECTROMEHANIKA     
               BRYANSK 2015                 
***************************************/

#ifndef TSD_H
#define TSD_H

///******************* ������ TSD ********************************

#include "modbus.h"

///=== TSD_H constants

// ���������������� ��������� ������� ����������

#define HAGENT_HEADER_LEN   8 // ����� ��������� ������ ������� � ���������
#define HAGENT_PARAMS_MAX  32 // ������������ ���������� ���������� � ������ �������
#define HAGENT_METRIC_LEN  16 // ����� ��������� ������ � �������� ��������� �������

///=== TSD_H data types

typedef struct { // ��������� ������ �������
  unsigned int header; // ��������� �� ������ ��������� ���� ���
  unsigned int cycles; // ���������� ������ ���������� ������
  
  unsigned int min  ;  // ����� ������� ��������� ������ �������, ������
  unsigned int hour ;  // ����� ������� ��������� ������ �������, ���
  unsigned int day  ;  // ����� ������� ��������� ������ �������, ����
  unsigned int month;  // ����� ������� ��������� ������ �������, �����
  unsigned int year ;  // ����� ������� ��������� ������ �������, ���
  
  unsigned int prmnum; // ������ ������ �������, ���������� ���������� � ������
  unsigned int msprec; // ������ ������ �������, �������� (0 - ���; 1 - ��)
  
  unsigned int fault1; // ����� 1 ������ ������������� ����������
  unsigned int fault2; // ����� 2 ������ ������������� ����������
  unsigned int group1; // ����� 1 ����������� ����������
  unsigned int group2; // ����� 2 ����������� ����������
  
	} HB_Header;

typedef struct { // �������� �������
  char         metric[HAGENT_METRIC_LEN]; // ������� ��� ������ � openTSDB
  unsigned int adc_min; // ������ ��������� ��������� � ����� ���
  unsigned int adc_max; // �����  ��������� ��������� � ����� ���
  float        eng_min; // ������ ��������� ��������� � ���������� ��������
  float        eng_max; // �����  ��������� ��������� � ���������� ��������
	} HB_Parameter;

///=== TSD_H public variables

// �������� ������ ������ �������, ����������� �������������� � ������ T01..T04
HB_Parameter HB_Param[4][HAGENT_PARAMS_MAX];

///=== TSD_H public functions

// ������� �����������
int init_tsd_h(int port);

int  check_header(u8 *rsp_adu, HB_Header *hbhdr);
void  copy_header(HB_Header *src, HB_Header *dst);

///*************************************************************************************

#endif  /* TSD_H */
