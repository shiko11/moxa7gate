/*

*/
#ifndef GLOBAL_H
#define GLOBAL_H

#include <pthread.h>
#include <sys/time.h>
#include <sys/socket.h>

#include "modbus_rtu.h"
#include "mx_keypad_lcm.h"
#include "mxlib/mxwdg.h"

//������� �����
#define   PSTAT_NO_INVOLVED        0x00 //����� �� ������������� � ���������������� �������
#define   PSTAT_INVOLVED           0x01 //����� ������������� � ���������������� �������
#define   PSTAT_AUTHENTIC          0x02 //������, ��������� � ����� - �����������

/* Predel'nye ������� */
#define		MB_TCP_MAX_ADU_LENGTH		260		/* ������������ ����� ������ TCP MODBUS ADU ������������ o� OPC */
#define 	MB_TCP_ADU_HEADER_LEN		7
#define MB_SERIAL_MAX_ADU_LEN			256       // ������������ ����� ������-������
#define MB_SERIAL_MIN_ADU_LEN			3              // ����������� ����� ������-������  (7

#define MB_FUNCTIONS_IMPLEMENTED 8
#define MB_FUNCTION_0x01	0
#define MB_FUNCTION_0x02	1
#define MB_FUNCTION_0x03	2
#define MB_FUNCTION_0x04	3
#define MB_FUNCTION_0x05	4
#define MB_FUNCTION_0x06	5
#define MB_FUNCTION_0x0f	6
#define MB_FUNCTION_0x10	7

#define MAX_LATENCY_HISTORY_POINTS	8

typedef struct { // net initsializatsii polej structury
	unsigned int accepted;

	unsigned int errors_input_communication;
	unsigned int errors_tcp_adu;
	unsigned int errors_tcp_pdu;						// est' detalisatsiya po modbus-funktsiyam v zhurnale

	unsigned int errors_serial_sending;				// MB_SERIAL_WRITE_ERR
	unsigned int errors_serial_accepting;

	unsigned int timeouts;

	unsigned int crc_errors;
	unsigned int errors_serial_adu;
	unsigned int errors_serial_pdu;						// est' detalisatsiya po modbus-funktsiyam v zhurnale

	unsigned int errors_tcp_sending;

	unsigned int errors;
	unsigned int sended;
	
	unsigned int request_time_min;
	unsigned int request_time_max;
	unsigned int request_time_average;				// algoritm usredneniya po n poslednih znachenij
	
	unsigned int scan_rate;				// algoritm usredneniya po n poslednih znachenij
	
	unsigned int latency_history[MAX_LATENCY_HISTORY_POINTS];
	unsigned int clp; // current latensy point
	
	unsigned int input_messages [MB_FUNCTIONS_IMPLEMENTED*2+1];
	unsigned int output_messages[MB_FUNCTIONS_IMPLEMENTED*2+1];
	} GW_StaticData;

#define EVENT_LOG_LENGTH			64
#define EVENT_MESSAGE_LENGTH	32
#define EVENT_SOURCE_SYSTEM		16
typedef struct {
	time_t	time;
	unsigned int source;
	unsigned int code;
	char desc[EVENT_MESSAGE_LENGTH]; // proverit' dlinu soobscheniya functsii perror()
	} GW_EventLog;

#define MAX_TCP_CLIENTS_PER_PORT	8
#define MB_CONNECTION_CLOSED			1
#define MB_CONNECTION_ESTABLISHED	2
#define MB_CONNECTION_ERROR				3
#define MB_SLAVE_NOT_DEFINED			0xff
#define MB_ADDRESS_NO_SHIFT				0
#define MB_SCAN_RATE_INFINITE			100000
typedef struct {
	u8        p_num;						// ����� ����������������� �����

	u8        c_num;						// ���������� �����, 0..MAX_TCP_CLIENTS_PER_PORT-1
	time_t	connection_time;		// ����� �����������

	u8        mb_slave;					// ����� modbus-���������� ��� ��������������� �������� � ������ BRIDGE
	unsigned int ip;						
	unsigned int port;

	int connection_status;	// ��������� TCP-����������
	int csd;								// TCP-���� ��������� ��� ���������� ����������

	int rc;									// ��������� ������� �������� ������ (������ ����� GATEWAY)
	pthread_t	tid_srvr; 		// ������������� ������ � ������� (������ ����� GATEWAY)

	GW_StaticData stat;
	
	int address_shift;
	} GW_Client;

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
typedef struct { // ��������� ����������������� �����
	int				fd;						// �������������
	u8        p_num;				// ����� ����� MOXA
	char      p_name[12];		// ��� ����� (���������� � �������)
	char      p_mode[12];		// ����� 232/485_2W/422/485_4W
	char			speed[12];		// �������� ������
	char			parity[12];		// �������� ��������
	int		timeout;					// ������� �����

	int ch_interval_timeout;
	} GW_SerialLine;

