/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010                 
***************************************/

#ifndef CLIENTS_H
#define CLIENTS_H

///****************** ������ � ����������� ������������ ************************

#include <sys/time.h>
#include <pthread.h>

#include "modbus.h"

///=== CLIENTS_H constants

// ���������������� ��������� ������� ����������

///!!! ��� ��������� ����������� ������ �����������
#define MAX_TCP_SERVERS 32

#define MAX_TCP_CLIENTS_PER_PORT 4
#define MOXAGATE_CLIENTS_NUMBER 32

#define MAX_CLIENT_ACTIVITY_TIMEOUT 30

//#define DEFAULT_CLIENT 0
//#define MB_SLAVE_NOT_DEFINED			0xff

#define TCP_PORT_MIN 1
#define TCP_PORT_MAX 0xFFFF

/// ���� ������ ����������� ������������
#define SECURITY_TCPPORT 75
#define MOXAGATE_MBADDR 8
#define MOXAGATE_STATINFO 9

/*--- ����� �������� 4 ��������� ���� �������� �����:
	1. ������ �� ������� TCP, ������������ � ����� IFACE_TCPSERVER;
	2. ������ �� ������� TCP, ������������ � ����� ����� �������� TCP ���� (������ 502);
	3. ������ �� ������� RTU, ������������ � ����� IFACE_RTUSLAVE;
	4. HMI ������ (������ ����������� ������� �� ��������� ������������);
*/

// ��������� ����������� ����������
#define GW_CLIENT_CLOSED			1
#define GW_CLIENT_ERROR				2
#define GW_CLIENT_TCP_GWS			3
#define GW_CLIENT_TCP_502			4
#define GW_CLIENT_RTU_SLV			5

// ���������, ��������������� ��� �������� � ���������� ������
typedef struct {

	time_t start_time;  // ����� ������� �����

  char Object[DEVICE_NAME_LENGTH];        // ������������ ������� �������������
  char Location[DEVICE_NAME_LENGTH];      // ����� ��������� ���������� MOXA UC-7410
  char Label[DEVICE_NAME_LENGTH];         //����������� �� ������������
  char NetworkName[DEVICE_NAME_LENGTH];   // ������� ��� ����������
  unsigned int LAN1Address;               // ������� ����� ��������
  unsigned int LAN2Address;               // ������� ����� ���������
  char VersionNumber[DEVICE_NAME_LENGTH]; // ������ ����������������� �����
  char VersionTime[DEVICE_NAME_LENGTH];   // ����� �������� ����������������� �����
  char Model[DEVICE_NAME_LENGTH];         // ������ ����������

  unsigned char show_data_flow;     // ���������� ������ ������ � ������ Telnet
  unsigned char show_sys_messages;	// �������� � ������ ���������� ���������
  unsigned char watchdog_timer;     // ������������ Watchdog-������
  unsigned char use_buzzer;         // ����������� ������ ��� ������������

  ///!!!
	time_t timestamp;  // ����� ���������� ���������� ������ ��� web-����������
 	unsigned char halt; // ���� ��������������� �������� ��������� �� �������
  unsigned int back_light;

  unsigned int tcp_port;	// ����� TCP-����� ��� ������ �������� ���������� ����������

  unsigned int accepted_connections_number;
  unsigned int current_connections_number;
  unsigned int rejected_connections_number;

  // ������ �������� ������������������ TCP-�����������
  unsigned char TCPIndex[MAX_TCP_SERVERS+1];

	} GW_Security;

typedef struct { // ��������� ����������� ����������

	int status;	// ��������� ����������� ����������
	unsigned char iface;		// ����� ����������, ������������� � ���� �������
	char device_name[DEVICE_NAME_LENGTH]; // ������������ ���������� (������� ���, DNS)

	unsigned int ip; 
	unsigned int port;

	time_t	connection_time;		// ����� �����������
	time_t	disconnection_time;		// ����� ����������
	time_t	last_activity_time;		// ����� ���������� �������

  // ������ ��� ������ �������� � �������� �������
  u8  inp_adu[MB_TCP_MAX_ADU_LENGTH];
  u16 inp_len;
  u8  out_adu[MB_TCP_MAX_ADU_LENGTH];
  u16 out_len;
  
	GW_StaticData stat;
	
  ///=== ������� ������� ���������� (private members)
	int csd;								// TCP-����� ���������
	int rc;									// ��������� ������� �������� ������ (������ ����� GATEWAY_SIMPLE)
	pthread_t	tid_srvr; 		// ������������� ������ � ������� (������ ����� GATEWAY_SIMPLE)

	} GW_Client;

///=== CLIENTS_H public variables

  GW_Security Security;
	GW_Client	Client[MOXAGATE_CLIENTS_NUMBER];

///=== CLIENTS_H public functions

  int init_clients();     // ������� �����������
  int close_clients();    // ������� ����������

  int check_Security();
  int init_main_socket();
	int clear_client(int client);

  int gateway_common_processing();

#endif  /* CLIENTS_H */

/**** SNMP notes ***************************************************************
Name: Name to refer to the device. Your system administrator may use a specific naming convention. 
Note: The maximum length of the entry is 255 characters including spaces and punctuation. 
 
Contact: Person responsible for maintenance and operation of the device, which may be a network or facility administrator or the vendor from whom you purchased the device. 
Note: The maximum length of the entry is 64 characters including spaces and punctuation. 
 
Location: Description of the location of the device. 
Note: The maximum length of the entry is 64 characters including spaces and punctuation. 
 
Description: Other information useful for record keeping or quick reference. 
Note: The maximum length of the entry is 64 characters including spaces and punctuation but maybe longer depending on the device. 
 
Note: The values described above can be composed of printable characters with the exception of a double quote.  

********************************************************************************
	RFC1213-MIB	sysDescr.0	Uninitialized
RFC1213-MIB	sysObjectID.0	liebertGlobalProducts
RFC1213-MIB	sysUpTime.0	56 minutes, 25 seconds
	RFC1213-MIB	sysContact.0	Uninitialized
	RFC1213-MIB	sysName.0	nb214_ibp1_liebert_bottom
	RFC1213-MIB	sysLocation.0	Uninitialized
RFC1213-MIB	sysServices.0	72

  // ���� ���������� SNMP, RFC1213-MIB
  char sysDescr[64];     //.0	Uninitialized
  char sysObjectID[64];  //.0	liebertGlobalProducts
  char sysUpTime[64];    //.0	56 minutes, 25 seconds
    char sysContact[64]; //.0	Uninitialized
  char sysName[256];     //.0	nb214_ibp1_liebert_bottom
  char sysLocation[64];  //.0	Uninitialized
    char sysServices[64];//.0	72

*******************************************************************************/
