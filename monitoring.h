
#ifndef MONITORING_H
#define MONITORING_H

#include "global.h"

#define EVENT_LOG_LENGTH			64
#define EVENT_MESSAGE_LENGTH	32

#define MAX_LATENCY_HISTORY_POINTS	8
#define GATE_WEB_INTERFACE_TIMEOUT	2

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

typedef struct { // net initsializatsii polej structury
	unsigned int accepted;

	unsigned int errors_input_communication; //#
	unsigned int errors_tcp_adu;
	unsigned int errors_tcp_pdu;						// est' detalisatsiya po modbus-funktsiyam v zhurnale

	unsigned int errors_serial_sending;				//# MB_SERIAL_WRITE_ERR
	unsigned int errors_serial_accepting;  //#

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
	
	unsigned int func[STAT_FUNC_AMOUNT][STAT_RES_AMOUNT];
	} GW_StaticData;

#define EVENT_SOURCE_P1				0
#define EVENT_SOURCE_P2				1
#define EVENT_SOURCE_P3				2
#define EVENT_SOURCE_P4				3
#define EVENT_SOURCE_P5				4
#define EVENT_SOURCE_P6				5
#define EVENT_SOURCE_P7				6
#define EVENT_SOURCE_P8				7
#define EVENT_SOURCE_SYSTEM		16
#define EVENT_SOURCE_GATE502	17

typedef struct {				// ЗАПИСЬ ЖУРНАЛА СОБЫТИЙ ШЛЮЗА
	time_t	time;					// время возникновения события
	unsigned int source;  // источник (аппаратное устройство или объект программы)
	unsigned int code;		// дополнительная информация о событии (например, код ошибки)
	char desc[EVENT_MESSAGE_LENGTH]; // proverit' dlinu soobscheniya functsii perror()
	} GW_EventLog;									 // текст сообщения


//unsigned app_log_current_entry, app_log_entries_total;
GW_EventLog *app_log;
char eventmsg[2*EVENT_MESSAGE_LENGTH]; /// создать список сообщений, генерируемых системой

void sysmsg(int source, int code, char *string, int show_anyway);
void copy_stat(GW_StaticData *dst, GW_StaticData *src);
void update_stat(GW_StaticData *dst, GW_StaticData *src);
void clear_stat(GW_StaticData *dst);
void func_res_ok(int mbf, GW_StaticData *dst);
void func_res_err(int mbf, GW_StaticData *dst);

#endif  /* MONITORING_H */
