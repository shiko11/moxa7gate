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

#include "queue.h"
#include "clients.h"

///=== INTERFACES_H constants

#define   MAX_MOXA_PORTS		8

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

#define PROXY_TCP	8

#define SERIAL_STUB 0xff

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

#define MAX_TCP_SERVERS 		32

///=== INTERFACES_H data types

typedef struct { // ��������� ����������������� �����
	int				fd;						// �������������
	unsigned char        p_num;				// ����� ����� MOXA
	char      p_name[12];		// ��� ����� (���������� � �������)
	char      p_mode[12];		// ����� 232/485_2W/422/485_4W
	char			speed[12];		// �������� ������
	char			parity[12];		// �������� ��������
	int		timeout;					// ������� �����

	int ch_interval_timeout;
	} GW_SerialLine;

typedef struct {
	GW_SerialLine serial;

	unsigned short				tcp_port;				// ����� TCP-����� ��� ������ �������� ���������� ����������
	int				ssd;						// TCP-���� ��� ������ �������� ���������� ����������
	int current_client;				// ����� ���������� ����������
	char bridge_status[4];		// ��������� �����
	int modbus_mode;  				// ����� �����

	GW_Client	clients[MAX_TCP_CLIENTS_PER_PORT];
  GW_StaticData stat;
	time_t start_time;  // ����� ��������� ������������� �����

  unsigned int accepted_connections_number;
  unsigned int  current_connections_number;
  unsigned int rejected_connections_number;
  
  pthread_mutex_t serial_mutex;

//	u8			queue_adu[MAX_GATEWAY_QUEUE_LENGTH][MB_TCP_MAX_ADU_LENGTH];// TCP ADU
//	u16			queue_adu_len[MAX_GATEWAY_QUEUE_LENGTH];
//	u16			queue_clients[MAX_GATEWAY_QUEUE_LENGTH];
//	u16			queue_slaves[MAX_GATEWAY_QUEUE_LENGTH];
//	int			queue_start, queue_len;

	GW_Queue queue;

	char description[DEVICE_NAME_LENGTH]; // �������� ������ (���� ModBus)
	} input_cfg;

typedef struct { // ������ ������� TCP_SERVERS
	unsigned char        mb_slave;					// ����� modbus-���������� ��� ��������������� �������� (ATM)
	unsigned int ip;						// ������� �����
	unsigned int port;					// TCP ����
	int address_shift;					// �������� ��������� ������������
	unsigned char        p_num;						// obsolete
	char device_name[DEVICE_NAME_LENGTH]; // ������������ ����������
	} GW_TCP_Server;

///=== INTERFACES_H public variables

input_cfg iDATA[MAX_MOXA_PORTS];     // ������ � ��������� ����������� RTU
input_cfg iDATAtcp[MAX_TCP_SERVERS]; // ������ � ��������� ����������� TCP
GW_TCP_Server tcp_servers[MAX_TCP_SERVERS];

///=== INTERFACES_H public functions

void *srvr_tcp_child(void *arg); /// ��������� ������� ������ GATEWAY_SIMPLE
void *srvr_tcp_child2(void *arg); /// ��������� ������� ������� GATEWAY_ATM, GATEWAY_RTM
void *srvr_tcp_bridge(void *arg); /// ��������� ������� ������ BRIDGE_TCP
void *gateway_proxy_thread(void *arg); /// ��������� ������� ������ GATEWAY_PROXY
void *bridge_proxy_thread(void *arg); /// ��������� ������� ������ BRIDGE_PROXY

#endif  /* INTERFACES_H */
