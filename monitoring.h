
#ifndef MONITORING_H
#define MONITORING_H

#include "global.h"

#define GATE_WEB_INTERFACE_TIMEOUT	2

///************* М Е Х А Н И З М   С Б О Р А   С Т А Т И С Т И К И ********************

#define MAX_LATENCY_HISTORY_POINTS	8

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

/// Структура для хранения статистики опроса по порту, клиенту и серверу,
/// она содержит данные как о результатах запросов, так и детализацию по функциям ModBus
typedef struct {

	//--- информация по результатам запросов ---
	unsigned int accepted; // количество принятых запросов

	unsigned int errors;

		unsigned int errors_input_communication;
		unsigned int errors_tcp_adu;
		unsigned int errors_tcp_pdu;
	
		unsigned int errors_serial_sending;
		unsigned int errors_serial_accepting;
	
		unsigned int timeouts;  // количество таймаутов
	
		unsigned int crc_errors; // количество ошибок при проверке контрольной суммы
		unsigned int errors_serial_adu;
		unsigned int errors_serial_pdu;
	
		unsigned int errors_tcp_sending;

	unsigned int sended;
																			 
	//--- детализация по функциям ModBus ---
	unsigned int func[STAT_FUNC_AMOUNT][STAT_RES_AMOUNT];

	//--- временнЫе характеристики опроса ---
	unsigned int request_time;	// время обработки последнего запроса
	unsigned int scan_rate;			// мгновенный период опроса

	///!!!--- нужен отдельный механизм сбора истории и вычисления статистических характеристик опроса
	/// который будет включаться отдельным ключом в конфигурационном скрипте и служить исключительно
	/// для выявления проблем в работе оборудования (подключение БД MySQL было бы здесь интересно)
	// кольцевой буфер результатов N последних запросов
//	unsigned int latency_history[MAX_LATENCY_HISTORY_POINTS]; 
//	unsigned int clp; // указатель на самое старое значение в буфере
	
//	unsigned int request_time_min;			///!!!
//	unsigned int request_time_max;			///!!!
//	unsigned int request_time_average;	///!!!
	
	} GW_StaticData;

void copy_stat(GW_StaticData *dst, GW_StaticData *src);
void update_stat(GW_StaticData *dst, GW_StaticData *src);
void clear_stat(GW_StaticData *dst);
void func_res_ok(int mbf, GW_StaticData *dst);
void func_res_err(int mbf, GW_StaticData *dst);

///******************* М Е Х А Н И З М   С О О Б Щ Е Н И Й ***************************
#define EVENT_LOG_LENGTH			100
#define EVENT_MESSAGE_LENGTH	81
#define EVENT_TEMPLATE_AMOUNT	0x100

#define EVENT_CAT_MONITOR			0x40
#define EVENT_CAT_DEBUG				0x80
#define EVENT_CAT_TRAFFIC			0xC0
#define EVENT_CAT_RESERV			0x00
#define EVENT_CAT_MASK				0xC0

#define EVENT_TYPE_INF				0x10
#define EVENT_TYPE_WRN				0x20
#define EVENT_TYPE_ERR				0x30
#define EVENT_TYPE_RESERV			0x00
#define EVENT_TYPE_MASK				0x30

#define EVENT_SRC_P1					0x00
#define EVENT_SRC_P2					0x01
#define EVENT_SRC_P3					0x02
#define EVENT_SRC_P4					0x03
#define EVENT_SRC_P5					0x04
#define EVENT_SRC_P6					0x05
#define EVENT_SRC_P7					0x06
#define EVENT_SRC_P8					0x07
#define EVENT_SRC_SYSTEM			0x08
#define EVENT_SRC_GATE502			0x09
#define EVENT_SRC_MOXAMB			0x0A
//#define   MOXA_MB_DEVICE		0x0A
#define EVENT_SRC_MOXATCP			0x0B
#define EVENT_SRC_TCPBRIDGE		0x0C
#define EVENT_SRC_RESERV1			0x0D
#define EVENT_SRC_RESERV2			0x0E
#define EVENT_SRC_RESERV3			0x0F
#define EVENT_SRC_MASK				0x0F

typedef struct {				// ЗАПИСЬ ЖУРНАЛА СОБЫТИЙ ШЛЮЗА
	time_t	time;					// время возникновения события

//	unsigned int source;  // источник (аппаратное устройство или объект программы)
//	unsigned int code;		// дополнительная информация о событии (например, код ошибки)
//	char desc[EVENT_MESSAGE_LENGTH]; // proverit' dlinu soobscheniya functsii perror()

	unsigned char msgtype;
	unsigned char msgcode;
	unsigned int prm[4];
	} GW_EventLog;									 // текст сообщения


GW_EventLog *app_log;
char eventmsg[EVENT_MESSAGE_LENGTH];
char message_template[EVENT_TEMPLATE_AMOUNT][EVENT_MESSAGE_LENGTH];

struct timeval msg_tv;
struct timezone msg_tz;

void sysmsg_ex(unsigned char msgtype, unsigned char msgcode,
								unsigned int prm1,
								unsigned int prm2,
								unsigned int prm3,
								unsigned int prm4);

#define TRAFFIC_RTU_RECV	182
#define TRAFFIC_RTU_SEND	183
#define TRAFFIC_TCP_RECV	184
#define TRAFFIC_TCP_SEND	185
#define TRAFFIC_QUEUE_IN	186
#define TRAFFIC_QUEUE_OUT	187

void show_traffic(int traffic, int port_id, int client_id, u8 *adu, u16 adu_len);

void make_msgstr(	unsigned char msgcode, char *str,
									unsigned int prm1,
									unsigned int prm2,
									unsigned int prm3,
									unsigned int prm4);

///*************************************************************************************

#endif  /* MONITORING_H */
