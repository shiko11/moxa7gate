#include "global.h"

#define EVENT_LOG_LENGTH			64
#define EVENT_MESSAGE_LENGTH	32

#define MAX_LATENCY_HISTORY_POINTS	8

#define GATE_WEB_INTERFACE_TIMEOUT	2

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
	
	unsigned int ok_0x01, ok_0x02, ok_0x03, ok_0x04, ok_0x05, ok_0x06, ok_0x0f, ok_0x10, ok_0xXX;
	unsigned int err_0x01, err_0x02, err_0x03, err_0x04, err_0x05, err_0x06, err_0x0f, err_0x10, err_0xXX;
	unsigned int exp_0x01, exp_0x02, exp_0x03, exp_0x04, exp_0x05, exp_0x06, exp_0x0f, exp_0x10, exp_0xXX;

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
void update_stat(GW_StaticData *dst, GW_StaticData *src);
void clear_stat(GW_StaticData *dst);
