/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

#ifndef CLI_H
#define CLI_H

///**************** ������ ������� ���������� ��������� ������������ ***********

#include "interfaces.h"
#include "moxagate.h"

///=== CLI_H constants

#define   MAX_KEYS 64        //������������ ���������� ���������� ��������� ������ � ��������� "--"
//������ ������������� ��������� ������
#define   CL_ERR_NONE_PARAM        -1    //��������� ������ �����
#define   CL_INFO                  -5
#define   CL_ERR_PORT_WORD         -2   //������ � �������� ������ // keyword PORT absent
#define   CL_ERR_IN_STRUCT         -3   //������ � ������ ��������� ������
#define   CL_ERR_IN_PORT_SETT      -4   //������ � ������ ��������� ������
#define   CL_ERR_GATEWAY_MODE      -6
#define   CL_ERR_IN_MAP						 -7
#define   CL_ERR_MIN_PARAM				 -8
#define   CL_ERR_MUTEX_PARAM			 -9
#define   CL_ERR_VSLAVES_CFG			 -10
#define   CL_ERR_QT_CFG						 -11
#define   CL_ERR_TCPSRV_CFG				 -12
#define   CL_ERR_NOT_ALLOWED			 -13
#define   CL_OK                    0    //��� �������

//#define   CL_ERR_MIN_PARAM         -1   //����� 9 ����������
//#define   CL_ERR_NUM_STAT_BIT			 -4 //����� ���� ������ 16 (�������� 1-16)

///=== CLI_H public variables

///=== CLI_H public functions

int get_command_line (int 	argc,
											char	*argv[],
											GW_Iface *ptr_iDATA,
											GW_MoxaDevice *ptr_gate502,
											RT_Table_Entry *vslave,
											Query_Table_Entry *query_table
											);

//void sigpipe_handler();
//void sigio_handler();

#endif  /* CLI_H */
