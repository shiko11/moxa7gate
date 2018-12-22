/*
MOXA7GATE MODBUS GATEWAY SOFTWARE
SEM-ENGINEERING
                    BRYANSK 2009
*/

#ifndef GLOBAL_H
#define GLOBAL_H

#include <pthread.h>
#include <sys/time.h>
#include <sys/socket.h>

#include <errno.h>
#include <fcntl.h>

#include <netinet/in.h>
#include <signal.h>

//#include <sys/ipc.h>
#include <sys/sem.h>
//#include <sys/types.h>
union semun {
	int val;
	struct semid_ds *buf;
	unsigned short int *array;
	struct seminfo *__buf;
  };
int semaphore_id;

#include "modbus_rtu.h"
#include "mx_keypad_lcm.h"
#include "mxlib/mxwdg.h"
#include "monitoring.h"

//������� �����
#define   PSTAT_NO_INVOLVED        0x00 //����� �� ������������� � ���������������� �������
#define   PSTAT_INVOLVED           0x01 //����� ������������� � ���������������� �������
#define   PSTAT_AUTHENTIC          0x02 //������, ��������� � ����� - �����������

/* Predel'nye ������� */
#define		MB_TCP_MAX_ADU_LENGTH		260		/* ������������ ����� ������  TCP MODBUS ADU ������������ �� OPC */
#define 	MB_TCP_ADU_HEADER_LEN		7
#define MB_SERIAL_MAX_ADU_LEN			256       // ������������ ����� ������-������
#define MB_SERIAL_MIN_ADU_LEN			3              // ����������� ����� ������-������ (7

#define MB_FUNCTIONS_IMPLEMENTED 8
#define MB_FUNCTION_0x01	0
#define MB_FUNCTION_0x02	1
#define MB_FUNCTION_0x03	2
#define MB_FUNCTION_0x04	3
#define MB_FUNCTION_0x05	4
#define MB_FUNCTION_0x06	5
#define MB_FUNCTION_0x0f	6
#define MB_FUNCTION_0x10	7

#define MAX_TCP_CLIENTS_PER_PORT	8
#define MB_CONNECTION_CLOSED			1
#define MB_CONNECTION_ESTABLISHED	2
#define MB_CONNECTION_ERROR				3
#define MB_SLAVE_NOT_DEFINED			0xff
#define MB_ADDRESS_NO_SHIFT				0
#define MB_SCAN_RATE_INFINITE			100000
#define MAX_CLIENT_ACTIVITY_TIMEOUT	30

#define DEVICE_NAME_LENGTH	64

