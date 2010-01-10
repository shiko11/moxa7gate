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

//УФБФХУЩ ФПЮЕЛ
#define   PSTAT_NO_INVOLVED        0x00 //ФПЮЛБ ОЕ ЪБДЕКУФЧПЧБОБ Ч ЛПОЖЙЗХТБГЙПООПН УЛТЙРФЕ
#define   PSTAT_INVOLVED           0x01 //ФПЮЛБ ЪБДЕКУФЧПЧБОБ Ч ЛПОЖЙЗХТБГЙПООПН УЛТЙРФЕ
#define   PSTAT_AUTHENTIC          0x02 //ДБООЩЕ, УЮЙФБООЩЕ У ФПЮЛЙ - ДПУФПЧЕТОЩЕ

/* Predel'nye ТБЪНЕТЩ */
#define		MB_TCP_MAX_ADU_LENGTH		260		/* НБЛУЙНБМШОБС ДМЙОБ РБЛЕФБ TCP MODBUS ADU РТЙОЙНБЕНПЗП oФ OPC */
#define 	MB_TCP_ADU_HEADER_LEN		7
#define MB_SERIAL_MAX_ADU_LEN			256       // НБЛУЙНБМШОБС ДМЙОБ РБЛЕФБ-ПФЧЕФБ
#define MB_SERIAL_MIN_ADU_LEN			3              // НЙОЙНБМШОБС ДМЙОБ РБЛЕФБ-ПФЧЕФБ  (7

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

