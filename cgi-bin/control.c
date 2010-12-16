/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///** МОДУЛЬ МОНИТОРИНГА И УПРАВЛЕНИЯ РАБОТОЙ ШЛЮЗА СРЕДСТВАМИ WEB-ИНТЕРФЕЙСА **

///=== HMI_WEB_H IMPLEMENTATION

#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>

#include "../hmi_web.h"
#include "../interfaces.h"
#include "../moxagate.h"
#include "../clients.h"
#include "../messages.h"

#define DISPLAY_MAIN_TABLE		1
#define DISPLAY_PORT_X				2
#define DISPLAY_SETTINGS			3
#define DISPLAY_EVENTS				4
#define DISPLAY_QUERY_TABLES	5

#define MAX_FORM_FIELDS			16
#define MAX_FORM_FIELD_LEN	32
#define MAX_FORM_VALUE_LEN	32

#define FORM_ACTION_CANCEL	0
#define FORM_ACTION_SAVE		1
#define FORM_ACTION_APPLY		2
#define FORM_ACTION_RESET		3
#define FORM_ACTION_REBOOT	4

extern char **environ;

int shm_segment_id;

char *pointer;

// статические данные
GW_AddressMap_Entry *addrmap;
RT_Table_Entry *vslave;
Query_Table_Entry *pquery;
GW_Exception *exception;

// динамически обновляемые данные
GW_Security *security;
GW_MoxaDevice *gate;
GW_Iface *iface_rtu;
GW_Iface *iface_tcp;
GW_Client *client;
GW_EventLog *event_log;

key_t access_key;
time_t moment;

void main_table();
void port_info(int port);
void settings();
void show_events();
void show_vslaves(char port);
//void show_tcp_servers(char port);
void show_proxy_queries(char port);

void printMOXA_UC7410(time_t moment);
void show_stat_by_stage(u8 p_num);
void show_stat_by_func(u8 P);
void show_SL_settings(u8 p_num);
void show_interface_clients(u8 P);

int process_input_data();
///-----------------------------------------------------------
int main(int argc, char *argv[])
	{
	int diff;

	access_key=ftok("/tmp/app", 'a');

	unsigned int mem_size_ttl =
		sizeof(GW_AddressMap_Entry)*(MODBUS_ADDRESS_MAX+1)+
		sizeof(RT_Table_Entry)*MAX_VIRTUAL_SLAVES+
		sizeof(Query_Table_Entry)*MAX_QUERY_ENTRIES+
		sizeof(GW_Exception)*MOXAGATE_EXCEPTIONS_NUMBER+
		sizeof(GW_Security)+
		sizeof(GW_MoxaDevice)+
		sizeof(GW_Iface)*MAX_MOXA_PORTS+
		sizeof(GW_Iface)*MAX_TCP_SERVERS+
		sizeof(GW_Client)*MOXAGATE_CLIENTS_NUMBER+
		sizeof(GW_EventLog)+
		sizeof(unsigned int) * EVENT_TEMPLATE_AMOUNT+
		sizeof(char) * EVENT_TEMPLATE_AMOUNT * EVENT_MESSAGE_LENGTH+
		sizeof(GW_Event)*EVENT_LOG_LENGTH;

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
Content-Type: text/html\r\n\r\n\
<div class=\"err_block\">\
Modbus-шлюз не отвечает на этом компьютере MOXA. Код ошибки: %s\
</div>\
\n", estr);
		return 0;
	  }

  unsigned int k;

	pointer = shmat(shm_segment_id, 0, 0);

  k=0;

	addrmap=(GW_AddressMap_Entry *) (pointer+k);

  k+= sizeof(GW_AddressMap_Entry)*(MODBUS_ADDRESS_MAX+1);

	vslave=(RT_Table_Entry *) (pointer+k);

  k+= sizeof(RT_Table_Entry)*MAX_VIRTUAL_SLAVES;

	pquery=(Query_Table_Entry *) (pointer+k);

  k+= sizeof(Query_Table_Entry)*MAX_QUERY_ENTRIES;

	exception=(GW_Exception *) (pointer+k);

  k+= sizeof(GW_Exception)*MOXAGATE_EXCEPTIONS_NUMBER;

	security=(GW_Security *) (pointer+k);

  k+= sizeof(GW_Security);

	gate=(GW_MoxaDevice *) (pointer+k);

  k+= sizeof(GW_MoxaDevice);

	iface_rtu=(GW_Iface *) (pointer+k);

  k+= sizeof(GW_Iface)*MAX_MOXA_PORTS;

	iface_tcp=(GW_Iface *) (pointer+k);

  k+= sizeof(GW_Iface)*MAX_TCP_SERVERS;

	client=(GW_Client *) (pointer+k);

  k+= sizeof(GW_Client)*MOXAGATE_CLIENTS_NUMBER;

	event_log=(GW_EventLog *) (pointer+k);

  k+= sizeof(GW_EventLog);

	EventLog.msg_index= // указатель нужен для использования в функциях модуля MESSAGES_H
  event_log->msg_index=(unsigned int *) (pointer+k);

  k+= sizeof(unsigned int) * EVENT_TEMPLATE_AMOUNT;

  EventLog.msg_tpl=  // указатель нужен для использования в функциях модуля MESSAGES_H
	event_log->msg_tpl=(char *) (pointer+k);

  k+= sizeof(char) * EVENT_TEMPLATE_AMOUNT * EVENT_MESSAGE_LENGTH;

  //EventLog.app_log=
  event_log->app_log=
    (GW_Event *) (pointer+k);

	time(&moment);
	diff=difftime(moment, security->timestamp);

///----------------------------------------------------------------------------------------
	int t, i;
	char **p;

	p = environ;
	while(*p != NULL) {if(strncmp(*p, "REQUEST_METHOD", 14)==0) break; p++;}
	if(	(*p!=NULL) && (strncmp(&(*p)[15], "POST", 4)==0) ) return process_input_data();

///----------------------------------------------------------------------------------------

  //--- вывод контента
  printf("Content-Type: text/html\r\n\r\n"); 

	if(diff>GATE_WEB_INTERFACE_TIMEOUT) {
		printf("\
<div class=\"err_block\">\
Modbus-шлюз не отвечает %d секунд(ы).\
</div>\
\n", diff);
	  }

