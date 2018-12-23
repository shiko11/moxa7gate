/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010                 
***************************************/

#ifndef STATISTICS_H
#define STATISTICS_H

///*************  ������ ����� ���������� ������ *******************************

///=== STATISTICS_H constants

// ���������������� ��������� ������� ����������

#define MB_SCAN_RATE_INFINITE			100000

#define STAT_FUNC_0x01		0
#define STAT_FUNC_0x02		1
#define STAT_FUNC_0x03		2
#define STAT_FUNC_0x04		3
#define STAT_FUNC_0x05		4
#define STAT_FUNC_0x06		5
#define STAT_FUNC_0x0f		6
#define STAT_FUNC_0x10		7
#define STAT_FUNC_OTHER		8
#define STAT_FUNC_AMOUNT	9

#define STAT_RES_OK			0
#define STAT_RES_ERR		1
#define STAT_RES_EXP		2
#define STAT_RES_AMOUNT 3

// ���� ������������
#define MBCOM_RTU_RECV	1
#define MBCOM_RTU_SEND	2
#define MBCOM_TCP_RECV	3
#define MBCOM_TCP_SEND	4
#define MBCOM_FRWD_REQ	5
#define MBCOM_QUEUE_OUT	6
#define MBCOM_FRWD_RSP	7

// ���� ������������ ������
#define MBCOM_REQ	1
#define MBCOM_RSP	2

///=== STATISTICS_H data types

/// ��������� ��� �������� ���������� ������ �� ����������, ������� � �������,
/// ��� �������� ������ ��� � ����������� ��������, ��� � ����������� �� �������� ModBus

///!!! ���������� ���������� � ���� ��������:
//	- �� ������� ����������� �������/������;
//	- �� �������� Modbus;
//	- �� ��������� ������: �������, ����������, �������

typedef struct {

	//--- ����� ���������� �� ����������� �������� ---

	unsigned int accepted; // ����� �������� �������
	unsigned int frwd_p;   // ����� �������� �������������� � ������ FRWD_TYPE_PROXY
	unsigned int frwd_a;   // ����� �������� �������������� � ������ FRWD_TYPE_ADDRESS
	unsigned int frwd_r;   // ����� �������� �������������� � ������ FRWD_TYPE_REGISTER
	unsigned int errors;   // ����� ������� �� ���������� (������ ���������� �������)
	unsigned int sended;   // ����� ������� ���������� (������ ���������� �������)

	//--- ��������� �������������� ������ ---

	// ����� ��������� ���������� �������, ����
	unsigned int proc_time;
	unsigned int proc_time_min;
	unsigned int proc_time_max;
	
	// �������� ����� ����� ���������� ���������, ����
	unsigned int poll_time;
	unsigned int poll_time_min;
	unsigned int poll_time_max;
	
	//--- ����������� �� ������� ����������� ---

	unsigned int tcp_req_recv;	// ��������� ������� �� TCP
	unsigned int tcp_req_adu;		// �������� ������������ ADU TCP �������
	unsigned int tcp_req_pdu;		// �������� ������������ PDU TCP �������
	unsigned int rtu_req_recv;	// ��������� ������� �� RTU
	unsigned int rtu_req_crc;		// �������� ����������� ����� RTU �������
	unsigned int rtu_req_adu;		// �������� ������������ ADU RTU �������
	unsigned int rtu_req_pdu;		// �������� ������������ PDU RTU �������

	unsigned int frwd_proxy;			// ��������������� ���� FRWD_TYPE_PROXY
	unsigned int frwd_atm;				// ��������������� ���� FRWD_TYPE_ADDRESS
	unsigned int frwd_rtm;				// ��������������� ���� FRWD_TYPE_REGISTER
	unsigned int frwd_queue_in;		// ���������� ������� � �������
	unsigned int frwd_queue_out;	// ���������� ������� �� �������

	unsigned int rtu_req_send;		// �������� ������� RTU �������
	unsigned int rtu_rsp_recv;		// ��������� ������ �� RTU �������
	unsigned int rtu_rsp_timeout;	// ������� �������� ������ �� RTU �������
	unsigned int rtu_rsp_crc;			// �������� ����������� ����� RTU ������
	unsigned int rtu_rsp_adu;			// �������� ������������ ADU RTU ������
	unsigned int rtu_rsp_pdu;			// �������� ������������ PDU RTU ������

	unsigned int tcp_req_send;		// �������� ������� TCP �������
	unsigned int tcp_rsp_recv;		// ��������� ������ �� TCP �������
	unsigned int tcp_rsp_timeout;	// ������� �������� ������ �� TCP �������
	unsigned int tcp_rsp_adu;			// �������� ������������ ADU TCP ������
	unsigned int tcp_rsp_pdu;			// �������� ������������ PDU TCP ������

	unsigned int frwd_rsp;			// ��������������� ������ �������
	unsigned int tcp_rsp_send;	// �������� ������ TCP �������
	unsigned int rtu_rsp_send;	// �������� ������ RTU �������

	//--- ����������� �� �������� ModBus ---
	unsigned int func[STAT_FUNC_AMOUNT][STAT_RES_AMOUNT];

	} GW_StaticData;

///=== STATISTICS_H public variables

///=== STATISTICS_H public functions

int init_statistics_h();

void copy_stat(GW_StaticData *dst, GW_StaticData *src);
void update_stat(GW_StaticData *dst, GW_StaticData *src);
void clear_stat(GW_StaticData *dst);

void func_res_ok(int mbf, GW_StaticData *dst);
void func_res_err(int mbf, GW_StaticData *dst);

void stage_to_stat(int mbcom, GW_StaticData *dst);

#endif  /* STATISTICS_H */
