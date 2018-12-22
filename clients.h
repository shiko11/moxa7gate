/*
MODBUS GATEWAY SOFTWARE
MOXA7GATE VERSION 1.2
SEM-ENGINEERING
					BRYANSK 2010
*/

#ifndef CLIENTS_H
#define CLIENTS_H

// ��������� ����������� ����������
#define MB_CONNECTION_CLOSED			1
#define MB_CONNECTION_ESTABLISHED	2
#define MB_CONNECTION_ERROR				3
#define MB_SLAVE_NOT_DEFINED			0xff
#define MB_ADDRESS_NO_SHIFT				0
#define MB_SCAN_RATE_INFINITE			100000

/*--- ����� �������� 4 ��������� ���� �������� �����:
	1. ������ �� ������� TCP, ������������ � ����� GATEWAY_SIMPLE;
	2. ������ �� ������� TCP, ������������ � ����� ����� ����� TCP ���� (������ 502);
	3. ������ �� ������� RTU, ������������ � ����� BRIDGE_PROXY;
	4. HMI ������;
*/

typedef struct { // ��������� ����������� ����������
	u8        mb_slave;					// ����� modbus-���������� ��� ��������������� �������� � ������ BRIDGE_SIMPLE
	unsigned int ip;						
	unsigned int port;
	int address_shift;
	u8        p_num;						// ����� �����, ������������� � ���� �������
	char device_name[DEVICE_NAME_LENGTH]; // ������������ ����������

	u8        c_num;						// ���������� �����, 0..MAX_TCP_CLIENTS_PER_PORT-1
	time_t	connection_time;		// ����� �����������
	time_t	last_activity_time;		// ����� ���������� �������

	int connection_status;	// ��������� TCP-����������
	int csd;								// TCP-����� ��������� ��� ���������� ����������

	int rc;									// ��������� ������� �������� ������ (������ ����� GATEWAY_SIMPLE)
	pthread_t	tid_srvr; 		// ������������� ������ � ������� (������ ����� GATEWAY_SIMPLE)

	GW_StaticData stat;
	
	} GW_Client;

#endif  /* CLIENTS_H */