printf("\
<table id=\"mode\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\" style=\"float:right;\">\n\
<tr>\n\
<td>%s \n\
<td>%s \n\
<td>%s \n\
<td style=\"border-right:none;\">%d.%d.%d.%d\n\
</table>\n\
\n", security->Object,
     security->Location,
     security->NetworkName,
     security->LAN1Address>>24,
     security->LAN1Address>>16&0xff,
     security->LAN1Address>>8&0xff,
     security->LAN1Address&0xff);

  //--- определяем запрашиваемую страницу

int display=DISPLAY_MAIN_TABLE;

p = environ;
while(*p != NULL) {
	if(strncmp(*p, "QUERY_STRING_UNESCAPED", 22)==0) break;
	//printf("%s<br>\n",*p);
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
	) {
		security->halt=1;
		printf("<div class=\"err_block\">Modbus-шлюз остановлен. Перейдите на главную</div>\n");
		return 0;
		}
  
switch(display) {
  case DISPLAY_PORT_X:
  	port_info((*p)[23+4]-48);
  	break;

  case DISPLAY_SETTINGS:
  	settings();
  	break;
														
  case DISPLAY_EVENTS:
  	show_events(event_log, gate);
  	break;

  case DISPLAY_QUERY_TABLES:
		printf("<h1 align=\"center\">Таблицы опроса</h1>\n");
  	show_vslaves(GATEWAY_NONE);
		printf("<p>&nbsp;</p>\n");
//  	show_tcp_servers(GATEWAY_NONE);
//		printf("<p>&nbsp;</p>\n");
  	show_proxy_queries(GATEWAY_NONE);
  	break;

  case DISPLAY_MAIN_TABLE:
  default:
		printMOXA_UC7410(moment);
		main_table(iface_rtu);
  }

