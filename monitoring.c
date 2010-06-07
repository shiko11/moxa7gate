#include "global.h"

void update_stat(GW_StaticData *dst, GW_StaticData *src)
	{
	dst->accepted+=src->accepted;
	dst->errors_input_communication+=src->errors_input_communication;
	dst->errors_tcp_adu+=src->errors_tcp_adu;
	dst->errors_tcp_pdu+=src->errors_tcp_pdu;
	dst->errors_serial_sending+=src->errors_serial_sending;
	dst->errors_serial_accepting+=src->errors_serial_accepting;
	dst->timeouts+=src->timeouts;
	dst->crc_errors+=src->crc_errors;
	dst->errors_serial_adu+=src->errors_serial_adu;
	dst->errors_serial_pdu+=src->errors_serial_pdu;
	dst->errors_tcp_sending+=src->errors_tcp_sending;
	dst->errors+=src->errors;
	dst->sended+=src->sended;

	return;
	}
	
void clear_stat(GW_StaticData *dst)
	{
	dst->accepted=\
	dst->errors_input_communication=\
	dst->errors_tcp_adu=\
	dst->errors_tcp_pdu=\
	dst->errors_serial_sending=\
	dst->errors_serial_accepting=\
	dst->timeouts=\
	dst->crc_errors=\
	dst->errors_serial_adu=\
	dst->errors_serial_pdu=\
	dst->errors_tcp_sending=\
	dst->errors=\
	dst->sended=0;

	return;
	}

void sysmsg(int source, int code, char *message, int show_anyway)
{
int i;

/// создаем запись о новом событии
if(app_log==NULL) {
  printf("error. event log is not initialized now. current message:\n%s\n", message);
  return;
  }

time(&app_log[gate502.app_log_current_entry].time);
app_log[gate502.app_log_current_entry].source=source;
app_log[gate502.app_log_current_entry].code=code;

for(i=0; i<EVENT_MESSAGE_LENGTH-1; i++) 
  app_log[gate502.app_log_current_entry].desc[i]=message[i];
app_log[gate502.app_log_current_entry].desc[EVENT_MESSAGE_LENGTH-1]=0;

gate502.app_log_entries_total++;

/// выводим событие на консоль

if(_show_sys_messages==1 || show_anyway==1) {

/*  if((source&0xff)<8) {
    if(((source>>8)&0xff)<8)
		  printf("PORT%d.CLIENT%d: %s\n", (source&0xff)+1, ((source>>8)&0xff)+1, string);
		  else printf("PORT%d         : %s\n", (source&0xff)+1, string);
	  } else switch (source&0xff) {
			case EVENT_SOURCE_GATE502:
				printf(" GATE502 #%4.4X: %s\n", ((source>>8)&0xff), string);
				break;
			default:
				printf(" SYSTEM #%4.4X: %s\n", ((source>>8)&0xff)+1, string);
			}*/

	struct tm *tmd;
	tmd=gmtime(&app_log[gate502.app_log_current_entry].time);
	printf("%2.2d.%2.2d.%4.4d %2.2d:%2.2d:%2.2d\t", tmd->tm_mday, tmd->tm_mon+1, tmd->tm_year+1900, tmd->tm_hour, tmd->tm_min, tmd->tm_sec);

	switch(app_log[gate502.app_log_current_entry].source) {
			case EVENT_SOURCE_P1:
			case EVENT_SOURCE_P2:
			case EVENT_SOURCE_P3:
			case EVENT_SOURCE_P4:
			case EVENT_SOURCE_P5:
			case EVENT_SOURCE_P6:
			case EVENT_SOURCE_P7:
			case EVENT_SOURCE_P8:
				printf("PORT%d\t", (app_log[gate502.app_log_current_entry].source&0xff)+1);
	 			break;
			case EVENT_SOURCE_GATE502:
				printf("GATE502\t");
				break;
			default:
				printf("SYSTEM\t");
			}

	printf("%s\t%d\n", app_log[gate502.app_log_current_entry].desc, gate502.app_log_current_entry);
	}

gate502.app_log_current_entry=gate502.app_log_current_entry==EVENT_LOG_LENGTH-1?0:gate502.app_log_current_entry+1;

return;
}
