/* env.cgi.c */

#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <errno.h>

#include "../global.h"

#define DISPLAY_MAIN_TABLE	1
#define DISPLAY_PORT_X			2
#define DISPLAY_SETTINGS		3

extern char **environ;

int shm_segment_id;
input_cfg *shared_memory;
time_t *timestamp;
key_t access_key;
time_t moment;

void main_table(input_cfg *shared_memory);
void port_info(input_cfg *shared_memory, int port);
void settings(input_cfg *shared_memory);
///-----------------------------------------------------------
int main(int argc, char *argv[])
{
printf("Content-Type: text/html\r\n\r\n"); 

//printf("<html> <head>\n"); 
//printf("<title>CGI Environment</title>\n"); 
//printf("</head>\n");
//printf("<body>\n");
//printf("<h1>CGI Environment</h1>\n"); 

//printf("</body></html>\n");
//return 0;

	access_key=ftok("/tmp/app", 'a');
	shm_segment_id=shmget(access_key, sizeof(time_t)+sizeof(input_cfg)*MAX_MOXA_PORTS, S_IRUSR | S_IWUSR);
	if(shm_segment_id==-1) {
		printf("\
<div class=\"err_block\">\
Modbus-шлюз не запущен на этом компьютере MOXA.\
</div>\
\n");
		return 0;
	  }

	timestamp=(time_t *) shmat(shm_segment_id, 0, 0);
	shared_memory=(input_cfg *)(timestamp+sizeof(time_t));
	
	time(&moment);
	int diff=difftime(moment, *timestamp);
	if(diff>4) {
		printf("\
<div class=\"err_block\">\
Modbus-шлюз не отвечает %d секунд(ы).\
</div>\
\n", diff);
		return 0;
	  }

//	printf("shared memory attached at address %p\n", shared_memory);
//	shmctl(segment_id, IPC_STAT, &shmbuffer);
//	segment_size=shmbuffer.shm_segsz;
//	printf("segment size: %d\n", segment_size);
///----------------------------------------------------------------------------------------
	int t, i;
	diff=shared_memory[0].start_time==0?0:difftime(moment, shared_memory[0].start_time);
  for(i=1; i<MAX_MOXA_PORTS; i++) {
  	t=shared_memory[i].start_time==0?0:difftime(moment, shared_memory[i].start_time);
  	if(diff<t) diff=t;
    }

printf("\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" style=\"float:right;\" id=\"uc7410\">\n\
<tr><th colspan=\"2\"><a href=\"http://www.moxa.com/Product/UC-7400.htm\">MOXA UC-7410</a>\n\
<tr><td>RAM usage<td>00%\n\
<tr><td>CPU usage<td>00%\n\
<tr><td>FLASH usage<td>00%\n\
<tr><td colspan=\"2\">Шлюз в работе\n\
<tr><td colspan=\"2\" style=\"text-align:center;\">%3.3dд %2.2d:%2.2d:%2.2d\n\
</table>\n\
\n", (diff/86400)%1000, (diff/3600)%24, (diff/60)%60, diff%60);

///----------------------------------------------------------------------------------------

int display=DISPLAY_MAIN_TABLE;

char **p = environ;
while(*p != NULL) {
	if(strncmp(*p, "QUERY_STRING_UNESCAPED", 22)==0) break;
//	printf("%s<br>\n",*p);
	p++;
  }

//if(*p!=NULL) printf("*p=%s", *p);
//  else printf("*p=NULL", *p);
//printf("*p[23]=%s", &(*p)[23]);
//return 0;

if(	(*p!=NULL) &&
		(strncmp(&(*p)[23], "PORT", 4)==0) &&
  	((*p)[23+4]>=48)&&((*p)[23+4]<=57)
  ) display=DISPLAY_PORT_X;
  
if(	(*p!=NULL) &&
		(strncmp(&(*p)[23], "SETTINGS", 8)==0)
	) display=DISPLAY_SETTINGS;
  
switch(display) {
  case DISPLAY_PORT_X:
  	port_info(shared_memory, (*p)[23+4]-48);
  	break;

  case DISPLAY_SETTINGS:
  	settings(shared_memory);
  	break;

  case DISPLAY_MAIN_TABLE:
  default: main_table(shared_memory);
  }

shmdt(shared_memory);
return 0;
} 
///-----------------------------------------------------------
void main_table(input_cfg *shared_memory)
{
printf("\
<h1>Статистика опроса по портам</h1>\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\">\n\
<caption>Modbus - шлюз</caption>\n\
<tr>\n\
<th rowspan=\"2\">Порт\n\
<th rowspan=\"2\">Состояние\n\
<th rowspan=\"2\">Режим\n\
<th colspan=\"3\" class=\"internal\">Сообщения Modbus\n\
<th colspan=\"3\" class=\"internal\">Соединения\n\
<th rowspan=\"2\">TCP порт\n\
<tr>\n\
<th>передано\n\
<th>ошибки\n\
<th>задержка\n\
<th>принято\n\
<th>активные\n\
<th>отклонено\n\
\n");

  char pstatus[12], row_class[6];
  int i;
  for(i=0; i<MAX_MOXA_PORTS; i++) {
  	strcpy(pstatus, "Включен");
  	strcpy(row_class, "p_on");
		if(shared_memory[i].modbus_mode==MODBUS_PORT_OFF) {
	  	strcpy(pstatus, "Отключен");
	  	strcpy(row_class, "p_off");
			}
		if(shared_memory[i].modbus_mode==MODBUS_PORT_ERROR) {
	  	strcpy(pstatus, "Сбой");
	  	strcpy(row_class, "p_err");
			}

		printf("\
<tr class=\"%s\">\n\
<td><a href=\"index.shtml?PORT%d\">P%d</a>\n\
<td class=\"pstatus\">%s\n",
						row_class, i+1, i+1, pstatus);

	  if(shared_memory[i].modbus_mode==MODBUS_PORT_OFF) {
printf("\
<td>&nbsp;\
<td>&nbsp;\
<td>&nbsp;\
<td>&nbsp;\
<td>&nbsp;\
<td>&nbsp;\
<td>&nbsp;\
<td>&nbsp;\
");
	  	continue;
	    }
	  
	  if(shared_memory[i].modbus_mode==MODBUS_GATEWAY_MODE)
	  	printf("<td>GATEWAY\n");
	  	else if(shared_memory[i].modbus_mode==MODBUS_MASTER_MODE)
	  	  printf("<td>MASTER");
	  	  else if(shared_memory[i].modbus_mode==MODBUS_BRIDGE_MODE)
					printf("<td>BRIDGE");
					else printf("<td>&nbsp;&nbsp;");
	  
	  printf("<td>%d\n", shared_memory[i].stat.sended);
	  printf("<td>%d\n", shared_memory[i].stat.errors);
	  printf("<td>%d\n", shared_memory[i].stat.request_time_average);
	  printf("<td>%d\n", shared_memory[i].accepted_connections_number);
	  printf("<td>%d\n", shared_memory[i].current_connections_number);
	  printf("<td>%d\n", shared_memory[i].rejected_connections_number);
	  if(shared_memory[i].modbus_mode!=MODBUS_BRIDGE_MODE)
	    printf("<td>%d\n", shared_memory[i].tcp_port);
	    else printf("<td>N/A\n");
	  }


printf("\
<tr><td colspan=\"10\">\n\
<form action=\"/index.shtml\" method=\"post\" name=\"all_ports\" style=\"padding:0; margin:0; border:none;\">\n\
<input type=\"submit\" name=\"all_ports_reset_counters\" value=\"Сбросить счетчики\">\n\
</form>\n\
</table>\n\
\n");
	
return;
}
///-----------------------------------------------------------
void port_info(input_cfg *shared_memory, int port)
{
int P=0;
if(((port-1)>=SERIAL_P1)&&((port-1)<=SERIAL_P8)) P=port-1;
//printf("<p>port %d</p>", P);

  char pstatus[128], pmode[32], row_class[32];
  int i;
  
	strcpy(pstatus, "Включен");
	strcpy(pmode, "N/A");
	strcpy(row_class, "hstatus p_on");
  switch(shared_memory[P].modbus_mode) {
  	
		case MODBUS_GATEWAY_MODE:
  		strcpy(pmode, "GATEWAY");
			break;

		case MODBUS_MASTER_MODE:
  		strcpy(pmode, "MASTER");
			break;

		case MODBUS_BRIDGE_MODE:
  		strcpy(pmode, "BRIDGE");
			break;

		case MODBUS_PORT_OFF:
	  	strcpy(pstatus, "Отключен");
	  	strcpy(row_class, "hstatus p_off");
			break;

		case MODBUS_PORT_ERROR:
	  	strcpy(pstatus, "Сбой");
	  	strcpy(row_class, "hstatus p_err");
			break;

		default:;
  	}

printf("\
<h1>Последовательный порт <span class=\"hstatus p_on\">P%d</span> \
<span class=\"%s\">%s</span> \
<span class=\"%s\">%s</span></h1>\
\n", P+1, row_class, pmode, row_class, pstatus);

///------------ table
printf("\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" id=\"p_counters\">\n\
<caption>Статистика опроса</caption>\n\
\n");

printf("\
<tr><td colspan=\"2\">Запросов принято\n\
<td style=\"width:24pt;\">%d\n\
\n", shared_memory[P].stat.accepted);

	if(shared_memory[P].modbus_mode==MODBUS_BRIDGE_MODE) {
printf("\
<tr><td colspan=\"2\">Ошибки в ADU RTU\n\
<td>%d\n\
\n", shared_memory[P].stat.errors_serial_adu);

printf("\
<tr><td colspan=\"2\">Ошибки в PDU RTU\n\
<td>%d\n\
\n", shared_memory[P].stat.errors_serial_pdu);

printf("\
<tr class=\"main_prm p_err\"><td colspan=\"2\">Ошибки CRC\n\
<td>%d\n\
\n", shared_memory[P].stat.crc_errors);

printf("\
<tr class=\"main_prm p_err\"><td colspan=\"2\">Таймауты связи\n\
<td>%d\n\
\n", shared_memory[P].stat.timeouts);

printf("\
<tr><td colspan=\"2\">Ошибки в ADU TCP\n\
<td>%d\n\
\n", shared_memory[P].stat.errors_tcp_adu);

printf("\
<tr><td colspan=\"2\">Ошибки в PDU TCP\n\
<td>%d\n\
\n", shared_memory[P].stat.errors_tcp_pdu);

	} else {

printf("\
<tr><td colspan=\"2\">Ошибки в ADU TCP\n\
<td>%d\n\
\n", shared_memory[P].stat.errors_tcp_adu);

printf("\
<tr><td colspan=\"2\">Ошибки в PDU TCP\n\
<td>%d\n\
\n", shared_memory[P].stat.errors_tcp_pdu);

printf("\
<tr class=\"main_prm p_err\"><td colspan=\"2\">Таймауты связи\n\
<td>%d\n\
\n", shared_memory[P].stat.timeouts);

printf("\
<tr class=\"main_prm p_err\"><td colspan=\"2\">Ошибки CRC\n\
<td>%d\n\
\n", shared_memory[P].stat.crc_errors);

printf("\
<tr><td colspan=\"2\">Ошибки в ADU RTU\n\
<td>%d\n\
\n", shared_memory[P].stat.errors_serial_adu);

printf("\
<tr><td colspan=\"2\">Ошибки в PDU RTU\n\
<td>%d\n\
\n", shared_memory[P].stat.errors_serial_pdu);
		  }

printf("\
<tr><td colspan=\"2\">Ошибки нижнего уровня\n\
<td>%d\n\
\n", 	shared_memory[P].stat.errors_input_communication+
			shared_memory[P].stat.errors_serial_sending+
			shared_memory[P].stat.errors_serial_accepting+
			shared_memory[P].stat.errors_tcp_sending);

printf("\
<tr class=\"main_prm p_err\"><td colspan=\"2\">Ошибок всего\n\
<td>%d\n\
\n", shared_memory[P].stat.errors);

printf("\
<tr class=\"main_prm p_on\"><td colspan=\"2\">Передано ответов\n\
<td>%d\n\
\n", shared_memory[P].stat.sended);

printf("\
<tr><td rowspan=\"4\">Задержка<br />передачи\n\
<tr><td>минимальная\n\
<td>%d\n\
\n", shared_memory[P].stat.request_time_min);

printf("\
<tr class=\"main_prm p_on\"><td>средняя\n\
<td>%d\n\
\n", shared_memory[P].stat.request_time_average);

printf("\
<tr><td>максимальная\n\
<td>%d\n\
\n", shared_memory[P].stat.request_time_max);

printf("\
<tr><td colspan=\"4\" style=\"text-align:center;\">\n\
<form action=\"\" method=\"post\" name=\"port_stat\" style=\"padding:0; margin:0; border:none;\">\n\
<input type=\"hidden\" name=\"p_num\" value=\"1\">\n\
<input type=\"submit\" name=\"port_stat_reset\" value=\"Сбросить\">\n\
</form>\n\
</table>\n\
\n");

///------------ another table
	if(shared_memory[P].modbus_mode==MODBUS_BRIDGE_MODE) {
	  strcpy(pmode, "сервера");
	  strcpy(pstatus, "\n<th rowspan=\"2\">Адрес<br />modbus\n<th rowspan=\"2\">Сдвиг<br />адреса<br />опроса\n");
	  } else {
	  	strcpy(pmode, "клиенты");
	  	strcpy(pstatus, "\n");
	    }

printf("\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" align=\"center\" id=\"p_clients\">\n\
<caption>Modbus-TCP %s</caption>\n\
<tr>\n\
<th rowspan=\"2\">Адрес IP%s\
<th rowspan=\"2\">Соединено\n\
<th colspan=\"2\" class=\"internal\">Количество\n\
<th rowspan=\"2\">Задержка<br />передачи, мс\n\
<th rowspan=\"2\">Период<br />опроса, мс\n\
<tr>\n\
<th>ответов\n\
<th>ошибок\n\
\n", pmode, pstatus);

for(i=0; i<MAX_TCP_CLIENTS_PER_PORT; i++) {

  if((
  	 (shared_memory[P].clients[i].connection_status!=MB_CONNECTION_ESTABLISHED)&&
  	 (shared_memory[P].modbus_mode!=MODBUS_BRIDGE_MODE)	)||(
  	 (shared_memory[P].modbus_mode==MODBUS_BRIDGE_MODE)&&
  	 (i>=shared_memory[P].accepted_connections_number)	)	) {
printf("\
<tr class=\"p_off\">\n\
<td>&nbsp;\n\
<td>&nbsp;\n\
<td>&nbsp;\n\
<td>&nbsp;\n\
<td>&nbsp;\n\
<td>&nbsp;\n\
");
		if(shared_memory[P].modbus_mode==MODBUS_BRIDGE_MODE)
		  printf("<td>&nbsp;\n<td>&nbsp;\n");

  	continue;
  	}

	int diff=	shared_memory[P].clients[i].connection_time==0?0:\
						difftime(moment, shared_memory[P].clients[i].connection_time);

	sprintf(pmode, "%3.3d.%3.3d.%3.3d.%3.3d",	shared_memory[P].clients[i].ip >> 24,
																(shared_memory[P].clients[i].ip >> 16) & 0xff,
																(shared_memory[P].clients[i].ip >> 8) & 0xff,
																shared_memory[P].clients[i].ip & 0xff);
																
	if(shared_memory[P].modbus_mode==MODBUS_BRIDGE_MODE)
		sprintf(pstatus, "%d", shared_memory[P].clients[i].mb_slave);
		else strcpy(pstatus, "N/A");
		
	strcpy(row_class, "p_on");
	if((shared_memory[P].modbus_mode==MODBUS_BRIDGE_MODE)&&
		 (shared_memory[P].clients[i].connection_status!=MB_CONNECTION_ESTABLISHED)) strcpy(row_class, "p_err");

printf("\
<tr class=\"%s\">\n\
<td style=\"text-align:left;\">%s:%d\n\
",	row_class, pmode, shared_memory[P].clients[i].port);
  		
	if(shared_memory[P].modbus_mode==MODBUS_BRIDGE_MODE) {
	  if(shared_memory[P].clients[i].address_shift!=MB_ADDRESS_NO_SHIFT)
		  sprintf(pmode, "%+d", shared_memory[P].clients[i].address_shift);
		  else sprintf(pmode, "Нет");
		printf("\
<td>%s\n\
<td>%s\n\
",	pstatus, pmode);
	  }
			
  if(shared_memory[P].clients[i].stat.scan_rate<MB_SCAN_RATE_INFINITE)
    sprintf(pstatus, "%d", shared_memory[P].clients[i].stat.scan_rate);
    else sprintf(pstatus, "---");

printf("\
<td style=\"white-space:nowrap;\">%3.3dд %2.2d:%2.2d:%2.2d\n\
<td>%d\n\
<td>%d\n\
<td>%d\n\
<td>%s\n\
",	  (diff/86400)%1000, (diff/3600)%24, (diff/60)%60, diff%60,
  		shared_memory[P].clients[i].stat.sended,
  		shared_memory[P].clients[i].stat.errors,
  		shared_memory[P].clients[i].stat.request_time_average,
  		pstatus);
  		
	}

pmode[0]=shared_memory[P].modbus_mode==MODBUS_BRIDGE_MODE?'4':'2';

printf("\
<tr><td colspan=\"%c\">&nbsp;<td colspan=\"2\" style=\"text-align:center;\">\n\
<form action=\"\" method=\"post\" name=\"port_clients\" style=\"padding:0; margin:0; border:none;\">\n\
<input type=\"hidden\" name=\"p_num\" value=\"1\">\n\
<input type=\"submit\" name=\"port_clients_reset_counters\" value=\"Сбросить\">\n\
</form>\n\
<td colspan=\"2\">&nbsp;\n\
</table>\n\
\n", pmode[0]);

return;
}
///-----------------------------------------------------------
void settings(input_cfg *shared_memory)
{
		printf("\
<h1>Настройка шлюза</h1>\
<div class=\"err_block\">\
Эта функция в настоящий момент не доступна.\
</div>\
\n");
return;
}
///-----------------------------------------------------------
