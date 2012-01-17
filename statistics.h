/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010                 
***************************************/

#ifndef STATISTICS_H
#define STATISTICS_H

///*************  МОДУЛЬ СБОРА СТАТИСТИКИ ОПРОСА *******************************

///=== STATISTICS_H constants

// конфигурационные константы времени компиляции

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

// типы коммуникации
#define MBCOM_RTU_RECV	1
#define MBCOM_RTU_SEND	2
#define MBCOM_TCP_RECV	3
#define MBCOM_TCP_SEND	4
#define MBCOM_FRWD_REQ	5
#define MBCOM_QUEUE_OUT	6
#define MBCOM_FRWD_RSP	7

// типы пересылаемых данных
#define MBCOM_REQ	1
#define MBCOM_RSP	2

///=== STATISTICS_H data types

/// Структура для хранения статистики опроса по интерфейсу, клиенту и серверу,
/// она содержит данные как о результатах запросов, так и детализацию по функциям ModBus

///!!! Статистика собирается в трех разрезах:
//	- по стадиям прохождения запроса/ответа;
//	- по функциям Modbus;
//	- по субъектам обмена: клиенты, интерфейсы, серверы

typedef struct {

	//--- общая информация по результатам запросов ---

	unsigned int accepted; // всего запросов принято
	unsigned int frwd_p;   // всего запросов перенаправлено в режиме FRWD_TYPE_PROXY
	unsigned int frwd_a;   // всего запросов перенаправлено в режиме FRWD_TYPE_ADDRESS
	unsigned int frwd_r;   // всего запросов перенаправлено в режиме FRWD_TYPE_REGISTER
	unsigned int errors;   // всего ответов не отправлено (запрос завершился ошибкой)
	unsigned int sended;   // всего ответов отправлено (запрос завершился успешно)

	//--- временнЫе характеристики опроса ---

	// время обработки последнего запроса, мсек
	unsigned int proc_time;
	unsigned int proc_time_min;
	unsigned int proc_time_max;
	
	// интервал между двумя последними запросами, мсек
	unsigned int poll_time;
	unsigned int poll_time_min;
	unsigned int poll_time_max;
	
	//--- детализация по стадиям прохождения ---

	unsigned int tcp_req_recv;	// получение запроса по TCP
	unsigned int tcp_req_adu;		// проверка корректности ADU TCP запроса
	unsigned int tcp_req_pdu;		// проверка корректности PDU TCP запроса
	unsigned int rtu_req_recv;	// получение запроса по RTU
	unsigned int rtu_req_crc;		// проверка контрольной суммы RTU запроса
	unsigned int rtu_req_adu;		// проверка корректности ADU RTU запроса
	unsigned int rtu_req_pdu;		// проверка корректности PDU RTU запроса

	unsigned int frwd_proxy;			// перенаправление типа FRWD_TYPE_PROXY
	unsigned int frwd_atm;				// перенаправление типа FRWD_TYPE_ADDRESS
	unsigned int frwd_rtm;				// перенаправление типа FRWD_TYPE_REGISTER
	unsigned int frwd_queue_in;		// постановка запроса в очередь
	unsigned int frwd_queue_out;	// извлечение запроса из очереди

	unsigned int rtu_req_send;		// отправка запроса RTU серверу
	unsigned int rtu_rsp_recv;		// получение ответа от RTU сервера
	unsigned int rtu_rsp_timeout;	// таймаут ожидания ответа от RTU сервера
	unsigned int rtu_rsp_crc;			// проверка контрольной суммы RTU ответа
	unsigned int rtu_rsp_adu;			// проверка корректности ADU RTU ответа
	unsigned int rtu_rsp_pdu;			// проверка корректности PDU RTU ответа

	unsigned int tcp_req_send;		// отправка запроса TCP серверу
	unsigned int tcp_rsp_recv;		// получение ответа от TCP сервера
	unsigned int tcp_rsp_timeout;	// таймаут ожидания ответа от TCP сервера
	unsigned int tcp_rsp_adu;			// проверка корректности ADU TCP ответа
	unsigned int tcp_rsp_pdu;			// проверка корректности PDU TCP ответа

	unsigned int frwd_rsp;			// перенаправление ответа клиенту
	unsigned int tcp_rsp_send;	// отправка ответа TCP клиенту
	unsigned int rtu_rsp_send;	// отправка ответа RTU клиенту

	//--- детализация по функциям ModBus ---
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
