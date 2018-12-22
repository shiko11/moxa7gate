/*
MODBUS GATEWAY SOFTWARE
MOXA7GATE VERSION 1.2
SEM-ENGINEERING
					BRYANSK 2010
*/

#ifndef GLOBAL_H
#define GLOBAL_H

#include <pthread.h>
#include <sys/time.h>
#include <sys/socket.h>

#include <errno.h>
#include <fcntl.h>

#include <netinet/in.h>

//#include <signal.h>
//#include <sys/ipc.h>
//#include <sys/types.h>

#define DEVICE_NAME_LENGTH	64
#define GATE_WEB_INTERFACE_TIMEOUT	2

///=== �������� ������ ���������

#include "modbus.h"
#include "modbus_rtu.h"
#include "hmi_keypad_lcm.h"
#include "mxlib/mxwdg.h"
#include "messages.h"
#include "statistics.h"
#include "clients.h"
#include "interfaces.h"
#include "moxagate.h"
#include "hmi_web.h"

#define MAX_VIRTUAL_SLAVES 128
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

#define MAX_QUERY_ENTRIES 128
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

//unsigned int buzzer_flag; // ������ ���� 1, 2 � 3 ����� � ����������� �� ���������� ������: <15, 15-30, >30
struct timeval tv;
struct timeval tv_mem;
struct timezone tz;
unsigned int p_errors[MAX_MOXA_PORTS];

// ���������� ��������������������� �����
fd_set watchset;
fd_set inset;

RT_Table_Entry vslave[MAX_VIRTUAL_SLAVES];
Query_Table_Entry query_table[MAX_QUERY_ENTRIES];

// ������ �������������� �������� ������ ��� ���������� ������� ��� ������, ��������� �������������
// �������� ��������� modbus-slave. �� �������� ����� ������, ���������� ������������ ��������� �
// ������������ ���������.

// ���������� ��� ���-07, �������� - ������� ������ ������� ���������������� ������, � ������� ���������� �������
#define EXCEPTION_DIOGEN 0x00000001

unsigned int exceptions; // ������ �� 16 ������
unsigned int except_prm[16]; // �������� ����������

#define   MAX_KEYS                 64        //������������ ���������� ���������� ��������� ������ � ��������� "--"
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

int get_command_line (int 	argc,
											char	*argv[],
											input_cfg *ptr_iDATA,
											input_cfg_502 *ptr_gate502,
											RT_Table_Entry *vslave,
											Query_Table_Entry *query_table,
											GW_TCP_Server *tcp_servers
											);

//void sigpipe_handler();
//void sigio_handler();

int gateway_common_processing();
int gateway_single_port_processing();
int query_translating();

int get_ip_from_string(char *str, unsigned int *ip, unsigned int *port);

#endif  /* GLOBAL_H */