typedef struct { // параметры клиентского устройства
	u8        p_num;						// номер последовательного порта

	u8        c_num;						// порядковый номер, 0..MAX_TCP_CLIENTS_PER_PORT-1
	time_t	connection_time;		// время подключения
	time_t	last_activity_time;		// время последнего запроса

	u8        mb_slave;					// адрес modbus-устройства для перенаправления запросов в режиме BRIDGE
	unsigned int ip;						
	unsigned int port;

	int connection_status;	// состояние TCP-соединения
	int csd;								// TCP-сокет принятого или созданного соединения

	int rc;									// результат функции создания потока (только режим GATEWAY)
	pthread_t	tid_srvr; 		// идентификатор потока в системе (только режим GATEWAY)

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
#define SERIAL_STUB 0xff
typedef struct { // параметры последовательного порта
	int				fd;						// идентификатор
	u8        p_num;				// номер порта MOXA
	char      p_name[12];		// имя порта (устройства в системе)
	char      p_mode[12];		// режим 232/485_2W/422/485_4W
	char			speed[12];		// скорость обмена
	char			parity[12];		// контроль четности
	int		timeout;					// таймаут связи

	int ch_interval_timeout;
	} GW_SerialLine;

#define   MAX_MOXA_PORTS		8
#define MODBUS_GATEWAY_MODE	1
#define MODBUS_PROXY_MODE	2
#define MODBUS_BRIDGE_MODE	3
#define MODBUS_PORT_ERROR		4
#define MODBUS_PORT_OFF			5
#define   MAX_GATEWAY_QUEUE_LENGTH	16
//УФТХЛФХТБ ЛПОЖЙЗХТЙТХЕНЩИ ФПЮЕЛ Й РПТФПЧ
typedef struct {
	u16				tcp_port;				// номер TCP-порта для приема входящих клиентских соединений
	int				ssd;						// TCP-порт для приема входящих клиентских соединений
	int current_client;				// номер свободного соединения

	GW_Client	clients[MAX_MOXA_PORTS*MAX_TCP_CLIENTS_PER_PORT];
	} input_cfg_502;

typedef struct {
	GW_SerialLine serial;

	u16				tcp_port;				// номер TCP-порта для приема входящих клиентских соединений
	int				ssd;						// TCP-порт для приема входящих клиентских соединений
	int current_client;				// номер свободного соединения
	char bridge_status[4];		// состояние шлюза
	int modbus_mode;  				// режим шлюза

	GW_Client	clients[MAX_TCP_CLIENTS_PER_PORT];
  GW_StaticData stat;
	time_t start_time;  // время последней инициализации порта

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

#define MAX_VIRTUAL_SLAVES 64
/// пока определия этих блоков адресов относится к Holding-регистрам modbus и функции 06
typedef struct {			// блок регистров внутреннего адресного пространства шлюза
	unsigned start;			// начальный регистр диапазона адресного пространства шлюза
	unsigned length;		// количество регистров дипазона адресного пространства шлюза
	unsigned port;			// последовательный порт шлюза для перенаправления запроса
	unsigned device;		// адрес устройства в сети modbus для перенаправления запроса

  unsigned modbus_table; // одна из 4-х стандартных таблиц протокола MODBUS (см. файл modbus_rtu.h)
  unsigned offset;  // смещение в адресном пространстве устройства modbus (чтобы читать регистры не с нуля)
  } RT_Table_Entry;

#define MAX_QUERY_ENTRIES 64
#define GATE_STAT_AREA 49
typedef struct {			// modbus-запрос для циклического опроса ведомого устройства в режиме PROXY
	unsigned start;			// начальный регистр диапазона адресного пространства ведомого устройства
	unsigned length;		// количество регистров в диапазоне адресов ведомого устройства
  unsigned offset;  // смещение в адресном пространстве шлюза, с которого запивывать полученные данные
	unsigned port;			// последовательный порт шлюза для опроса ведомого устройства
	unsigned device;		// адрес ведомого устройства в сети modbus

  unsigned mbf; 			// одна из стандартных функций протокола MODBUS (см. файл modbus_rtu.h)
  unsigned delay;  // задержка перед отправкой запроса при работе в режиме PROXY в милисекундах
  unsigned status_register;  // номер регистра статуса опроса
  unsigned status_bit;  // номер бита статуса
  } Query_Table_Entry;

input_cfg		iDATA[MAX_MOXA_PORTS];        //НБУУЙЧ РБТБНЕФТПЧ УЛПОЖЙЗХТЙТПЧБООЩИ ФПЮЕЛ ()
input_cfg_502 gate502;
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
time_t start_time;  // время запуска программы
RT_Table_Entry vslave[MAX_VIRTUAL_SLAVES];
Query_Table_Entry query_table[MAX_QUERY_ENTRIES];

#define PROXY_MODE_REGISTERS 1024
	u16	*oDATA;          //НБУУЙЧ ЧЙТФХБМШОЩИ ТЕЗЙУФТПЧ нпиб
	
/// управление клиентскими соединениями
/// управление запросами modbus (переадресация)
/// управление запросами modbus (синхронизация и буферизация)


//ЖМБЗЙ ТЕЦЙНПЧ
//u8                       _cl;                                    //ЖМБЗ ЧЛМАЮЕОЙС ТЕЦЙНБ ПФМБДЛЙ ЛПНБОДОПК УФТПЛЙ
//u8                       _mb_rtu;                                //ЖМБЗ ЧЛМАЮЕОЙС ТЕЦЙНБ ПФМБДЛЙ Modbus RTU
//u8                       _mb_tcp;                                //ЖМБЗ ЧЛМАЮЕОЙС ТЕЦЙНБ ПФМБДЛЙ Modbus TCP
//u8                       _idata;                                 //ЖМБЗ ЧЛМАЮЕОЙС ТЕЦЙНБ ПФМБДЛЙ УФБФХУБ ФПЮЕЛ
//u8                       _odata;                                 //ЖМБЗ ЧЛМАЮЕОЙС ТЕЦЙНБ ПФМБДЛЙ ЧЙТФХБМШОЩИ 4И ТЕЗЙУФТПЧ нпиб
//u8                       _work;                                  //ЖМБЗ ЧЛМАЮЕОЙС ТЕЦЙНБ ПФМБДЛЙ (РПЛБ ОЕ ЪБДЕКУФЧПЧБО)
//u8                       _tcp_crc_off;                      //ЖМБЗ ПФЛМАЮЕОЙС ЖПТНЙТПЧБОЙС Й РТПЧЕТЛЙ CRC16 Ч Modbus TCP
//u8                       _rtu_crc_off;                      //ЖМБЗ ПФЛМАЮЕОЙС ЖПТНЙТПЧБОЙС Й РТПЧЕТЛЙ CRC16 Ч Modbus RTU
u8					_show_data_flow;
u8					_show_sys_messages;
u8					_single_gateway_port_502;
u8					_single_address_space;
u8					_proxy_mode;

//#define   MIN_CL_PARAM             14        //НЙОЙНБМШОПЕ ЛПМЙЮЕУФЧП ЛПОЖЙЗХТБГЙПООЩИ РБТБНЕФТПЧ ЛПНБОДОПК УФТПЛЙ
#define   MAX_KEYS                 10        //НБЛУЙНБМШОПЕ ЛПМЙЮЕУФЧП РБТБНЕФТПЧ ЛПНБОДОПК УФТПЛЙ У РТЕЖЙЛУПН "--"
//ПЫЙВЛЙ ЙОФЕТРТЕФБГЙЙ ЛПНБОДОПК УФТПЛЙ
#define   CL_ERR_NONE_PARAM        -1    //ЛПНБОДОБС УФТПЛБ РХУФБ
#define   CL_INFO                  -5
#define   CL_ERR_PORT_WORD         -2   //ПЫЙВЛБ Ч ЛМАЮЕЧЩИ УМПЧБИ // keyword PORT absent
#define   CL_ERR_IN_STRUCT         -3   //ПЫЙВЛБ Ч ДБООЩИ ЛПНБОДОПК УФТПЛЙ
#define   CL_ERR_IN_PORT_SETT      -4   //ПЫЙВЛБ Ч ДБООЩИ ЛПНБОДОПК УФТПЛЙ
#define   CL_ERR_GATEWAY_MODE      -6
#define   CL_ERR_IN_MAP						 -7
#define   CL_ERR_MIN_PARAM				 -8
#define   CL_ERR_MUTEX_PARAM			 -9
#define   CL_ERR_VSLAVES_CFG			 -10
#define   CL_ERR_QT_CFG						 -11
#define   CL_OK                    0    //ХУЕ ОПТНХМШ

//#define   CL_ERR_MIN_PARAM         -1   //НЕОЕЕ 9 РБТБНЕФТПЧ
//#define   CL_ERR_NUM_STAT_BIT			 -4 //ОПНЕТ ВЙФБ ВПМШЫЕ 16 (ДЙБРБЪПО 1-16)

int get_command_line (int 	argc,
											char	*argv[],
											input_cfg *ptr_iDATA,
											RT_Table_Entry *vslave,
											Query_Table_Entry *query_table,
											u8	*show_data_flow,
											u8	*show_sys_messages,
											u8	*single_gateway_port_502,
											u8  *single_address_space,
											u8  *proxy_mode
											);

//void		*test_idata(void *arg);
//void		*test_odata(void *arg);
//void		*run_work(void *arg);

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


#endif  /* GLOBAL_H */
