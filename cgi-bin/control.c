/* env.cgi.c */

#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <errno.h>

#include "../global.h"

#define DISPLAY_MAIN_TABLE		1
#define DISPLAY_PORT_X				2
#define DISPLAY_SETTINGS			3
#define DISPLAY_EVENTS				4
#define DISPLAY_QUERY_TABLES	5

extern char **environ;

int shm_segment_id;

input_cfg *shared_memory;
input_cfg_502 *gate;
GW_EventLog *app_log;

RT_Table_Entry *t_rtm; //[MAX_VIRTUAL_SLAVES];
Query_Table_Entry *t_proxy; //[MAX_QUERY_ENTRIES];
GW_TCP_Server *t_tcpsrv; //[MAX_TCP_SERVERS];

key_t access_key;
time_t moment;

void main_table(input_cfg *shared_memory);
void port_info(input_cfg *shared_memory, int port);
void settings(input_cfg *shared_memory);
void show_events(GW_EventLog *app_log, input_cfg_502 *gate);
void show_vslaves(char port);
void show_tcp_servers(char port);
void show_proxy_queries(char port);

void printMOXA_UC7410(time_t moment);
///-----------------------------------------------------------
int main(int argc, char *argv[])
{
	printf("Content-Type: text/html\r\n\r\n"); 

	int diff;

	access_key=ftok("/tmp/app", 'a');

	unsigned mem_size_ttl =
		sizeof(input_cfg_502)+
		sizeof(input_cfg)*MAX_MOXA_PORTS+
		sizeof(GW_EventLog)*EVENT_LOG_LENGTH+
		sizeof(RT_Table_Entry)*MAX_VIRTUAL_SLAVES+
		sizeof(Query_Table_Entry)*MAX_QUERY_ENTRIES+
		sizeof(GW_TCP_Server)*MAX_TCP_SERVERS;

	shm_segment_id=shmget(access_key, mem_size_ttl, S_IRUSR | S_IWUSR);

	if(shm_segment_id==-1) {
		char estr[16];
		switch(errno) {
			case ENOENT: strcpy(estr, "ENOENT"); break;
			case EACCES: strcpy(estr, "EACCES"); break;
			case EINVAL: strcpy(estr, "EINVAL"); break;
			case ENOMEM: strcpy(estr, "ENOMEM"); break;
			case EEXIST: strcpy(estr, "EEXIST"); break;
			default: sprintf(estr, "unknown %d", errno);
			}

		printf("\
<div class=\"err_block\">\
Modbus-шлюз не отвечает на этом компьютере MOXA. Код ошибки: %s\
</div>\
\n", estr);
		return 0;
	  }

	char *pointer=shmat(shm_segment_id, 0, 0);
	gate=(input_cfg_502 *) pointer;
	shared_memory=(input_cfg *)(pointer+sizeof(input_cfg_502));
	app_log=(GW_EventLog *)(pointer+sizeof(input_cfg_502)+sizeof(input_cfg)*MAX_MOXA_PORTS);

	t_rtm=(RT_Table_Entry *)(pointer+sizeof(input_cfg_502)+sizeof(input_cfg)*MAX_MOXA_PORTS+
													 sizeof(GW_EventLog)*EVENT_LOG_LENGTH);
	t_proxy=(Query_Table_Entry *)(pointer+sizeof(input_cfg_502)+sizeof(input_cfg)*MAX_MOXA_PORTS+
													 sizeof(GW_EventLog)*EVENT_LOG_LENGTH+sizeof(RT_Table_Entry)*MAX_VIRTUAL_SLAVES);
	t_tcpsrv=(GW_TCP_Server *)(pointer+sizeof(input_cfg_502)+sizeof(input_cfg)*MAX_MOXA_PORTS+
														sizeof(GW_EventLog)*EVENT_LOG_LENGTH+sizeof(RT_Table_Entry)*MAX_VIRTUAL_SLAVES+
														sizeof(Query_Table_Entry)*MAX_QUERY_ENTRIES);
	time(&moment);
	diff=difftime(moment, gate->timestamp);
	if(diff>GATE_WEB_INTERFACE_TIMEOUT) {
		printf("\
<div class=\"err_block\">\
Modbus-шлюз не отвечает %d секунд(ы).\
</div>\
\n", diff);
		return 0;
	  }

///----------------------------------------------------------------------------------------
	int t, i;

//	char sel1[16], sel2[16], sel3[16], sel4[16];
//  sel1[0]=sel2[0]=sel3[0]=sel4[0]=0;
//	if(shm_data->ATM)		strcpy(sel2, " class=\"sel\"");
//	if(shm_data->RTM && !shm_data->PROXY)		strcpy(sel3, " class=\"sel\"");
//	if(shm_data->PROXY)	strcpy(sel4, " class=\"sel\"");
//  if(!(shm_data->ATM || shm_data->RTM || shm_data->PROXY)) strcpy(sel1, " class=\"sel\"");

printf("\
<table id=\"mode\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\" style=\"float:right;\">\n\
<tr>\n\
<td>%s\n\
<td>%s\n\
<td>%s\n\
<td style=\"border-right:none;\">%d.%d.%d.%d\n\
</table>\n\
\n", gate->object, gate->location, gate->networkName, gate->IPAddress>>24, gate->IPAddress>>16&0xff, gate->IPAddress>>8&0xff, gate->IPAddress&0xff);

/*printf("\
<table id=\"mode\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\" style=\"float:right;\">\n\
<tr>\n\
<td class=\"sel\">РЕЖИМ\n\
<td%s>GATEWAY SIMPLE\n\
<td%s>GATEWAY ATM\n\
<td%s>GATEWAY RTM\n\
<td%s style=\"border-right:none;\">GATEWAY PROXY\n\
</table>\n\
\n", sel1, sel2, sel3, sel4);*/

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
  
if(	(*p!=NULL) &&
		(strncmp(&(*p)[23], "EVENTS", 6)==0)
	) display=DISPLAY_EVENTS;

if(	(*p!=NULL) &&
		(strncmp(&(*p)[23], "TABLES", 6)==0)
	) display=DISPLAY_QUERY_TABLES;

if(	(*p!=NULL) &&
		(strncmp(&(*p)[23], "HALT", 4)==0)
	) {gate->halt=1;
		printf("\
<div class=\"err_block\">\
Modbus-шлюз остановлен. Перейдите на главную\
</div>\
\n", diff);
		return 0;
		}
  
switch(display) {
  case DISPLAY_PORT_X:
  	port_info(shared_memory, (*p)[23+4]-48);
  	break;

  case DISPLAY_SETTINGS:
  	settings(shared_memory);
  	break;
														
  case DISPLAY_EVENTS:
  	show_events(app_log, gate);
  	break;

  case DISPLAY_QUERY_TABLES:
		printf("<h1 align=\"center\">Таблицы опроса</h1>\n");
  	show_vslaves(SERIAL_STUB);
		printf("<p>&nbsp;</p>\n");
  	show_tcp_servers(SERIAL_STUB);
		printf("<p>&nbsp;</p>\n");
  	show_proxy_queries(SERIAL_STUB);
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
printMOXA_UC7410(moment);

printf("\
<h1>Статистика опроса</h1>\n\n\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" align=\"center\" style=\"width:80%\">\n\
<caption>Шлюз MODBUS</caption>\n\
<tr>\n\
<th rowspan=\"2\">Порт\n\
<th rowspan=\"2\">Состояние\n\
<th rowspan=\"2\">Режим\n\
<th colspan=\"3\">Сообщения ModBus\n\
<th rowspan=\"2\">Адрес<br />ModBus\n\
<th rowspan=\"2\">Порт<br />TCP\n\
<tr>\n\
<th>передано\n\
<th>ошибки\n\
<th>задержка\n\
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
");
	  	continue;
	    }
	  
	  switch(shared_memory[i].modbus_mode) {
			case GATEWAY_SIMPLE: printf("<td class=\"pstatus\">GATEWAY_SIMPLE\n"); break;
			case GATEWAY_ATM: printf("<td class=\"pstatus\">GATEWAY_ATM\n"); break;
			case GATEWAY_RTM: printf("<td class=\"pstatus\">GATEWAY_RTM\n"); break;
			case GATEWAY_PROXY: printf("<td class=\"pstatus\">GATEWAY_PROXY\n"); break;
			case BRIDGE_PROXY: printf("<td class=\"pstatus\">BRIDGE_PROXY\n"); break;
			case BRIDGE_SIMPLE: printf("<td class=\"pstatus\">BRIDGE_SIMPLE\n"); break;
			default: printf("<td class=\"pstatus\">UNKNOWN\n");
			}	  

	  printf("<td>%d\n", shared_memory[i].stat.sended);
	  printf("<td>%d\n", shared_memory[i].stat.errors);
	  printf("<td>%d\n", shared_memory[i].stat.request_time_average);
//	  printf("<td>%d\n", shared_memory[i].accepted_connections_number);
//	  printf("<td>%d\n", shared_memory[i].current_connections_number);
//	  printf("<td>%d\n", shared_memory[i].rejected_connections_number);
	  switch(shared_memory[i].modbus_mode) {
			case GATEWAY_SIMPLE: printf("<td>1..247\n"); break;
			case GATEWAY_ATM: printf("<td>%d..%d\n", i*30+1, i*30+31); break;
			case GATEWAY_RTM: printf("<td>%d\n", gate->modbus_address); break;
			case GATEWAY_PROXY: printf("<td>%d\n", gate->modbus_address); break;
	    default: printf("<td>---\n");						
			}

	  switch(shared_memory[i].modbus_mode) {
			case GATEWAY_SIMPLE: printf("<td>%d\n", shared_memory[i].tcp_port); break;
			case GATEWAY_ATM: printf("<td>%d\n", gate->tcp_port); break;
			case GATEWAY_RTM: printf("<td>%d\n", gate->tcp_port); break;
			case GATEWAY_PROXY: printf("<td>%d\n", gate->tcp_port); break;
	    default: printf("<td>---\n");						
			}
	  }

printf("\
<tr><td colspan=\"8\">\n\
<form action=\"index.shtml?HALT\" method=\"post\" name=\"all_ports\" style=\"padding:0; margin:0; border:none;\">\n\
<input type=\"submit\" name=\"all_ports_reset_counters\" value=\"С Т О П\">\n\
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
void show_events(GW_EventLog *app_log, input_cfg_502 *gate)
{

		printf("\
<h1 align=\"center\">Журнал событий</h1>\n\n\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" align=\"center\" style=\"width:80%\" id=\"events_table\">\n\
<caption>&nbsp;</caption>\n\n\
<tr>\n\
<th>№\n\
<th>Время\n\
<th>Источник\n\
<th>Код ошибки\n\
<th>Сообщение\n\
\n");

	struct tm *tmd;
  int i, j;
	char timestr[32], sourcestr[16];

  j=gate->app_log_current_entry;
	for(i=0; i<EVENT_LOG_LENGTH; i++) {

	if(app_log[j].desc[0]==0) {
		j=j==EVENT_LOG_LENGTH-1?0:j+1;
		continue;
		}

	tmd=gmtime(&app_log[j].time);
	sprintf(timestr, "%2.2d.%2.2d.%4.4d %2.2d:%2.2d:%2.2d\t", tmd->tm_mday, tmd->tm_mon+1, tmd->tm_year+1900, tmd->tm_hour, tmd->tm_min, tmd->tm_sec);

	switch(app_log[j].source) {
			case EVENT_SOURCE_P1:
			case EVENT_SOURCE_P2:
			case EVENT_SOURCE_P3:
			case EVENT_SOURCE_P4:
			case EVENT_SOURCE_P5:
			case EVENT_SOURCE_P6:
			case EVENT_SOURCE_P7:
			case EVENT_SOURCE_P8:
				sprintf(sourcestr, "PORT%d\t", (app_log[j].source&0xff)+1);
	 			break;
			case EVENT_SOURCE_GATE502:
				sprintf(sourcestr, "GATE502\t");
				break;
			default:
				sprintf(sourcestr, "SYSTEM\t");
			}

	printf("\
<tr>\n\
<td>%d\n\
<td>%s\n\
<td>%s\n\
<td>&nbsp;\n\
<td>%s\n\
\n", j+1, timestr, sourcestr, app_log[j].desc);

	j=j==EVENT_LOG_LENGTH-1?0:j+1;
  }

		printf("\
</table>\n\
\n");

return;
}
///-----------------------------------------------------------
void printMOXA_UC7410(time_t moment)
{
int diff=difftime(moment, gate->start_time);

printf("\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" style=\"float:right;\" id=\"uc7410\">\n\
<tr><th colspan=\"2\"><a href=\"http://www.moxa.com/Product/UC-7400.htm\" style=\"color:white;\">MOXA UC-7410</a>\n\
<tr><td colspan=\"2\">Ресурсы системы\n\
<tr><td>RAM<td class=\"val\">00%\n\
<tr><td>CPU<td class=\"val\">00%\n\
<tr><td>FLASH<td class=\"val\">00%\n\
<tr><td colspan=\"2\">Версия конфигурации\n\
<tr><td colspan=\"2\" class=\"val\">%s\n\
<tr><td colspan=\"2\">Время запуска шлюза\n\
<tr><td colspan=\"2\" class=\"val\">01.01.1970 00:00\n\
<tr><td colspan=\"2\">Время работы шлюза\n\
<tr><td colspan=\"2\" class=\"val\">%3.3dд %2.2d:%2.2d:%2.2d\n\
</table>\n\
\n", gate->version, (diff/86400)%1000, (diff/3600)%24, (diff/60)%60, diff%60);
}
///-----------------------------------------------------------
void show_vslaves(char port)
	{
	int i;
	char mbtbl[20];

printf("\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" align=\"center\" style=\"width:80%\" id=\"rtm_table\">\n\
<caption>Таблица RTM_TABLE</caption>\n\
<tr>\n\
<th>№<br /><span style=\"white-space:nowrap;\">п/п</span>\n\
<th>Адрес ModBus\n\
<th>Таблица ModBus\n\
<th>Начальный адрес диапазона\n\
<th>Размер диапазона\n\
<th>Смещение диапазона\n\
<th>Порт\n\
<th>Наименование устройства\n\
\n");

	for(i=0; i<MAX_VIRTUAL_SLAVES; i++) {

		switch(t_rtm[i].modbus_table) {
			case COIL_STATUS_TABLE: strcpy(mbtbl, "COIL_STATUS"); break;
			case INPUT_STATUS_TABLE: strcpy(mbtbl, "INPUT_STATUS"); break;
			case HOLDING_REGISTER_TABLE: strcpy(mbtbl, "HOLDING_REGISTER"); break;
			case INPUT_REGISTER_TABLE: strcpy(mbtbl, "INPUT_REGISTER"); break;
			default: continue;
			}

printf("\
<tr>\n\
<td>%d\n\
<td>%d\n\
<td>%s\n\
<td>%d\n\
<td>%d\n\
<td>%d\n\
<td>P%d\n\
<td>%s\n",
		i+1,
		t_rtm[i].device,
		mbtbl,
		t_rtm[i].start+1,
		t_rtm[i].length,
		t_rtm[i].address_shift,
		t_rtm[i].port+1,
		t_rtm[i].device_name);
	  }
	
	printf("</table>\n");
	return;
	}
///-----------------------------------------------------------
void show_tcp_servers(char port)
	{
	int i;

printf("\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" align=\"center\" style=\"width:80%\" id=\"tcp_servers\">\n\
<caption>Таблица TCP_SERVERS</caption>\n\
<tr>\n\
<th>№<br /><span style=\"white-space:nowrap;\">п/п</span>\n\
<th>Адрес ModBus\n\
<th>Адрес IP\n\
<th>Порт TCP\n\
<th>Смещение адреса\n\
<th>Порт\n\
<th>Наименование устройства\n\
\n");

	for(i=0; i<MAX_TCP_SERVERS; i++) {

		if(t_tcpsrv[i].mb_slave==0) continue;

printf("<tr>\n\
<td>%d\n\
<td>%d\n\
<td>%d.%d.%d.%d\n\
<td>%d\n\
<td>%d\n\
<td>P%d\n\
<td>%s\n",
		i+1,
		t_tcpsrv[i].mb_slave,
		t_tcpsrv[i].ip>>24, t_tcpsrv[i].ip>>16&0xff, t_tcpsrv[i].ip>>8&0xff, t_tcpsrv[i].ip&0xff, 
		t_tcpsrv[i].port,
		t_tcpsrv[i].address_shift,
		t_tcpsrv[i].p_num+1,
		t_tcpsrv[i].device_name);
	  }

	printf("</table>\n");
	return;
	}
///-----------------------------------------------------------
void show_proxy_queries(char port)
	{
	int i;
	char mbtbl[20];

printf("\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" align=\"center\" style=\"width:80%\" id=\"proxy_table\">\n\
<caption>Таблица PROXY_TABLE</caption>\n\
<tr>\n\
<th>№<br /><span style=\"white-space:nowrap;\">п/п</span>\n\
<th>Адрес ModBus\n\
<th>Таблица ModBus\n\
<th>Адрес блока для чтения\n\
<th>Количество регистров (статусов)\n\
<th>Адрес блока для записи\n\
<th>Порт\n\
<th>Выдержка времени, мс\n\
<th>Критичное количество запросов\n\
<th>Наименование устройства\n\
\n");

	for(i=0; i<MAX_QUERY_ENTRIES; i++) {

		switch(t_proxy[i].mbf) {
			case COIL_STATUS_TABLE: strcpy(mbtbl, "COIL_STATUS"); break;
			case INPUT_STATUS_TABLE: strcpy(mbtbl, "INPUT_STATUS"); break;
			case HOLDING_REGISTER_TABLE: strcpy(mbtbl, "HOLDING_REGISTER"); break;
			case INPUT_REGISTER_TABLE: strcpy(mbtbl, "INPUT_REGISTER"); break;
			default: continue;
			}

printf("<tr>\n\
<td>%d\n\
<td>%d\n\
<td>%s\n\
<td>%d\n\
<td>%d\n\
<td>%d\n\
<td>P%d\n\
<td>%d\n\
<td>%d\n\
<td>%s\n",
		i+1,
		t_proxy[i].device,
		mbtbl,
		t_proxy[i].start,
		t_proxy[i].length,
		t_proxy[i].offset,
		t_proxy[i].port+1,
		t_proxy[i].delay,
		t_proxy[i].critical,
		t_proxy[i].device_name);
	  }

	printf("</table>\n");
	return;
	}
///-----------------------------------------------------------