#define   MAX_MOXA_PORTS		8
#define MODBUS_GATEWAY_MODE	1
#define MODBUS_MASTER_MODE	2
#define MODBUS_BRIDGE_MODE	3
#define MODBUS_PORT_ERROR		4
#define MODBUS_PORT_OFF			5
//��������� ��������������� ����� � ������
typedef struct {
	GW_SerialLine serial;

	u16				tcp_port;				// ����� TCP-����� ��� ������ �������� ���������� ����������
	int				ssd;						// TCP-���� ��� ������ �������� ���������� ����������
	int current_client;				// ����� ���������� ����������
	char bridge_status[4];		// ��������� �����
	int modbus_mode;  				// ����� �����

	GW_Client	clients[MAX_TCP_CLIENTS_PER_PORT];
  GW_StaticData stat;
	time_t start_time;  // vremya zapuska programmy

  unsigned int accepted_connections_number;
  unsigned int  current_connections_number;
  unsigned int rejected_connections_number;
  
  pthread_mutex_t serial_mutex;

	u16	*oDATA;          //������ ����������� ��������� ����

	} input_cfg;

input_cfg		iDATA[MAX_MOXA_PORTS];        //������ ���������� ������������������ ����� ()
int mxlcm_handle;
int mxkpd_handle;
int mxbzr_handle;
int shm_segment_id;
GW_Display screen;
GW_EventLog app_log;
unsigned int buzzer_flag; // zummer daet 1, 2 i 3 gudka v zavisimosti ot kolichestva oshibok: <15, 15-30, >30
struct timeval tv;
struct timeval tv_mem;
struct timezone tz;
unsigned int p_errors[MAX_MOXA_PORTS];
char eventmsg[2*EVENT_MESSAGE_LENGTH];
	

//����� �������
//u8                       _cl;                                    //���� ��������� ������ ������� ��������� ������
//u8                       _mb_rtu;                                //���� ��������� ������ ������� Modbus RTU
//u8                       _mb_tcp;                                //���� ��������� ������ ������� Modbus TCP
//u8                       _idata;                                 //���� ��������� ������ ������� ������� �����
//u8                       _odata;                                 //���� ��������� ������ ������� ����������� 4� ��������� ����
//u8                       _work;                                  //���� ��������� ������ ������� (���� �� ������������)
//u8                       _tcp_crc_off;                      //���� ���������� ������������ � �������� CRC16 � Modbus TCP
//u8                       _rtu_crc_off;                      //���� ���������� ������������ � �������� CRC16 � Modbus RTU
u8					_show_data_flow;
u8					_show_sys_messages;

//#define   MIN_CL_PARAM             14        //����������� ���������� ���������������� ���������� ��������� ������
#define   MAX_KEYS                 10        //������������ ���������� ���������� ��������� ������ � ��������� "--"
//������ ������������� ��������� ������
#define   CL_ERR_NONE_PARAM        -1    //��������� ������ �����
#define   CL_INFO                  -5
#define   CL_ERR_PORT_WORD         -2   //������ � �������� ������ // keyword PORT absent
#define   CL_ERR_IN_STRUCT         -3   //������ � ������ ��������� ������
#define   CL_ERR_IN_PORT_SETT      -4   //������ � ������ ��������� ������
#define   CL_ERR_GATEWAY_MODE      -6
#define   CL_ERR_IN_MAP						 -7
#define   CL_ERR_MIN_PARAM				 -8
#define   CL_OK                    0    //��� �������

//#define   CL_ERR_MIN_PARAM         -1   //����� 9 ����������
//#define   CL_ERR_NUM_STAT_BIT			 -4 //����� ���� ������ 16 (�������� 1-16)

int get_command_line (int 	argc,
											char	*argv[],
											input_cfg *ptr_iDATA,
											u8	*show_data_flow,
											u8	*show_sys_messages
											);

void		*test_idata(void *arg);
void		*test_odata(void *arg);
void		*run_work(void *arg);

int 		mb_check_request_pdu(unsigned char *pdu, unsigned char len);
int 		mb_check_response_pdu(unsigned char *pdu, unsigned char len, unsigned char *request);


///--- shared memory operations
int init_shm();
int refresh_shm(void *arg);
int close_shm();
void sysmsg(int source, char *string, int show_anyway);

void update_stat(GW_StaticData *dst, GW_StaticData *src);
void clear_stat(GW_StaticData *dst);
int check_gate_settings(input_cfg *data);

#endif  /* GLOBAL_H */