shmdt(pointer);
return 0;
}
///-----------------------------------------------------------
void main_table()
{

printf("\
<h1>Статистика опроса</h1>\n\n\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" align=\"center\" id=\"m_stat_tbl\">\n\
<caption>Шлюз MODBUS</caption>\n\
<tr>\n\
<th rowspan=\"2\" class=\"f\">Порт\n\
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
		if(iface_rtu[i].modbus_mode==MODBUS_PORT_OFF) {
	  	strcpy(pstatus, "Отключен");
	  	strcpy(row_class, "p_off");
			}
		if(iface_rtu[i].modbus_mode==MODBUS_PORT_ERROR) {
	  	strcpy(pstatus, "Сбой");
	  	strcpy(row_class, "p_err");
			}

		printf("\
<tr class=\"%s\">\n\
<td class=\"f\" title=\"Порт P%d. %s\"><a href=\"index.shtml?PORT%d\">P%d</a>\n\
<td class=\"pstatus\">%s\n",
						row_class, i+1, iface_rtu[i].description, i+1, i+1, pstatus);

	  if(iface_rtu[i].modbus_mode==MODBUS_PORT_OFF) {
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
	  
	  switch(iface_rtu[i].modbus_mode) {
			case GATEWAY_SIMPLE: printf("<td class=\"pstatus\">GATEWAY_SIMPLE\n"); break;
			case GATEWAY_ATM: printf("<td class=\"pstatus\">GATEWAY_ATM\n"); break;
			case GATEWAY_RTM: printf("<td class=\"pstatus\">GATEWAY_RTM\n"); break;
			case GATEWAY_PROXY: printf("<td class=\"pstatus\">GATEWAY_PROXY\n"); break;
			case BRIDGE_PROXY: printf("<td class=\"pstatus\">BRIDGE_PROXY\n"); break;
//			case BRIDGE_SIMPLE: printf("<td class=\"pstatus\">BRIDGE_SIMPLE\n"); break;
			default: printf("<td class=\"pstatus\">UNKNOWN\n");
			}	  

	  printf("<td>%d\n", iface_rtu[i].stat.sended);
	  printf("<td>%d\n", iface_rtu[i].stat.errors);
	  printf("<td>%d\n", iface_rtu[i].stat.request_time);
//	  printf("<td>%d\n", iface_rtu[i].accepted_connections_number);
//	  printf("<td>%d\n", iface_rtu[i].current_connections_number);
//	  printf("<td>%d\n", iface_rtu[i].rejected_connections_number);
	  switch(iface_rtu[i].modbus_mode) {
			case GATEWAY_SIMPLE: printf("<td>1..247\n"); break;
			case GATEWAY_ATM: printf("<td>%d..%d\n", i*30+1, i*30+31); break;
			case GATEWAY_RTM: printf("<td>%d\n", gate->modbus_address); break;
			case GATEWAY_PROXY: printf("<td>%d\n", gate->modbus_address); break;
	    default: printf("<td>---\n");						
			}

	  switch(iface_rtu[i].modbus_mode) {
			case GATEWAY_SIMPLE: printf("<td>%d\n", iface_rtu[i].Security.tcp_port); break;
			case GATEWAY_ATM: printf("<td>%d\n", security->tcp_port); break;
			case GATEWAY_RTM: printf("<td>%d\n", security->tcp_port); break;
			case GATEWAY_PROXY: printf("<td>%d\n", security->tcp_port); break;
	    default: printf("<td>---\n");						
			}
	  }

printf("\
<tr><td colspan=\"8\" class=\"f\">\n\
<form action=\"\" method=\"post\" name=\"all_ports\" style=\"padding:0; margin:0; border:none;\">\n\
<input type=\"submit\" name=\"all_ports_reset_counters\" value=\"Сбросить значения счетчиков\">\n\
</form>\n\
<tr><td colspan=\"8\" style=\"border:none; text-align:left;\">\n\
* Для получения дополнительной информации наведите указатель мыши на первый столбец каждой строчки.\n\
</table>\n\
\n");

return;
}
///-----------------------------------------------------------
void port_info(int port)
{
int P=0;
if(((port-1)>=GATEWAY_P1)&&((port-1)<=GATEWAY_P8)) P=port-1; 
	else {
		printf("<div class=\"err_block\">Неверно указан номер последовательного интерфейса. Проверте ссылку</div>\n");
		return;
		}

	show_stat_by_stage(P);
	show_SL_settings(P);

	printf("\
<h1>Порт P%d. %s</h1>",
	P+1, iface_rtu[P].description);

	show_stat_by_func(P);

	switch(iface_rtu[P].modbus_mode) {
		case GATEWAY_SIMPLE:
	//		case BRIDGE_SIMPLE:
  //		show_interface_clients(P);
	//		break;

		case GATEWAY_RTM:
	  	show_vslaves(P);
			break;

		case GATEWAY_PROXY:
	  	show_proxy_queries(P);
			break;

		default:;
		}

return;
}
///-----------------------------------------------------------
void settings()
	{
	char wdt[16], buz[16], bl[16];

	if(security->watchdog_timer==1)	strcpy(wdt, " checked=\"on\""); else wdt[0]=0;
	if(security->use_buzzer==1)			strcpy(buz, " checked=\"on\""); else buz[0]=0;
	if(security->back_light==1)			strcpy(bl,  " checked=\"on\""); else bl[0]=0;

	printf("\
<h1>Настройка шлюза</h1>\n\n\
<form action=\"/cgi-bin/control\" method=\"POST\" name=\"gateway_settings\" id=\"gw_sett_form\">\n\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" align=\"center\" id=\"gw_sett\">\n\
<caption>Параметры</caption>\n\n\
<tr><td class=\"f\">\n\
<input type=\"text\" name=\"ip_address\" value=\"%3.3d.%3.3d.%3.3d.%3.3d\" size=\"15\" maxlength=\"15\" readonly=\"on\" />\n\
<td>Адрес IP\n\n\
<tr><td class=\"f\">\n\
<input type=\"text\" name=\"tcp_port\" value=\"%d\" size=\"4\" maxlength=\"5\" readonly=\"on\" />\n\
<td>Порт TCP\n\n\
<tr><td class=\"f\">\n\
<input type=\"text\" name=\"mb_address\" value=\"%d\" size=\"4\" maxlength=\"3\" readonly=\"on\" />\n\
<td>Адрес ModBus\n\n\
<tr><td class=\"f\">\n\
<input type=\"text\" name=\"status_info\" value=\"%d\" size=\"4\" maxlength=\"5\" readonly=\"on\" />\n\
<td>Адрес блока диагностической информации\n\n\
<tr><td class=\"f\">\n\
<input type=\"checkbox\" name=\"wdt_enabled\"%s />\n\
<td>WatchDog таймер включен\n\n\
<tr><td class=\"f\">\n\
<input type=\"checkbox\" name=\"buzzer_enabled\"%s />\n\
<td>Звуковой сигнал при ошибке\n\n\
<tr><td class=\"f\">\n\
<input type=\"checkbox\" name=\"backlight_enabled\"%s />\n\
<td>Фоновая подсветка дисплея\n\n\
<tr><td colspan=\"2\" style=\"text-align: center;\">\n\
<input type=\"submit\" name=\"apply\" value=\"Применить\">\n\
<input type=\"reset\" value=\"Отмена\">\n\
<input type=\"submit\" name=\"reboot\" value=\"Перезагрузка MOXA\">\n\
<input type=\"hidden\" name=\"form_id\" value=\"gateway_settings\">\n\
</table>\n\
</form>\n",
	security->LAN1Address>>24, security->LAN1Address>>16&0xff, security->LAN1Address>>8&0xff, security->LAN1Address&0xff,
	security->tcp_port,
	gate->modbus_address,
	gate->status_info+1,																 
	wdt, buz, bl);

	printf("\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" align=\"center\" id=\"gw_memory\">\
<caption>Внутренние ModBus регистры MOXA</caption>\n\
<tr>\n\
<th>Таблица\n\
<th>Стартовый адрес\n\n\
<th>Количество\n\
<tr>\n<td>COIL STATUS\n<td>%d\n<td>%d\n\n\
<tr>\n<td>INPUT STATUS\n<td>%d\n<td>%d\n\n\
<tr>\n<td>INPUT REGISTER\n<td>%d\n<td>%d\n\n\
<tr>\n<td>HOLDING REGISTER\n<td>%d\n<td>%d\n\n\
</table>\n",
	gate->offset1xStatus+1,	gate->amount1xStatus,
	gate->offset2xStatus+1,	gate->amount2xStatus,
	gate->offset3xRegisters+1,	gate->amount3xRegisters,
	gate->offset4xRegisters+1,	gate->amount4xRegisters
	);


return;
}
///-----------------------------------------------------------
void show_events()
{

		printf("\
<h1 align=\"center\">Журнал событий</h1>\n\n\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" align=\"center\" style=\"width:80%\" id=\"events_table\">\n\
<caption>&nbsp;</caption>\n\n\
<tr>\n\
<th>№\n\
<th>Время\n\
<th>Тип\n\
<th>Источник\n\
<th>Текст сообщения\n\
\n");

	struct tm *tmd;
  int i, j;
	char timestr[32], sourcestr[16], typestr[8];
  char eventmsg[EVENT_MESSAGE_LENGTH+EVENT_MESSAGE_PREFIX];

  ///!!! использовать разделяемый сегмент памяти для хранения шаблонов сообщений
  ///!!! для экономии процессорного времени:
	//init_message_templates();

  j = event_log->app_log_current_entry;
	for(i=0; i<EVENT_LOG_LENGTH; i++) {

	if(event_log->app_log[j].msgtype==0) {
		j = j==EVENT_LOG_LENGTH-1 ? 0 : j+1;
		continue;
		}

	tmd=gmtime(&event_log->app_log[j].time);
  strftime(eventmsg, 16, " %b", tmd);
	sprintf(timestr, "%2.2d%s %2.2d:%2.2d:%2.2d ", tmd->tm_mday,
                                                 eventmsg,
                                                 tmd->tm_hour,
                                                 tmd->tm_min,
                                                 tmd->tm_sec);

	get_msgtype_str(event_log->app_log[j].msgtype, typestr);

	get_msgsrc_str(event_log->app_log[j].msgtype, event_log->app_log[j].prm[0], sourcestr);

	eventmsg[0]=0;
	make_msgstr(event_log->app_log[j].msgcode, eventmsg, event_log->app_log[j].prm[0],
                                                       event_log->app_log[j].prm[1],
                                                       event_log->app_log[j].prm[2],
                                                       event_log->app_log[j].prm[3]);

	printf("\
<tr>\n\
<td>%d\n\
<td>%s\n\
<td>%s\n\
<td>%s\n\
<td>%s\n\
\n", j+1, timestr, typestr, sourcestr, eventmsg);

	j = j==EVENT_LOG_LENGTH-1 ? 0 : j+1;
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
<tr><th colspan=\"2\"><a href=\"http://www.moxa.com/Product/UC-7400.htm\" style=\"color:white;\" title=\"Перейти на официальный сайт устройства\">MOXA UC-7410</a>\n\
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
\n", security->VersionNumber, (diff/86400)%1000, (diff/3600)%24, (diff/60)%60, diff%60);
}
///-----------------------------------------------------------
void show_vslaves(char port)
	{
	int i;
	char mbtbl[20];
	char any_entry=0;

printf("\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" align=\"center\" style=\"width:80%\" id=\"rtm_table\">\n\
<caption>Таблица RTM_TABLE</caption>\n\
<tr>\n\
<th>№<br /><span style=\"white-space:nowrap;\">п/п</span>\n\
<th>Адрес<br />ModBus\n\
<th>Таблица ModBus\n\
<th>Начальный<br />адрес\n\
<th>Размер<br />диапазона\n\
<th>Смещение<br />диапазона\n\
<th>Порт\n\
<th>Наименование устройства\n\
\n");

	for(i=0; i<MAX_VIRTUAL_SLAVES; i++) {

		switch(vslave[i].modbus_table) {
			case COIL_STATUS_TABLE: strcpy(mbtbl, "COIL_STATUS"); break;
			case INPUT_STATUS_TABLE: strcpy(mbtbl, "INPUT_STATUS"); break;
			case HOLDING_REGISTER_TABLE: strcpy(mbtbl, "HOLDING_REGISTER"); break;
			case INPUT_REGISTER_TABLE: strcpy(mbtbl, "INPUT_REGISTER"); break;
			default: continue;
			}

		if((vslave[i].iface!=port)&&(port!=GATEWAY_NONE)) continue;

printf("\
<tr>\n\
<td>%d\n\
<td>%d\n\
<td>%s\n\
<td>%d\n\
<td>%d\n\
<td>%d\n\
<td>P%d\n\
<td style=\"text-align: left;\">&nbsp;%s\n",
		i+1,
		vslave[i].device,
		mbtbl,
		vslave[i].start+1,
		vslave[i].length,
		vslave[i].offset,
		vslave[i].iface+1,
		vslave[i].device_name);

		any_entry=1;
	  }
	
	if(any_entry==0) printf("<tr><td colspan=\"8\">Таблица не содержит записей.\n");

	printf("</table>\n");
	return;
	}
/*//-----------------------------------------------------------
void show_tcp_servers(char port)
	{
	int i;
	char any_entry=0;

printf("\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" align=\"center\" style=\"width:80%\" id=\"tcp_servers\">\n\
<caption>Таблица TCP_SERVERS</caption>\n\
<tr>\n\
<th>№<br /><span style=\"white-space:nowrap;\">п/п</span>\n\
<th>Адрес<br />ModBus\n\
<th>Адрес IP\n\
<th>Порт<br />TCP\n\
<th>Смещение<br />адреса\n\
<th>Порт\n\
<th>Наименование устройства\n\
\n");

	for(i=0; i<MAX_TCP_SERVERS; i++) {

		if(t_tcpsrv[i].mb_slave==0) continue;

		if((t_tcpsrv[i].p_num!=port)&&(port!=SERIAL_STUB)) continue;

printf("<tr>\n\
<td>%d\n\
<td>%d\n\
<td>%3.3d.%3.3d.%3.3d.%3.3d\n\
<td>%d\n\
<td>%d\n\
<td>P%d\n\
<td style=\"text-align: left;\">&nbsp;%s\n",
		i+1,
		t_tcpsrv[i].mb_slave,
		t_tcpsrv[i].ip>>24, t_tcpsrv[i].ip>>16&0xff, t_tcpsrv[i].ip>>8&0xff, t_tcpsrv[i].ip&0xff, 
		t_tcpsrv[i].port,
		t_tcpsrv[i].address_shift,
		t_tcpsrv[i].p_num+1,
		t_tcpsrv[i].device_name);

		any_entry=1;
	  }

	if(any_entry==0) printf("<tr><td colspan=\"7\">Таблица не содержит записей.\n");

	printf("</table>\n");
	return;
	}
*///-----------------------------------------------------------
void show_proxy_queries(char port)
	{
	int i;
	char mbtbl[20];
	char sClass[32];
	char any_entry=0;

printf("\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" align=\"center\" style=\"width:80%\" id=\"proxy_table\">\n\
<caption>Таблица PROXY_TABLE</caption>\n\
<tr>\n\
<th rowspan=\"2\">№<br /><span style=\"white-space:nowrap;\">п/п</span>\n\
<th rowspan=\"2\">Адрес<br />ModBus\n\
<th rowspan=\"2\">Таблица ModBus\n\
<th colspan=\"3\">Блок регистров\n\
<th rowspan=\"2\">Порт\n\
<th rowspan=\"2\">Задержка, мс\n\
<th rowspan=\"2\">Критичное<br />количество<br />запросов\n\
<th rowspan=\"2\">Наименование устройства\n\
<tr><th>Начало\n\
<th>Кол-во\n\
<th>Запись\n\
\n");

	for(i=0; i<MAX_QUERY_ENTRIES; i++) {

		switch(pquery[i].mbf) {
			case MBF_READ_COILS: strcpy(mbtbl, "COIL_STATUS"); break;
			case MBF_READ_DECRETE_INPUTS: strcpy(mbtbl, "INPUT_STATUS"); break;
			case MBF_READ_HOLDING_REGISTERS: strcpy(mbtbl, "HOLDING_REGISTER"); break;
			case MBF_READ_INPUT_REGISTERS: strcpy(mbtbl, "INPUT_REGISTER"); break;
			default: continue;
			}

		if((pquery[i].iface!=port)&&(port!=GATEWAY_NONE)) continue;

		if(pquery[i].status_bit==1)
			strcpy(sClass, "p_on");
			else strcpy(sClass, "p_err");

printf("<tr class=\"%s\">\n\
<td>%d\n\
<td>%d\n\
<td>%s\n\
<td>%d\n\
<td>%d\n\
<td>%d\n\
<td>P%d\n\
<td>%d\n\
<td>%d\n\
<td style=\"text-align: left;\">&nbsp;%s\n",
		sClass,
		i+1,
		pquery[i].device,
		mbtbl,
		pquery[i].start+1,
		pquery[i].length,
		pquery[i].offset+1,
		pquery[i].iface+1,
		pquery[i].delay,
		pquery[i].critical,
		pquery[i].device_name);

		any_entry=1;
	  }

	if(any_entry==0) printf("<tr><td colspan=\"10\">Таблица не содержит записей.\n");

	printf("</table>\n");
	return;
	}
///-----------------------------------------------------------
void show_stat_by_stage(u8 p_num)
{
/*
printf("\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" style=\"float:right;\" id=\"p_counters\">\n\
<tr><th colspan=\"3\">Опрос по стадиям\n\
<tr><td colspan=\"2\">Принято запросов TCP\n\
<td>%d\n\
<tr><td colspan=\"2\">Ошибки в ADU TCP\n\
<td>%d\n\
<tr><td colspan=\"2\">Ошибки в PDU TCP\n\
<td>%d\n\
<tr class=\"main_prm p_err\"><td colspan=\"2\">Таймауты связи\n\
<td>%d\n\
<tr class=\"main_prm p_err\"><td colspan=\"2\">Ошибки CRC\n\
<td>%d\n\
<tr><td colspan=\"2\">Ошибки в ADU RTU\n\
<td>%d\n\
<tr><td colspan=\"2\">Ошибки в PDU RTU\n\
<td>%d\n\
<tr><td colspan=\"2\">Ошибки нижнего уровня\n\
<td>%d\n\
<tr class=\"main_prm p_err\"><td colspan=\"2\">Ошибок всего\n\
<td>%d\n\
<tr class=\"main_prm p_on\"><td colspan=\"2\">Передано ответов\n\
<td>%d\n\
<tr><td rowspan=\"4\">Задержка<br />передачи<br />пакета\n\
<tr><td>минимальная\n\
<td>%d\n\
<tr class=\"main_prm p_on\"><td>текущая\n\
<td>%d\n\
<tr><td>максимальная\n\
<td>%d\n\
<tr class=\"main_prm p_on\"><td colspan=\"2\">Период опроса, мс\n\
<td>%d\n\
<tr><td colspan=\"4\" style=\"text-align:center;\">\n\
<form action=\"\" method=\"post\" name=\"port_stat\" style=\"padding:0; margin:0; border:none;\">\n\
<input type=\"hidden\" name=\"p_num\" value=\"1\">\n\
<input type=\"submit\" name=\"port_stat_reset\" value=\"Сбросить\">\n\
</form>\n\
</table>\n\
\n",
	iface_rtu[p_num].stat.accepted,
	iface_rtu[p_num].stat.errors_tcp_adu,
	iface_rtu[p_num].stat.errors_tcp_pdu,
	iface_rtu[p_num].stat.timeouts,
	iface_rtu[p_num].stat.crc_errors,
	iface_rtu[p_num].stat.errors_serial_adu,
	iface_rtu[p_num].stat.errors_serial_pdu,
	iface_rtu[p_num].stat.errors_tcp_sending,
	iface_rtu[p_num].stat.errors,
	iface_rtu[p_num].stat.sended,
	0,
	iface_rtu[p_num].stat.request_time,
	0,
	iface_rtu[p_num].stat.scan_rate
	);
*/
return;
}
///-----------------------------------------------------------
void show_stat_by_func(u8 P)
	{
	int i;
	char str[12];
	
	printf("\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" align=\"center\" id=\"p_mbfunc\">\n\
<caption>Опрос по функциям</caption>\n\n\
<tr>\n\
<th rowspan=\"2\">Пакеты\n\
<th colspan=\"4\">Функции чтения\n\
<th colspan=\"4\">Функции записи\n\
<th rowspan=\"2\">Другие\n\n\
<tr>\n<th>0x01\n<th>0x02\n<th>0x03\n<th>0x04\n<th>0x05\n<th>0x06\n<th>0x0f\n<th>0x10\n");

	for(i=0; i<STAT_RES_AMOUNT; i++) {
	
		switch(i) {
		  case STAT_RES_OK:  strcpy(str, "Успешные");		break;
		  case STAT_RES_ERR: strcpy(str, "Ошибки");			break;
		  case STAT_RES_EXP: strcpy(str, "Исключения"); break;
		  default: strcpy(str, "Default");
			}
		
		printf("\
<tr>\n<th class=\"row_th\">%s\n<td>%d\n<td>%d\n<td>%d\n<td>%d\n<td>%d\n<td>%d\n<td>%d\n<td>%d\n<td>%d\n\n",
			str,
			iface_rtu[P].stat.func[STAT_FUNC_0x01][i], iface_rtu[P].stat.func[STAT_FUNC_0x02][i], 
			iface_rtu[P].stat.func[STAT_FUNC_0x03][i], iface_rtu[P].stat.func[STAT_FUNC_0x04][i], 
			iface_rtu[P].stat.func[STAT_FUNC_0x05][i], iface_rtu[P].stat.func[STAT_FUNC_0x06][i], 
			iface_rtu[P].stat.func[STAT_FUNC_0x0f][i], iface_rtu[P].stat.func[STAT_FUNC_0x10][i], 
			iface_rtu[P].stat.func[STAT_FUNC_OTHER][i]);
		}

	printf("\
<tr><td colspan=\"10\" style=\"text-align:center;\">\n\
<form action=\"\" method=\"post\" name=\"port_clients\" style=\"padding:0; margin:0; border:none;\">\n\
<input type=\"hidden\" name=\"p_num\" value=\"1\">\n\
<input type=\"submit\" name=\"port_clients_reset_counters\" value=\"Сбросить\">\n\
</form>\n\
</table>\n");

	return;
	}
///-----------------------------------------------------------
void show_SL_settings(u8 p_num)
{
char sClass[32];
char sText[32];
char modeText[10][24];

strcpy(sClass, "p_on");
strcpy(sText, "ВКЛЮЧЕН");
memset(modeText, 0, sizeof(modeText));

switch(iface_rtu[p_num].modbus_mode) {
	case MODBUS_PORT_OFF:
		strcpy(sClass, "p_off");
		strcpy(sText, "ОТКЛЮЧЕН");
		strcpy(modeText[6], " selected=\"on\"");
		break;

	case MODBUS_PORT_ERROR:
		strcpy(sClass, "p_err");
		strcpy(sText, "СБОЙ");
		break;

	case GATEWAY_SIMPLE:	strcpy(modeText[0], " selected=\"on\""); break;
	case GATEWAY_ATM:			strcpy(modeText[1], " selected=\"on\""); break;
	case GATEWAY_RTM:			strcpy(modeText[2], " selected=\"on\""); break;
	case GATEWAY_PROXY:		strcpy(modeText[3], " selected=\"on\""); break;
	case BRIDGE_PROXY:		strcpy(modeText[4], " selected=\"on\""); break;
//	case BRIDGE_SIMPLE:		strcpy(modeText[5], " selected=\"on\""); break;
	default:;
	}

	printf("\
<div class=\"form_keeper\">\n\
<form action=\"\" method=\"post\" name=\"serial_port_settings\">\n\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" align=\"center\" class=\"psett\">\n\
<caption>Состояние интерфейса</caption>\n\
<tr><td colspan=\"2\" class=\"%s\">%s</td>\n\
<tr style=\"height:4px; font-size:4px; line-height: 4px;\"><td colspan=\"2\" style=\"border:none;\">&nbsp;</td>\n\
<tr><td colspan=\"2\" class=\"cpt\">Параметры интерфейса</td>\n\
<input type=\"hidden\" name=\"p_num\" value=\"1\">\n\
\n", sClass, sText);

	printf("\
<tr><td colspan=\"2\">\n\
<select name=\"mode\" style=\"width:100%;\">\n\
<option value=\"0\"%s>GATEWAY_SIMPLE</option>\n\
<option value=\"1\"%s>GATEWAY_ATM</option>\n\
<option value=\"2\"%s>GATEWAY_RTM</option>\n\
<option value=\"3\"%s>GATEWAY_PROXY</option>\n\
<option value=\"4\"%s>BRIDGE_PROXY</option>\n\
<option value=\"5\"%s>BRIDGE_SIMPLE</option>\n\
<option value=\"6\"%s>ОТКЛЮЧЕН</option>\n\
</select>\n\
<!-- td style=\"text-align:left;\">Режим -->\n\n",
	modeText[0], modeText[1], modeText[2], modeText[3], modeText[4], modeText[5], modeText[6]);

	if(iface_rtu[p_num].modbus_mode==GATEWAY_SIMPLE)
		printf("\
<tr><td class=\"f\">\n\
<input type=\"text\" name=\"tcp_port\" value=\"1502\" maxlength=\"5\" style=\"width:64pt;\" />\n\
<td style=\"text-align:left;\">Порт TCP\n\n", iface_rtu[p_num].Security.tcp_port);

	memset(modeText, 0, sizeof(modeText));
	if(strcmp(iface_rtu[p_num].serial.p_mode, "RS232")==0) strcpy(modeText[0], " selected=\"on\"");
	if(strcmp(iface_rtu[p_num].serial.p_mode, "RS422")==0) strcpy(modeText[1], " selected=\"on\"");
	if(strcmp(iface_rtu[p_num].serial.p_mode, "RS485_2w")==0) strcpy(modeText[2], " selected=\"on\"");
	if(strcmp(iface_rtu[p_num].serial.p_mode, "RS485_4w")==0) strcpy(modeText[3], " selected=\"on\"");

	if((strlen(modeText[0])==0)&&(strlen(modeText[1])==0)&&(strlen(modeText[2])==0)&&(strlen(modeText[3])==0))
		strcpy(modeText[4], " style=\"color:red;\"");

	printf("\
<tr%s><td class=\"f\">\n\
<select name=\"p_mode\">\n\
<option value=\"0\"%s>RS232</option>\n\
<option value=\"1\"%s>RS422</option>\n\
<option value=\"2\"%s>RS485 2W</option>\n\
<option value=\"3\"%s>RS485 4W</option>\n\
</select>\n\
<td style=\"text-align:left;\">Стандарт работы\n\n",
	modeText[4], modeText[0], modeText[1], modeText[2], modeText[3]);

	memset(modeText, 0, sizeof(modeText));
	if(strcmp(iface_rtu[p_num].serial.speed, "2400")==0)		strcpy(modeText[0], " selected=\"on\"");
	if(strcmp(iface_rtu[p_num].serial.speed, "4800")==0)		strcpy(modeText[1], " selected=\"on\"");
	if(strcmp(iface_rtu[p_num].serial.speed, "9600")==0)		strcpy(modeText[2], " selected=\"on\"");
	if(strcmp(iface_rtu[p_num].serial.speed, "14400")==0)		strcpy(modeText[3], " selected=\"on\"");
	if(strcmp(iface_rtu[p_num].serial.speed, "19200")==0)		strcpy(modeText[4], " selected=\"on\"");
	if(strcmp(iface_rtu[p_num].serial.speed, "38400")==0)		strcpy(modeText[5], " selected=\"on\"");
	if(strcmp(iface_rtu[p_num].serial.speed, "56000")==0)		strcpy(modeText[6], " selected=\"on\"");
	if(strcmp(iface_rtu[p_num].serial.speed, "57600")==0)		strcpy(modeText[7], " selected=\"on\"");
	if(strcmp(iface_rtu[p_num].serial.speed, "115200")==0)	strcpy(modeText[8], " selected=\"on\"");

	if(	(strlen(modeText[0])==0)&&
			(strlen(modeText[1])==0)&&
			(strlen(modeText[2])==0)&&
			(strlen(modeText[3])==0)&&
			(strlen(modeText[4])==0)&&
			(strlen(modeText[5])==0)&&
			(strlen(modeText[6])==0)&&
			(strlen(modeText[7])==0)&&
			(strlen(modeText[8])==0)) strcpy(modeText[9], " style=\"color:red;\"");

	printf("\
<tr%s><td class=\"f\">\n\
<select name=\"speed\">\n\
<!-- option value=\"\">110</option -->\n\
<!-- option value=\"\">300</option -->\n\
<!-- option value=\"\">600</option -->\n\
<!-- option value=\"\">1200</option -->\n\
<option value=\"0\"%s>2400</option>\n\
<option value=\"1\"%s>4800</option>\n\
<option value=\"2\"%s>9600</option>\n\
<option value=\"3\"%s>14400</option>\n\
<option value=\"4\"%s>19200</option>\n\
<option value=\"5\"%s>38400</option>\n\
<option value=\"6\"%s>56000</option>\n\
<option value=\"7\"%s>57600</option>\n\
<option value=\"8\"%s>115200</option>\n\
<!-- option value=\"\">128000</option -->\n\
</select>\n\
<td style=\"text-align:left;\">Скорость обмена\n",
	modeText[9], modeText[0], modeText[1], modeText[2], modeText[3],
	modeText[4], modeText[5], modeText[6], modeText[7], modeText[8]);

	memset(modeText, 0, sizeof(modeText));
	if(strcmp(iface_rtu[p_num].serial.parity, "none")==0) strcpy(modeText[0], " selected=\"on\"");
	if(strcmp(iface_rtu[p_num].serial.parity, "even")==0) strcpy(modeText[1], " selected=\"on\"");
	if(strcmp(iface_rtu[p_num].serial.parity, "odd")==0)	strcpy(modeText[2], " selected=\"on\"");

	if((strlen(modeText[0])==0)&&(strlen(modeText[1])==0)&&(strlen(modeText[2])==0))
		strcpy(modeText[3], " style=\"color:red;\"");

	printf("\
<tr%s><td class=\"f\">\n\
<select name=\"parity\">\n\
<option value=\"0\"%s>NONE</option>\n\
<option value=\"1\"%s>EVEN</option>\n\
<option value=\"2\"%s>ODD</option>\n\
</select>\n\n\
<td style=\"text-align:left;\">Контроль четности\n\
<tr><td class=\"f\">\n\
<input type=\"text\" name=\"timeout\" value=\"%d\" maxlength=\"5\" style=\"width:64pt;\" />\n\
<td style=\"text-align:left;\">Таймаут связи, мс\n\n",
	modeText[3], modeText[0], modeText[1], modeText[2],	iface_rtu[p_num].serial.timeout/1000);

printf("\
<tr><td class=\"f\">\n\
<select name=\"data_bits\" disabled=\"on\">\n\
<option value=\"0\" selected=\"on\">8&nbsp;</option>\n\
<option value=\"1\">7&nbsp;</option>\n\
</select>\n\
<td style=\"text-align:left;\">Биты данных\n\n\
<tr><td class=\"f\">\n\
<select name=\"stop_bits\" disabled=\"on\">\n\
<option value=\"0\" selected=\"on\">1</option>\n\
<option value=\"1\">1.5</option>\n\
<option value=\"2\">2</option>\n\
</select>\n\
<td style=\"text-align:left;\">Стоповые биты\n\n\
<tr><td class=\"f\">\n\
<select name=\"flow_control\" disabled=\"on\">\n\
<option value=\"0\">RTS/CTS</option>\n\
<option value=\"1\" selected=\"on\">XON/XOFF</option>\n\
</select>\n\
<td style=\"text-align:left;\">Контроль передачи\n\n\
<tr><td colspan=\"2\">\n\
<input type=\"submit\" name=\"serial_port_settings_apply\" value=\"Применить\">\n\
<input type=\"reset\" value=\"Отмена\">\n\n\
</table>\n\
</form></div>\n");

return;
}
/*//-----------------------------------------------------------
void show_interface_clients(u8 P)
{

//	if((iface_rtu[P].modbus_mode!=GATEWAY_SIMPLE)&&(iface_rtu[P].modbus_mode!=BRIDGE_SIMPLE)) return;
	if(iface_rtu[P].modbus_mode!=GATEWAY_SIMPLE) return;

  char pstatus[128], pmode[32], row_class[32];
  int i;
  
//	if(iface_rtu[P].modbus_mode==BRIDGE_SIMPLE) {
//	  strcpy(pmode, "сервера");
//	  strcpy(pstatus, "\n<th rowspan=\"2\">Адрес<br />modbus\n<th rowspan=\"2\">Сдвиг<br />адреса<br />опроса\n");
//	  } else {
	  	strcpy(pmode, "клиенты");
	  	strcpy(pstatus, "\n");
//	    }

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

  if(//(
  	 (iface_rtu[P].clients[i].connection_status!=MB_CONNECTION_ESTABLISHED//)&&
//  	 (iface_rtu[P].modbus_mode!=BRIDGE_SIMPLE)	)||(
//  	 (iface_rtu[P].modbus_mode==BRIDGE_SIMPLE)&&
//  	 (i>=iface_rtu[P].accepted_connections_number)	
)	) {
printf("\
<tr class=\"p_off\">\n\
<td>&nbsp;\n\
<td>&nbsp;\n\
<td>&nbsp;\n\
<td>&nbsp;\n\
<td>&nbsp;\n\
<td>&nbsp;\n\
");
//		if(iface_rtu[P].modbus_mode==BRIDGE_SIMPLE)
//		  printf("<td>&nbsp;\n<td>&nbsp;\n");

  	continue;
  	}

	int diff=	iface_rtu[P].clients[i].connection_time==0?0:\
						difftime(moment, iface_rtu[P].clients[i].connection_time);

	sprintf(pmode, "%3.3d.%3.3d.%3.3d.%3.3d",	iface_rtu[P].clients[i].ip >> 24,
																(iface_rtu[P].clients[i].ip >> 16) & 0xff,
																(iface_rtu[P].clients[i].ip >> 8) & 0xff,
																iface_rtu[P].clients[i].ip & 0xff);
																
//	if(iface_rtu[P].modbus_mode==BRIDGE_SIMPLE)
//		sprintf(pstatus, "%d", iface_rtu[P].clients[i].mb_slave);
//		else 
strcpy(pstatus, "N/A");
		
	strcpy(row_class, "p_on");
//	if((iface_rtu[P].modbus_mode==BRIDGE_SIMPLE)&&
//		 (iface_rtu[P].clients[i].connection_status!=MB_CONNECTION_ESTABLISHED)) strcpy(row_class, "p_err");

printf("\
<tr class=\"%s\">\n\
<td style=\"text-align:left;\">%s:%d\n\
",	row_class, pmode, iface_rtu[P].clients[i].port);
  		
//	if(iface_rtu[P].modbus_mode==BRIDGE_SIMPLE) {
//	  if(iface_rtu[P].clients[i].address_shift!=MB_ADDRESS_NO_SHIFT)
//		  sprintf(pmode, "%+d", iface_rtu[P].clients[i].address_shift);
//		  else 
sprintf(pmode, "Нет");
		printf("\
<td>%s\n\
<td>%s\n\
",	pstatus, pmode);
//	  }
			
  if(iface_rtu[P].clients[i].stat.scan_rate<MB_SCAN_RATE_INFINITE)
    sprintf(pstatus, "%d", iface_rtu[P].clients[i].stat.scan_rate);
    else sprintf(pstatus, "---");

printf("\
<td style=\"white-space:nowrap;\">%3.3dд %2.2d:%2.2d:%2.2d\n\
<td>%d\n\
<td>%d\n\
<td>%d\n\
<td>%s\n\
",	  (diff/86400)%1000, (diff/3600)%24, (diff/60)%60, diff%60,
  		iface_rtu[P].clients[i].stat.sended,
  		iface_rtu[P].clients[i].stat.errors,
  		iface_rtu[P].clients[i].stat.request_time,
  		pstatus);
  		
	}

//pmode[0]=iface_rtu[P].modbus_mode==BRIDGE_SIMPLE?'4':'2';
pmode[0]='2';

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
*///-----------------------------------------------------------
int process_input_data()
	{
/*	int i, contlen;
	char **p, srv[32];

	p = environ;
	while(*p != NULL) {
		if(strncmp(*p, "CONTENT_LENGTH", 14)==0) break;
		//printf("%s<br>\n",*p);
		p++;
	  }

	if(*p!=NULL) contlen=atoi(&(*p)[15]); else {
		printf("\
Content-Type: text/html\r\n\r\n\
<div class=\"err_block\">\
Ошибка при разборе параметров ввода.\
</div>\
\n");
		return 0;
		}

	p = environ;
	while(*p != NULL) {if(strncmp(*p, "SERVER_NAME", 11)==0) break; p++;}
	strcpy(srv, &(*p)[12]);

	char form_field[MAX_FORM_FIELDS][MAX_FORM_FIELD_LEN];
	char form_value[MAX_FORM_FIELDS][MAX_FORM_VALUE_LEN];

	memset(form_field, 0, sizeof(form_field));
	memset(form_value, 0, sizeof(form_value));

	char ch;
	int f=0, v=-1, listlen=0;
//	printf("\nSTDIN: ");
	for(i=0; i<contlen; i++) {
		ch = getchar();
//		printf("%c", ch);
		if(ch=='=') {form_field[listlen][f]=0; f=-1; v=0; continue;}
		if(ch=='&') {form_value[listlen][v]=0; v=-1; f=0;
			if(listlen<MAX_FORM_FIELDS-1) listlen++; else break;
			continue;
			}
		if((f>-1)&&(f<MAX_FORM_FIELD_LEN)) form_field[listlen][f++]=ch;
		if((v>-1)&&(v<MAX_FORM_VALUE_LEN)) form_value[listlen][v++]=ch;
		}
//	printf("\n");

 	int form_id=-1, action_id=-1;
//	printf("\
Content-Type: text/html\r\n\r\n\
<table border=\"1\"><tr><th>form_field<th>form_value\n");
	for(i=0; i<MAX_FORM_FIELDS; i++)
		if(strlen(form_field[i])>0) {
//			printf("<tr><td>%s<td>%s\n", form_field[i], form_value[i]);
			if(strcmp(form_field[i], "form_id")==0) form_id=i;

			if(strcmp(form_field[i], "save")==0)		action_id=FORM_ACTION_SAVE;
			if(strcmp(form_field[i], "apply")==0)		action_id=FORM_ACTION_APPLY;
			if(strcmp(form_field[i], "reset")==0)		action_id=FORM_ACTION_RESET;
			if(strcmp(form_field[i], "reboot")==0)	action_id=FORM_ACTION_REBOOT;
			}
//	printf("</table>\n");
		
//printf("Content-Type: text/html\r\n\r\n");

	if(strcmp(form_value[form_id], "gateway_settings")==0) switch(action_id) {
		case FORM_ACTION_REBOOT:
			security->halt=1;
			printf("Location: http://%s/index.shtml\r\n\r\n", srv);
//			printf("\
Content-Type: text/html\r\n\r\n\
<div class=\"err_block\">Modbus-шлюз остановлен. Перейдите на главную</div>\n");
//printf("HELLO WORLD!");
			break;

		case FORM_ACTION_APPLY:

			f=0;
			for(i=0; i<MAX_FORM_FIELDS; i++)
				if(strlen(form_field[i])>0)
					if(strcmp(form_field[i], "backlight_enabled")==0) f=1;
			security->back_light=f;

			f=0;
			for(i=0; i<MAX_FORM_FIELDS; i++)
				if(strlen(form_field[i])>0)
					if(strcmp(form_field[i], "buzzer_enabled")==0) f=1;
			gate->use_buzzer=f;

		default: printf("Location: http://%s/index.shtml?SETTINGS\r\n\r\n", srv);
		}

//printf("HELLO WORLD!!!");
*/
	return 0;
	}
///-----------------------------------------------------------