typedef struct { // ��������� ����������� ����������
	u8        mb_slave;					// ����� modbus-���������� ��� ��������������� �������� � ������ BRIDGE
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

	int rc;									// ��������� ������� �������� ������ (������ ����� GATEWAY)
	pthread_t	tid_srvr; 		// ������������� ������ � ������� (������ ����� GATEWAY)

	GW_StaticData stat;
	
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

#define PROXY_TCP	8

#define SERIAL_STUB 0xff
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
#define   MOXA_MB_DEVICE		8

#define MODBUS_GATEWAY_MODE	1	///###obsolete
#define MODBUS_PROXY_MODE		2	///###obsolete
#define MODBUS_BRIDGE_MODE	3	///###obsolete

#define GATEWAY_SIMPLE			6
#define GATEWAY_ATM					7
#define GATEWAY_RTM					8
#define GATEWAY_PROXY				9
#define BRIDGE_PROXY				10
#define BRIDGE_SIMPLE				11

#define MODBUS_PORT_ERROR		4
#define MODBUS_PORT_OFF			5

#define   MAX_GATEWAY_QUEUE_LENGTH	16
#define   GATE_STATUS_BLOCK_LENGTH	24

//��������� ��������������� ����� � ������
typedef struct {
	int				ssd;						// TCP-���� ��� ������ �������� ���������� ����������
	int current_client;				// ����� ���������� ����������

unsigned		tcp_port;	// ����� TCP-����� ��� ������ � ����������������� ������������ ����� � ������� ATM, RTM
u8					modbus_address; // ����������� Modbus-����� ����� ��� ����������� � ���������� ���������� HMI ������
unsigned		status_info; // ��������� ����� ����� ����������� ��������� ����� (�� ��������� ���������� � �������)

char object[DEVICE_NAME_LENGTH];			// ������������ ������� �������������
char location[DEVICE_NAME_LENGTH];		// ����� ��������� ���������� MOXA UC-7410
char version[DEVICE_NAME_LENGTH];			// ������ ����������������� �����
char networkName[DEVICE_NAME_LENGTH];	// ������� ��� ����������
unsigned int IPAddress;								// ������� �����

u8 show_data_flow;		// ���������� ������ ������ � ������ Telnet
u8 show_sys_messages;	// ���������� ��������� ��������� � ������ Telnet
u8 watchdog_timer;		// ������������ Watchdog-������
u8 use_buzzer;				// ����������� ������ ��� ������������ ������

  pthread_mutex_t moxa_mutex;

	u8			queue_adu[MAX_GATEWAY_QUEUE_LENGTH][MB_TCP_MAX_ADU_LENGTH];// TCP ADU
	u16			queue_adu_len[MAX_GATEWAY_QUEUE_LENGTH];
	u16			queue_clients[MAX_GATEWAY_QUEUE_LENGTH];
	u16			queue_slaves[MAX_GATEWAY_QUEUE_LENGTH];
	int			queue_start, queue_len;

	GW_Client	clients[MAX_MOXA_PORTS*MAX_TCP_CLIENTS_PER_PORT];
  GW_StaticData stat;

	time_t start_time;  // ����� ������� ���������
	time_t timestamp;  // ����� ���������� ���������� ������ ��� web-����������
	unsigned app_log_current_entry, app_log_entries_total;
	char halt;
	} input_cfg_502;

typedef struct {
	GW_SerialLine serial;

	u16				tcp_port;				// ����� TCP-����� ��� ������ �������� ���������� ����������
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

	u8			queue_adu[MAX_GATEWAY_QUEUE_LENGTH][MB_TCP_MAX_ADU_LENGTH];// TCP ADU
	u16			queue_adu_len[MAX_GATEWAY_QUEUE_LENGTH];
	u16			queue_clients[MAX_GATEWAY_QUEUE_LENGTH];
	u16			queue_slaves[MAX_GATEWAY_QUEUE_LENGTH];
	int			queue_start, queue_len;

	} input_cfg;

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
  unsigned offset;  // �������� � �������� ������������ �����, � �������� ���������� ���������� ������
	unsigned port;			// ���������������� ���� ����� ��� ������ �������� ����������
	unsigned device;		// ����� �������� ���������� � ���� modbus

  unsigned mbf; 			// ���� �� ����������� ������� ��������� MODBUS (��. ���� modbus_rtu.h)
  unsigned delay;  // �������� ����� ��������� ������� ��� ������ � ������ PROXY � ������������
	unsigned critical;
  unsigned status_register;  // ����� �������� ������� ������
  unsigned status_bit;  // ����� ���� �������
	char device_name[DEVICE_NAME_LENGTH]; // ������������ ����������
  } Query_Table_Entry;

#define MAX_TCP_SERVERS 32
typedef struct { // ������ ������� TCP_SERVERS
	u8        mb_slave;					// ����� modbus-���������� ��� ��������������� �������� � ������ BRIDGE
	unsigned int ip;						// ������� �����
	unsigned int port;					// TCP ����
	int address_shift;					// �������� ��������� ������������
	u8        p_num;						// ����� �����, ������������� � ���� �������
	char device_name[DEVICE_NAME_LENGTH]; // ������������ ����������
	} GW_TCP_Server;

