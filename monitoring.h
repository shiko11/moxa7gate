#include "global.h"

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
	
//	unsigned int input_messages [MB_FUNCTIONS_IMPLEMENTED*2+1];
//	unsigned int output_messages[MB_FUNCTIONS_IMPLEMENTED*2+1];

	} GW_StaticData;

#define EVENT_LOG_LENGTH			64
#define EVENT_MESSAGE_LENGTH	32
#define EVENT_SOURCE_SYSTEM		16
#define EVENT_SOURCE_GATE502	17

typedef struct {
	time_t	time;
	unsigned int source;
	unsigned int code;
	char desc[EVENT_MESSAGE_LENGTH]; // proverit' dlinu soobscheniya functsii perror()
	} GW_EventLog;


void sysmsg(int source, char *string, int show_anyway);
void update_stat(GW_StaticData *dst, GW_StaticData *src);
void clear_stat(GW_StaticData *dst);
