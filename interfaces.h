/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010                 
***************************************/

///*********************** ������ ����������� ����� ****************************
///*** �������� ������ MODBUS RTU MASTER
///*** �������� ������ MODBUS TCP MASTER
///*** �������� ������������ MODBUS RTU SLAVE
///*** �������� ��������������� GATEWAY_SIMPLE

#ifndef INTERFACES_H
#define INTERFACES_H

#include "forwarding.h"
#include "clients.h"

///=== INTERFACES_H constants

// ���������������� ��������� ������� ����������

#define   MAX_MOXA_PORTS 8

/*//!!! ���������� �������������� �������� moxa7gate:

�����������
  - 9 ���������� ����������������, �������� 3 ������ ������:
     - IFACE_GATEWAY_SIMPLE
     - IFACE_RTU_MASTER
     - IFACE_RTU_SLAVE
  - 2 ���������� ������� Ethernet
  - 32 ���������� IFACE_TCP_MASTER

�������

  - CLI_H
  - MODBUS_H
  - MAIN_H

  - CLIENTS_H
  - FRWD_QUEUE_H
  - INTERFACES_H
  - MOXAGATE_H

  - MESSAGES_H
  - STATISTICS_H
  - HMI_KEYPAD_LCM_H
  - HMI_WEB_H

��������
  - 32 ���������� ������� �����, �������� ��������� ��������:
    - GW_CLIENT_TCP_GWS
    - GW_CLIENT_TCP_502
    - GW_CLIENT_RTU_SLV

*/

/// MOXA7GATE ASSETS

#define GATEWAY_ASSETS 64

#define GATEWAY_P1	0
#define GATEWAY_P2	1
#define GATEWAY_P3	2
#define GATEWAY_P4	3
#define GATEWAY_P5	4
#define GATEWAY_P6	5
#define GATEWAY_P7	6
#define GATEWAY_P8	7

#define GATEWAY_P9        8
#define GATEWAY_SYSTEM    9
#define GATEWAY_MOXAGATE  10
#define GATEWAY_LANTCP    11
#define GATEWAY_SECURITY  12
#define GATEWAY_FRWD      13
#define GATEWAY_RESERVE   14
#define GATEWAY_HMI       15

#define GATEWAY_T01 16
#define GATEWAY_T02 17
#define GATEWAY_T03 18
#define GATEWAY_T04 19
#define GATEWAY_T05 20
#define GATEWAY_T06 21
#define GATEWAY_T07 22
#define GATEWAY_T08 23

#define GATEWAY_T09 24
#define GATEWAY_T10 25
#define GATEWAY_T11 26
#define GATEWAY_T12 27
#define GATEWAY_T13 28
#define GATEWAY_T14 29
#define GATEWAY_T15 30
#define GATEWAY_T16 31

#define GATEWAY_T17 32
#define GATEWAY_T18 33
#define GATEWAY_T19 34
#define GATEWAY_T20 35
#define GATEWAY_T21 36
#define GATEWAY_T22 37
#define GATEWAY_T23 38
#define GATEWAY_T24 39

#define GATEWAY_T25 40
#define GATEWAY_T26 41
#define GATEWAY_T27 42
#define GATEWAY_T28 43
#define GATEWAY_T29 44
#define GATEWAY_T30 45
#define GATEWAY_T31 46
#define GATEWAY_T32 47

#define GATEWAY_NONE 63

#define GATEWAY_IFACE 0x0F
#define IFACETCP_MASK 0x30

/// ���� ������ ����������� ������������
#define IFACE_MBMODE 73

#define IFACE_RTUPHYSPROT 13
#define IFACE_RTUSPEED 14
#define IFACE_RTUPARITY 15
#define IFACE_RTUTIMEOUT 16
#define IFACE_RTUTCPPORT 17

#define IFACE_TCPIP1 20
#define IFACE_TCPPORT1 21
#define IFACE_TCPUNITID 22
#define IFACE_TCPOFFSET 23
#define IFACE_TCPMBADDR 24
#define IFACE_TCPIP2 25
#define IFACE_TCPPORT2 26
#define IFACE_TCPIPEQUAL 27

// ����� �������� ������ ��� serial-���������� �����
// ����� ��� �������� � ���������� �� �������� �������
#define GATEWAY_SIMPLE				6
#define IFACE_TCPSERVER 6

///!!! ��� ����������� ������� ������ ��������, ��� ��������� ���� ����� ��� ���������� ��������� ���������������
#define GATEWAY_ATM					7
#define GATEWAY_RTM					8
#define GATEWAY_PROXY				9
// ������ ���� ���������� ��������� ����:
#define IFACE_RTUMASTER			13

