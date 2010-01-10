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

void sysmsg(int source, char *message, int show_anyway)
{
int i;
char string[EVENT_MESSAGE_LENGTH];
//if(strlen(string)>=EVENT_MESSAGE_LENGTH) string[EVENT_MESSAGE_LENGTH]=0;
for(i=0; i<EVENT_MESSAGE_LENGTH-1; i++)
  string[i]=message[i];
string[EVENT_MESSAGE_LENGTH-1]=0;

if(_show_sys_messages==1 || show_anyway==1)
  if((source&0xff)<8) {
    if(((source>>8)&0xff)<8)
		  printf("PORT%d.CLIENT%d: %s\n", (source&0xff)+1, ((source>>8)&0xff)+1, string);
		  else printf("PORT%d         : %s\n", (source&0xff)+1, string);
	  } else switch (source&0xff) {
			case EVENT_SOURCE_GATE502:
				printf(" GATE502 #%4.4X: %s\n", ((source>>8)&0xff), string);
				break;
			default:
				printf(" SYSTEM #%4.4X: %s\n", ((source>>8)&0xff)+1, string);
			}

///!!! добавление записи в журнал сообщений
//if(strlen(string)>=EVENT_MESSAGE_LENGTH) return;
	
return;
}
