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

#include "frwd_queue.h"
#include "clients.h"

///=== INTERFACES_H constants

#define   MAX_MOXA_PORTS 8

#define   NAME_MOXA_PORT           "PORT"
#define   NAME_MOXA_PORT_DEV  "/dev/ttyM"
#define SERIAL_P1	0
#define SERIAL_P2	1
#define SERIAL_P3	2
#define SERIAL_P4	3
#define SERIAL_P5	4
#define SERIAL_P6	5
#define SERIAL_P7	6
#define SERIAL_P8	7

// ����� �������� ������ ��� serial-���������� �����
// ����� ��� �������� � ���������� �� �������� �������
#define GATEWAY_SIMPLE				6
#define IFACE_GATEWAY_SIMPLE	6

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

#define MAX_TCP_SERVERS 32

#define EXCEPTION_DIOGEN 0x00000001

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
	unsigned char mb_slave; // ����� modbus-���������� ��� ��������������� �������� (ATM)
	unsigned short offset;      /// ��������� ������� ������ �������� ����������
	unsigned int ip;        // ������� �����
	unsigned int port;      // ����� TCP-�����
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

//GW_TCPIface tcp_servers[MAX_TCP_SERVERS]; ///!!!

// ������ �������������� �������� ������ ��� ���������� ������� ��� ������, ��������� �������������
// �������� ��������� modbus-slave. �� �������� ����� ������, ���������� ������������ ��������� �
// ������������ ���������.

///=== INTERFACES_H public functions

void *iface_rtu_gws(void *arg); /// ��������� ������� ������ GATEWAY_SIMPLE
void *srvr_tcp_child2(void *arg); ///!!! ��������� ������� ������� GATEWAY_ATM, GATEWAY_RTM
void *iface_tcp_master(void *arg); /// ��������� ������� ������ IFACE_TCPMASTER
void *iface_rtu_master(void *arg); /// ��������� ������� ������ IFACE_RTUMASTER
void *iface_rtu_slave(void *arg); /// ��������� ������� ������ IFACE_RTUSLAVE

#endif  /* INTERFACES_H */