// �� ������ ATM �������� ��������������� ��������� ���������� MOXA
// ��� ������������� ��� ���������� � ������ ������� HMI-������� �����
// �� ���� �������������� � ���������
#define IFACE_MOXAGATE			0x0A
//#define EVENT_SRC_MOXAMB	0x0A
// ��� ����� ���. ����� ��� ������������� ��������!
#define   MOXA_MB_DEVICE		0x0A

///!!! ��� ����������� ������� ������ ��������
#define BRIDGE_TCP					11
#define BRIDGE_PROXY				12
// ������ ���� ���������� ��������� ����:
#define IFACE_RTUSLAVE			14

// ��������� ����� ����� ��� ���������� �����: TCP-���������
#define IFACE_TCPMASTER			15

// ��������� ������ ��������� ����������� �����
#define MODBUS_PORT_ERROR		4
#define MODBUS_PORT_OFF			5
#define IFACE_ERROR					16
#define IFACE_OFF						17

#define   NAME_MOXA_PORT           "PORT"
#define   NAME_MOXA_PORT_DEV  "/dev/ttyM"

#define TIMEOUT_MIN 100000
#define TIMEOUT_MAX 10000000

///=== INTERFACES_H data types

// ��������� ����������������� ����� �����
///!!! �� ������� ������� ����������: ���������� �������� �����,
///    ����� ������, ������ �������� ������, ������ RTU/ASCII
typedef struct {
	int fd;                // ��������� �������������
	char p_name[12];  // ��� ����� (���������� � �������)
	char p_mode[12];  // ����� ������ 232/485_2W/422/485_4W
	char speed[12];   // �������� ������
	char parity[12];  // �������� ��������

	int timeout;      // ������� �����
	unsigned int ch_interval_timeout; ///!!!
	} GW_SerialIface;

// ��������� TCP-����� �����
typedef struct {
	unsigned int ip;        // ������� �����
	unsigned int port;      // ����� TCP-�����
  ///??? ��� unsigned char �������� ����� ������ �������� ���������� (runtime):
  ///??? ���������� ������ unsigned
	unsigned unit_id;  // ����� �� ������� �������� ������� ����������
	unsigned short offset;  /// ��������� ������� ������ �������� ����������
	unsigned mb_slave; // ����� modbus-���������� ��� ��������������� �������� (ATM) (�����������)
	unsigned int ip2;       // ��������� ������� �����
	unsigned int port2;     // ��������� ����� TCP-�����
	} GW_TCPIface;

// ��������� ���������� ����� (������� ��������� GW_SerialIface � GW_SerialIface)
typedef struct {
  
  ///=== ������� �������� ���������� (public members)

  GW_Security Security;
	int modbus_mode;          // ��������� ����������
	char bridge_status[4];    // ����������� ��������� ���������� ��� LCM
	char description[DEVICE_NAME_LENGTH]; // �������� ������ (���� ModBus)

	GW_SerialIface serial;
  GW_TCPIface ethernet;
						 
	GW_Queue queue;      // ������� �������� ����������
  GW_StaticData stat;  // ��������� ������ �� ����������� ������

  // ������ �������� �� ���� iface ������� ������
  unsigned char PQueryIndex[MAX_QUERY_ENTRIES+1];

  ///=== ������� ������� ���������� (private members)

  // ���������� ������ GATEWAY_SIMPLE
	int ssd;                  // TCP-���� ��� ������ �������� ���������� ����������
  pthread_mutex_t serial_mutex; // ������� - ��������� ���������� ��� ������������� �������

  // ���������� ������ IFACE_RTUMASTER, IFACE_RTUSLAVE
	int rc;          // ��������� ������� �������� ���������������� ������
	pthread_t tid_srvr; // ��������� ������������� ���������������� ������

	} GW_Iface;

///=== INTERFACES_H public variables

GW_Iface IfaceRTU[MAX_MOXA_PORTS];  // ������ � ��������� ����������� RTU
GW_Iface IfaceTCP[MAX_TCP_SERVERS]; // ������ � ��������� ����������� TCP

///=== INTERFACES_H public functions

int init_interfaces_h(); // ������� �����������

int check_Iface(GW_Iface *iface);

void *iface_rtu_gws(void *arg); /// ��������� ������� ������ GATEWAY_SIMPLE
void *srvr_tcp_child2(void *arg); ///!!! ��������� ������� ������� GATEWAY_ATM, GATEWAY_RTM
void *iface_tcp_master(void *arg); /// ��������� ������� ������ IFACE_TCPMASTER
void *iface_rtu_master(void *arg); /// ��������� ������� ������ IFACE_RTUMASTER
void *iface_rtu_slave(void *arg); /// ��������� ������� ������ IFACE_RTUSLAVE

#endif  /* INTERFACES_H */