input_cfg		iDATA[MAX_MOXA_PORTS];        //������ ���������� ������������������ ����� ()
input_cfg_502 gate502;
int mxlcm_handle;
int mxkpd_handle;
int mxbzr_handle;
int shm_segment_id;
GW_Display screen;
unsigned int buzzer_flag; // ������ ���� 1, 2 � 3 ����� � ����������� �� ���������� ������: <15, 15-30, >30
struct timeval tv;
struct timeval tv_mem;
struct timezone tz;
unsigned int p_errors[MAX_MOXA_PORTS];

fd_set watchset;
fd_set inset;

RT_Table_Entry vslave[MAX_VIRTUAL_SLAVES];
Query_Table_Entry query_table[MAX_QUERY_ENTRIES];
GW_TCP_Server tcp_servers[MAX_TCP_SERVERS];

#define GATE_STATUS_BLOCK_LENGHT 128

#define PROXY_MODE_REGISTERS 0 ///###obsolete
u16	*oDATA; ///###obsolete

// ������ ����� ���������� ��������� Moxa (��������)
unsigned offset1xStatus, offset2xStatus, offset3xRegisters, offset4xRegisters;
// ���������� ��������� ��� ������ �� ������
unsigned amount1xStatus, amount2xStatus, amount3xRegisters, amount4xRegisters;
// ��������� �� ������� ������
u16	*wData1x; //������ ����������� 1x ��������� MOXA (coil status)
u16	*wData2x; //������ ����������� 2x ��������� MOXA (input status)
u16	*wData3x; //������ ����������� 3x ��������� MOXA (input register)
u16	*wData4x; //������ ����������� 4x ��������� MOXA (holding register)
	
/// ���������� ����������� ������������
/// ���������� ��������� modbus (�������������)
/// ���������� ��������� modbus (������������� � �����������)


//����� �������
//u8                       _tcp_crc_off;                      //���� ���������� ������������ � �������� CRC16 � Modbus TCP
//u8                       _rtu_crc_off;                      //���� ���������� ������������ � �������� CRC16 � Modbus RTU
u8					_show_data_flow;
u8					_show_sys_messages;
u8					_single_gateway_port_502;
u8					_single_address_space;
u8					_proxy_mode;

//#define   MIN_CL_PARAM             14        //����������� ���������� ���������������� ���������� ��������� ������
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

//void		*test_idata(void *arg);
//void		*test_odata(void *arg);
//void		*run_work(void *arg);

#define TCPADU_SIZE_HI		4
#define TCPADU_SIZE_LO		5
#define TCPADU_ADDRESS		6
#define TCPADU_FUNCTION		7
#define TCPADU_START_HI		8
#define TCPADU_START_LO		9
#define TCPADU_LEN_HI			10
#define TCPADU_LEN_LO			11

#define RTUADU_SIZE_HI		123
#define RTUADU_SIZE_LO		123
#define RTUADU_ADDRESS		0
#define RTUADU_FUNCTION		1
#define RTUADU_START_HI		123
#define RTUADU_START_LO		123
#define RTUADU_LEN_HI			123
#define RTUADU_LEN_LO			123

int 		mb_check_request_pdu(unsigned char *pdu, unsigned char len);
int 		mb_check_response_pdu(unsigned char *pdu, unsigned char len, unsigned char *request);
int			enqueue_query(int port_id, int client_id, int device_id, u8 *tcp_adu, u16 tcp_adu_len);


///--- shared memory operations
int init_shm();
int refresh_shm(void *arg);
int close_shm();
int check_gate_settings(input_cfg *data);

void sigpipe_handler();
void sigio_handler();

int gateway_common_processing();
int gateway_single_port_processing();
int query_translating();

#define MOXA_DIAPASON_UNDEFINED		0
#define MOXA_DIAPASON_INSIDE			1
#define MOXA_DIAPASON_OUTSIDE			2
#define MOXA_DIAPASON_OVERLAPPED	3
int checkDiapason(int function, int start_address, int length);
int get_ip_from_string(char *str, unsigned int *ip, unsigned int *port);

#endif  /* GLOBAL_H */
