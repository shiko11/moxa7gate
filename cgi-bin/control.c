/* env.cgi.c */

#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <errno.h>

#include "../global.h"
#include "../monitoring.h"

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

input_cfg *shared_memory;
input_cfg_502 *gate;
GW_EventLog *app_log;

RT_Table_Entry *t_rtm; //[MAX_VIRTUAL_SLAVES];
Query_Table_Entry *t_proxy; //[MAX_QUERY_ENTRIES];
GW_TCP_Server *t_tcpsrv; //[MAX_TCP_SERVERS];

key_t access_key;
time_t moment;

void main_table(input_cfg *shared_memory);
void port_info(int port);
void settings();
void show_events(GW_EventLog *app_log, input_cfg_502 *gate);
void show_vslaves(char port);
void show_tcp_servers(char port);
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
Content-Type: text/html\r\n\r\n\
<div class=\"err_block\">\
Modbus-���� �� �������� �� ���� ���������� MOXA. ��� ������: %s\
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
Content-Type: text/html\r\n\r\n\
<div class=\"err_block\">\
Modbus-���� �� �������� %d ������(�).\
</div>\
\n", diff);
		return 0;
	  }

///----------------------------------------------------------------------------------------
	int t, i;
	char **p;

	p = environ;
	while(*p != NULL) {if(strncmp(*p, "REQUEST_METHOD", 14)==0) break; p++;}
	if(	(*p!=NULL) && (strncmp(&(*p)[15], "POST", 4)==0) ) return process_input_data();

//	char sel1[16], sel2[16], sel3[16], sel4[16];
//  sel1[0]=sel2[0]=sel3[0]=sel4[0]=0;
//	if(shm_data->ATM)		strcpy(sel2, " class=\"sel\"");
//	if(shm_data->RTM && !shm_data->PROXY)		strcpy(sel3, " class=\"sel\"");
//	if(shm_data->PROXY)	strcpy(sel4, " class=\"sel\"");
//  if(!(shm_data->ATM || shm_data->RTM || shm_data->PROXY)) strcpy(sel1, " class=\"sel\"");

printf("Content-Type: text/html\r\n\r\n"); 

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
<td class=\"sel\">�����\n\
<td%s>GATEWAY SIMPLE\n\
<td%s>GATEWAY ATM\n\
<td%s>GATEWAY RTM\n\
<td%s style=\"border-right:none;\">GATEWAY PROXY\n\
</table>\n\
\n", sel1, sel2, sel3, sel4);*/

///----------------------------------------------------------------------------------------

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
		gate->halt=1;
		printf("<div class=\"err_block\">Modbus-���� ����������. ��������� �� �������</div>\n");
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
  	show_events(app_log, gate);
  	break;

  case DISPLAY_QUERY_TABLES:
		printf("<h1 align=\"center\">������� ������</h1>\n");
  	show_vslaves(SERIAL_STUB);
		printf("<p>&nbsp;</p>\n");
  	show_tcp_servers(SERIAL_STUB);
		printf("<p>&nbsp;</p>\n");
  	show_proxy_queries(SERIAL_STUB);
  	break;

  case DISPLAY_MAIN_TABLE:
  default:
		printMOXA_UC7410(moment);
		main_table(shared_memory);
  }

shmdt(shared_memory);
return 0;
} 
///-----------------------------------------------------------
void main_table(input_cfg *shared_memory)
{

printf("\
<h1>���������� ������</h1>\n\n\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" align=\"center\" id=\"m_stat_tbl\">\n\
<caption>���� MODBUS</caption>\n\
<tr>\n\
<th rowspan=\"2\" class=\"f\">����\n\
<th rowspan=\"2\">���������\n\
<th rowspan=\"2\">�����\n\
<th colspan=\"3\">��������� ModBus\n\
<th rowspan=\"2\">�����<br />ModBus\n\
<th rowspan=\"2\">����<br />TCP\n\
<tr>\n\
<th>��������\n\
<th>������\n\
<th>��������\n\
\n");

  char pstatus[12], row_class[6];
  int i;
  for(i=0; i<MAX_MOXA_PORTS; i++) {
  	strcpy(pstatus, "�������");
  	strcpy(row_class, "p_on");
		if(shared_memory[i].modbus_mode==MODBUS_PORT_OFF) {
	  	strcpy(pstatus, "��������");
	  	strcpy(row_class, "p_off");
			}
		if(shared_memory[i].modbus_mode==MODBUS_PORT_ERROR) {
	  	strcpy(pstatus, "����");
	  	strcpy(row_class, "p_err");
			}

		printf("\
<tr class=\"%s\">\n\
<td class=\"f\" title=\"���� P%d. %s\"><a href=\"index.shtml?PORT%d\">P%d</a>\n\
<td class=\"pstatus\">%s\n",
						row_class, i+1, shared_memory[i].description, i+1, i+1, pstatus);

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
	  printf("<td>%d\n", shared_memory[i].stat.request_time);
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
<tr><td colspan=\"8\" class=\"f\">\n\
<form action=\"\" method=\"post\" name=\"all_ports\" style=\"padding:0; margin:0; border:none;\">\n\
<input type=\"submit\" name=\"all_ports_reset_counters\" value=\"�������� �������� ���������\">\n\
</form>\n\
<tr><td colspan=\"8\" style=\"border:none; text-align:left;\">\n\
* ��� ��������� �������������� ���������� �������� ��������� ���� �� ������ ������� ������ �������.\n\
</table>\n\
\n");

return;
}
///-----------------------------------------------------------
void port_info(int port)
{
int P=0;
if(((port-1)>=SERIAL_P1)&&((port-1)<=SERIAL_P8)) P=port-1; 
	else {
		printf("<div class=\"err_block\">������� ������ ����� ����������������� ����������. �������� ������</div>\n");
		return;
		}

	show_stat_by_stage(P);
	show_SL_settings(P);

	printf("\
<h1>���� P%d. %s</h1>",
	P+1, shared_memory[P].description);

	show_stat_by_func(P);

	switch(shared_memory[P].modbus_mode) {
		case GATEWAY_SIMPLE:
		case BRIDGE_SIMPLE:
  		show_interface_clients(P);
			break;

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

	if(gate->watchdog_timer==1)	strcpy(wdt, " checked=\"on\""); else wdt[0]=0;
	if(gate->use_buzzer==1)			strcpy(buz, " checked=\"on\""); else buz[0]=0;
	if(gate->back_light==1)			strcpy(bl,  " checked=\"on\""); else bl[0]=0;

	printf("\
<h1>��������� �����</h1>\n\n\
<form action=\"/cgi-bin/control\" method=\"POST\" name=\"gateway_settings\" id=\"gw_sett_form\">\n\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" align=\"center\" id=\"gw_sett\">\n\
<caption>���������</caption>\n\n\
<tr><td class=\"f\">\n\
<input type=\"text\" name=\"ip_address\" value=\"%3.3d.%3.3d.%3.3d.%3.3d\" size=\"15\" maxlength=\"15\" readonly=\"on\" />\n\
<td>����� IP\n\n\
<tr><td class=\"f\">\n\
<input type=\"text\" name=\"tcp_port\" value=\"%d\" size=\"4\" maxlength=\"5\" readonly=\"on\" />\n\
<td>���� TCP\n\n\
<tr><td class=\"f\">\n\
<input type=\"text\" name=\"mb_address\" value=\"%d\" size=\"4\" maxlength=\"3\" readonly=\"on\" />\n\
<td>����� ModBus\n\n\
<tr><td class=\"f\">\n\
<input type=\"text\" name=\"status_info\" value=\"%d\" size=\"4\" maxlength=\"5\" readonly=\"on\" />\n\
<td>����� ����� ��������������� ����������\n\n\
<tr><td class=\"f\">\n\
<input type=\"checkbox\" name=\"wdt_enabled\"%s />\n\
<td>WatchDog ������ �������\n\n\
<tr><td class=\"f\">\n\
<input type=\"checkbox\" name=\"buzzer_enabled\"%s />\n\
<td>�������� ������ ��� ������\n\n\
<tr><td class=\"f\">\n\
<input type=\"checkbox\" name=\"backlight_enabled\"%s />\n\
<td>������� ��������� �������\n\n\
<tr><td colspan=\"2\" style=\"text-align: center;\">\n\
<input type=\"submit\" name=\"apply\" value=\"���������\">\n\
<input type=\"reset\" value=\"������\">\n\
<input type=\"submit\" name=\"reboot\" value=\"������������ MOXA\">\n\
<input type=\"hidden\" name=\"form_id\" value=\"gateway_settings\">\n\
</table>\n\
</form>\n",
	gate->IPAddress>>24, gate->IPAddress>>16&0xff, gate->IPAddress>>8&0xff, gate->IPAddress&0xff,
	gate->tcp_port,
	gate->modbus_address,
	gate->status_info+1,																 
	wdt, buz, bl);

	printf("\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" align=\"center\" id=\"gw_memory\">\
<caption>���������� ModBus �������� MOXA</caption>\n\
<tr>\n\
<th>�������\n\
<th>��������� �����\n\n\
<th>����������\n\
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
void show_events(GW_EventLog *app_log, input_cfg_502 *gate)
{

		printf("\
<h1 align=\"center\">������ �������</h1>\n\n\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" align=\"center\" style=\"width:80%\" id=\"events_table\">\n\
<caption>&nbsp;</caption>\n\n\
<tr>\n\
<th>�\n\
<th>�����\n\
<th>���\n\
<th>��������\n\
<th>���������\n\
\n");

	struct tm *tmd;
  int i, j;
	char timestr[32], sourcestr[16], typestr[8];
	char eventmsg[EVENT_MESSAGE_LENGTH];

  j=gate->app_log_current_entry;
	for(i=0; i<EVENT_LOG_LENGTH; i++) {

	if(app_log[j].msgtype==0) {
		j=j==EVENT_LOG_LENGTH-1?0:j+1;
		continue;
		}

	tmd=gmtime(&app_log[j].time);
	sprintf(timestr, "%2.2d.%2.2d.%4.4d %2.2d:%2.2d:%2.2d", tmd->tm_mday, tmd->tm_mon+1, tmd->tm_year+1900, tmd->tm_hour, tmd->tm_min, tmd->tm_sec);

	switch(app_log[j].msgtype & EVENT_TYPE_MASK) {
			case EVENT_TYPE_INF: sprintf(typestr, "INF"); break;
			case EVENT_TYPE_WRN: sprintf(typestr, "WRN"); break;
			case EVENT_TYPE_ERR: sprintf(typestr, "ERR"); break;
			default: sprintf(typestr, "***");
			}

	switch(app_log[j].msgtype & EVENT_SRC_MASK) {
			case EVENT_SRC_SYSTEM: 	sprintf(sourcestr, "SYSTEM"); 	break;
			case EVENT_SRC_GATE502: sprintf(sourcestr, "GATE502"); 	break;
			case EVENT_SRC_P1: 			sprintf(sourcestr, "PORT1"); 		break;
			case EVENT_SRC_P2: 			sprintf(sourcestr, "PORT2"); 		break;
			case EVENT_SRC_P3: 			sprintf(sourcestr, "PORT3"); 		break;
			case EVENT_SRC_P4: 			sprintf(sourcestr, "PORT4"); 		break;
			case EVENT_SRC_P5: 			sprintf(sourcestr, "PORT5"); 		break;
			case EVENT_SRC_P6: 			sprintf(sourcestr, "PORT6"); 		break;
			case EVENT_SRC_P7: 			sprintf(sourcestr, "PORT7"); 		break;
			case EVENT_SRC_P8: 			sprintf(sourcestr, "PORT8"); 		break;
			case EVENT_SRC_MOXAMB: 	sprintf(sourcestr, "MOXAMB"); 	break;
			case EVENT_SRC_MOXATCP: sprintf(sourcestr, "MOXATCP"); 	break;

			default: 								sprintf(sourcestr, "NONAME");
			}

	eventmsg[0]=0;
	make_msgstr(app_log[j].msgcode, eventmsg, app_log[j].prm[0], app_log[j].prm[1], app_log[j].prm[2], app_log[j].prm[3]);

	printf("\
<tr>\n\
<td>%d\n\
<td>%s\n\
<td>%s\n\
<td>%s\n\
<td>%s\n\
\n", j+1, timestr, typestr, sourcestr, eventmsg);

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
<tr><th colspan=\"2\"><a href=\"http://www.moxa.com/Product/UC-7400.htm\" style=\"color:white;\" title=\"������� �� ����������� ���� ����������\">MOXA UC-7410</a>\n\
<tr><td colspan=\"2\">������� �������\n\
<tr><td>RAM<td class=\"val\">00%\n\
<tr><td>CPU<td class=\"val\">00%\n\
<tr><td>FLASH<td class=\"val\">00%\n\
<tr><td colspan=\"2\">������ ������������\n\
<tr><td colspan=\"2\" class=\"val\">%s\n\
<tr><td colspan=\"2\">����� ������� �����\n\
<tr><td colspan=\"2\" class=\"val\">01.01.1970 00:00\n\
<tr><td colspan=\"2\">����� ������ �����\n\
<tr><td colspan=\"2\" class=\"val\">%3.3d� %2.2d:%2.2d:%2.2d\n\
</table>\n\
\n", gate->version, (diff/86400)%1000, (diff/3600)%24, (diff/60)%60, diff%60);
}
///-----------------------------------------------------------
void show_vslaves(char port)
	{
	int i;
	char mbtbl[20];
	char any_entry=0;

printf("\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" align=\"center\" style=\"width:80%\" id=\"rtm_table\">\n\
<caption>������� RTM_TABLE</caption>\n\
<tr>\n\
<th>�<br /><span style=\"white-space:nowrap;\">�/�</span>\n\
<th>�����<br />ModBus\n\
<th>������� ModBus\n\
<th>���������<br />�����\n\
<th>������<br />���������\n\
<th>��������<br />���������\n\
<th>����\n\
<th>������������ ����������\n\
\n");

	for(i=0; i<MAX_VIRTUAL_SLAVES; i++) {

		switch(t_rtm[i].modbus_table) {
			case COIL_STATUS_TABLE: strcpy(mbtbl, "COIL_STATUS"); break;
			case INPUT_STATUS_TABLE: strcpy(mbtbl, "INPUT_STATUS"); break;
			case HOLDING_REGISTER_TABLE: strcpy(mbtbl, "HOLDING_REGISTER"); break;
			case INPUT_REGISTER_TABLE: strcpy(mbtbl, "INPUT_REGISTER"); break;
			default: continue;
			}

		if((t_rtm[i].port!=port)&&(port!=SERIAL_STUB)) continue;

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
		t_rtm[i].device,
		mbtbl,
		t_rtm[i].start+1,
		t_rtm[i].length,
		t_rtm[i].address_shift,
		t_rtm[i].port+1,
		t_rtm[i].device_name);

		any_entry=1;
	  }
	
	if(any_entry==0) printf("<tr><td colspan=\"8\">������� �� �������� �������.\n");

	printf("</table>\n");
	return;
	}
///-----------------------------------------------------------
void show_tcp_servers(char port)
	{
	int i;
	char any_entry=0;

printf("\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" align=\"center\" style=\"width:80%\" id=\"tcp_servers\">\n\
<caption>������� TCP_SERVERS</caption>\n\
<tr>\n\
<th>�<br /><span style=\"white-space:nowrap;\">�/�</span>\n\
<th>�����<br />ModBus\n\
<th>����� IP\n\
<th>����<br />TCP\n\
<th>��������<br />������\n\
<th>����\n\
<th>������������ ����������\n\
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

	if(any_entry==0) printf("<tr><td colspan=\"7\">������� �� �������� �������.\n");

	printf("</table>\n");
	return;
	}
///-----------------------------------------------------------
void show_proxy_queries(char port)
	{
	int i;
	char mbtbl[20];
	char sClass[32];
	char any_entry=0;

printf("\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" align=\"center\" style=\"width:80%\" id=\"proxy_table\">\n\
<caption>������� PROXY_TABLE</caption>\n\
<tr>\n\
<th rowspan=\"2\">�<br /><span style=\"white-space:nowrap;\">�/�</span>\n\
<th rowspan=\"2\">�����<br />ModBus\n\
<th rowspan=\"2\">������� ModBus\n\
<th colspan=\"3\">���� ���������\n\
<th rowspan=\"2\">����\n\
<th rowspan=\"2\">��������, ��\n\
<th rowspan=\"2\">���������<br />����������<br />��������\n\
<th rowspan=\"2\">������������ ����������\n\
<tr><th>������\n\
<th>���-��\n\
<th>������\n\
\n");

	for(i=0; i<MAX_QUERY_ENTRIES; i++) {

		switch(t_proxy[i].mbf) {
			case MBF_READ_COILS: strcpy(mbtbl, "COIL_STATUS"); break;
			case MBF_READ_DECRETE_INPUTS: strcpy(mbtbl, "INPUT_STATUS"); break;
			case MBF_READ_HOLDING_REGISTERS: strcpy(mbtbl, "HOLDING_REGISTER"); break;
			case MBF_READ_INPUT_REGISTERS: strcpy(mbtbl, "INPUT_REGISTER"); break;
			default: continue;
			}

		if((t_proxy[i].port!=port)&&(port!=SERIAL_STUB)) continue;

		if(t_proxy[i].status_bit==1)
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
		t_proxy[i].device,
		mbtbl,
		t_proxy[i].start+1,
		t_proxy[i].length,
		t_proxy[i].offset+1,
		t_proxy[i].port+1,
		t_proxy[i].delay,
		t_proxy[i].critical,
		t_proxy[i].device_name);

		any_entry=1;
	  }

	if(any_entry==0) printf("<tr><td colspan=\"10\">������� �� �������� �������.\n");

	printf("</table>\n");
	return;
	}
///-----------------------------------------------------------
void show_stat_by_stage(u8 p_num)
{

printf("\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" style=\"float:right;\" id=\"p_counters\">\n\
<tr><th colspan=\"3\">����� �� �������\n\
<tr><td colspan=\"2\">������� �������� TCP\n\
<td>%d\n\
<tr><td colspan=\"2\">������ � ADU TCP\n\
<td>%d\n\
<tr><td colspan=\"2\">������ � PDU TCP\n\
<td>%d\n\
<tr class=\"main_prm p_err\"><td colspan=\"2\">�������� �����\n\
<td>%d\n\
<tr class=\"main_prm p_err\"><td colspan=\"2\">������ CRC\n\
<td>%d\n\
<tr><td colspan=\"2\">������ � ADU RTU\n\
<td>%d\n\
<tr><td colspan=\"2\">������ � PDU RTU\n\
<td>%d\n\
<tr><td colspan=\"2\">������ ������� ������\n\
<td>%d\n\
<tr class=\"main_prm p_err\"><td colspan=\"2\">������ �����\n\
<td>%d\n\
<tr class=\"main_prm p_on\"><td colspan=\"2\">�������� �������\n\
<td>%d\n\
<tr><td rowspan=\"4\">��������<br />��������<br />������\n\
<tr><td>�����������\n\
<td>%d\n\
<tr class=\"main_prm p_on\"><td>�������\n\
<td>%d\n\
<tr><td>������������\n\
<td>%d\n\
<tr class=\"main_prm p_on\"><td colspan=\"2\">������ ������, ��\n\
<td>%d\n\
<tr><td colspan=\"4\" style=\"text-align:center;\">\n\
<form action=\"\" method=\"post\" name=\"port_stat\" style=\"padding:0; margin:0; border:none;\">\n\
<input type=\"hidden\" name=\"p_num\" value=\"1\">\n\
<input type=\"submit\" name=\"port_stat_reset\" value=\"��������\">\n\
</form>\n\
</table>\n\
\n",
	shared_memory[p_num].stat.accepted,
	shared_memory[p_num].stat.errors_tcp_adu,
	shared_memory[p_num].stat.errors_tcp_pdu,
	shared_memory[p_num].stat.timeouts,
	shared_memory[p_num].stat.crc_errors,
	shared_memory[p_num].stat.errors_serial_adu,
	shared_memory[p_num].stat.errors_serial_pdu,
	shared_memory[p_num].stat.errors_tcp_sending,
	shared_memory[p_num].stat.errors,
	shared_memory[p_num].stat.sended,
	0,
	shared_memory[p_num].stat.request_time,
	0,
	shared_memory[p_num].stat.scan_rate
	);

return;
}
///-----------------------------------------------------------
void show_stat_by_func(u8 P)
	{
	int i;
	char str[12];
	
	printf("\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" align=\"center\" id=\"p_mbfunc\">\n\
<caption>����� �� ��������</caption>\n\n\
<tr>\n\
<th rowspan=\"2\">������\n\
<th colspan=\"4\">������� ������\n\
<th colspan=\"4\">������� ������\n\
<th rowspan=\"2\">������\n\n\
<tr>\n<th>0x01\n<th>0x02\n<th>0x03\n<th>0x04\n<th>0x05\n<th>0x06\n<th>0x0f\n<th>0x10\n");

	for(i=0; i<STAT_RES_AMOUNT; i++) {
	
		switch(i) {
		  case STAT_RES_OK:  strcpy(str, "��������");		break;
		  case STAT_RES_ERR: strcpy(str, "������");			break;
		  case STAT_RES_EXP: strcpy(str, "����������"); break;
		  default: strcpy(str, "Default");
			}
		
		printf("\
<tr>\n<th class=\"row_th\">%s\n<td>%d\n<td>%d\n<td>%d\n<td>%d\n<td>%d\n<td>%d\n<td>%d\n<td>%d\n<td>%d\n\n",
			str,
			shared_memory[P].stat.func[STAT_FUNC_0x01][i], shared_memory[P].stat.func[STAT_FUNC_0x02][i], 
			shared_memory[P].stat.func[STAT_FUNC_0x03][i], shared_memory[P].stat.func[STAT_FUNC_0x04][i], 
			shared_memory[P].stat.func[STAT_FUNC_0x05][i], shared_memory[P].stat.func[STAT_FUNC_0x06][i], 
			shared_memory[P].stat.func[STAT_FUNC_0x0f][i], shared_memory[P].stat.func[STAT_FUNC_0x10][i], 
			shared_memory[P].stat.func[STAT_FUNC_OTHER][i]);
		}

	printf("\
<tr><td colspan=\"10\" style=\"text-align:center;\">\n\
<form action=\"\" method=\"post\" name=\"port_clients\" style=\"padding:0; margin:0; border:none;\">\n\
<input type=\"hidden\" name=\"p_num\" value=\"1\">\n\
<input type=\"submit\" name=\"port_clients_reset_counters\" value=\"��������\">\n\
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
strcpy(sText, "�������");
memset(modeText, 0, sizeof(modeText));

switch(shared_memory[p_num].modbus_mode) {
	case MODBUS_PORT_OFF:
		strcpy(sClass, "p_off");
		strcpy(sText, "��������");
		strcpy(modeText[6], " selected=\"on\"");
		break;

	case MODBUS_PORT_ERROR:
		strcpy(sClass, "p_err");
		strcpy(sText, "����");
		break;

	case GATEWAY_SIMPLE:	strcpy(modeText[0], " selected=\"on\""); break;
	case GATEWAY_ATM:			strcpy(modeText[1], " selected=\"on\""); break;
	case GATEWAY_RTM:			strcpy(modeText[2], " selected=\"on\""); break;
	case GATEWAY_PROXY:		strcpy(modeText[3], " selected=\"on\""); break;
	case BRIDGE_PROXY:		strcpy(modeText[4], " selected=\"on\""); break;
	case BRIDGE_SIMPLE:		strcpy(modeText[5], " selected=\"on\""); break;
	default:;
	}

	printf("\
<div class=\"form_keeper\">\n\
<form action=\"\" method=\"post\" name=\"serial_port_settings\">\n\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" align=\"center\" class=\"psett\">\n\
<caption>��������� ����������</caption>\n\
<tr><td colspan=\"2\" class=\"%s\">%s</td>\n\
<tr style=\"height:4px; font-size:4px; line-height: 4px;\"><td colspan=\"2\" style=\"border:none;\">&nbsp;</td>\n\
<tr><td colspan=\"2\" class=\"cpt\">��������� ����������</td>\n\
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
<option value=\"6\"%s>��������</option>\n\
</select>\n\
<!-- td style=\"text-align:left;\">����� -->\n\n",
	modeText[0], modeText[1], modeText[2], modeText[3], modeText[4], modeText[5], modeText[6]);

	if(shared_memory[p_num].modbus_mode==GATEWAY_SIMPLE)
		printf("\
<tr><td class=\"f\">\n\
<input type=\"text\" name=\"tcp_port\" value=\"1502\" maxlength=\"5\" style=\"width:64pt;\" />\n\
<td style=\"text-align:left;\">���� TCP\n\n", shared_memory[p_num].tcp_port);

	memset(modeText, 0, sizeof(modeText));
	if(strcmp(shared_memory[p_num].serial.p_mode, "RS232")==0) strcpy(modeText[0], " selected=\"on\"");
	if(strcmp(shared_memory[p_num].serial.p_mode, "RS422")==0) strcpy(modeText[1], " selected=\"on\"");
	if(strcmp(shared_memory[p_num].serial.p_mode, "RS485_2w")==0) strcpy(modeText[2], " selected=\"on\"");
	if(strcmp(shared_memory[p_num].serial.p_mode, "RS485_4w")==0) strcpy(modeText[3], " selected=\"on\"");

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
<td style=\"text-align:left;\">�������� ������\n\n",
	modeText[4], modeText[0], modeText[1], modeText[2], modeText[3]);

	memset(modeText, 0, sizeof(modeText));
	if(strcmp(shared_memory[p_num].serial.speed, "2400")==0)		strcpy(modeText[0], " selected=\"on\"");
	if(strcmp(shared_memory[p_num].serial.speed, "4800")==0)		strcpy(modeText[1], " selected=\"on\"");
	if(strcmp(shared_memory[p_num].serial.speed, "9600")==0)		strcpy(modeText[2], " selected=\"on\"");
	if(strcmp(shared_memory[p_num].serial.speed, "14400")==0)		strcpy(modeText[3], " selected=\"on\"");
	if(strcmp(shared_memory[p_num].serial.speed, "19200")==0)		strcpy(modeText[4], " selected=\"on\"");
	if(strcmp(shared_memory[p_num].serial.speed, "38400")==0)		strcpy(modeText[5], " selected=\"on\"");
	if(strcmp(shared_memory[p_num].serial.speed, "56000")==0)		strcpy(modeText[6], " selected=\"on\"");
	if(strcmp(shared_memory[p_num].serial.speed, "57600")==0)		strcpy(modeText[7], " selected=\"on\"");
	if(strcmp(shared_memory[p_num].serial.speed, "115200")==0)	strcpy(modeText[8], " selected=\"on\"");

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
<td style=\"text-align:left;\">�������� ������\n",
	modeText[9], modeText[0], modeText[1], modeText[2], modeText[3],
	modeText[4], modeText[5], modeText[6], modeText[7], modeText[8]);

	memset(modeText, 0, sizeof(modeText));
	if(strcmp(shared_memory[p_num].serial.parity, "none")==0) strcpy(modeText[0], " selected=\"on\"");
	if(strcmp(shared_memory[p_num].serial.parity, "even")==0) strcpy(modeText[1], " selected=\"on\"");
	if(strcmp(shared_memory[p_num].serial.parity, "odd")==0)	strcpy(modeText[2], " selected=\"on\"");

	if((strlen(modeText[0])==0)&&(strlen(modeText[1])==0)&&(strlen(modeText[2])==0))
		strcpy(modeText[3], " style=\"color:red;\"");

	printf("\
<tr%s><td class=\"f\">\n\
<select name=\"parity\">\n\
<option value=\"0\"%s>NONE</option>\n\
<option value=\"1\"%s>EVEN</option>\n\
<option value=\"2\"%s>ODD</option>\n\
</select>\n\n\
<td style=\"text-align:left;\">�������� ��������\n\
<tr><td class=\"f\">\n\
<input type=\"text\" name=\"timeout\" value=\"%d\" maxlength=\"5\" style=\"width:64pt;\" />\n\
<td style=\"text-align:left;\">������� �����, ��\n\n",
	modeText[3], modeText[0], modeText[1], modeText[2],	shared_memory[p_num].serial.timeout/1000);

printf("\
<tr><td class=\"f\">\n\
<select name=\"data_bits\" disabled=\"on\">\n\
<option value=\"0\" selected=\"on\">8&nbsp;</option>\n\
<option value=\"1\">7&nbsp;</option>\n\
</select>\n\
<td style=\"text-align:left;\">���� ������\n\n\
<tr><td class=\"f\">\n\
<select name=\"stop_bits\" disabled=\"on\">\n\
<option value=\"0\" selected=\"on\">1</option>\n\
<option value=\"1\">1.5</option>\n\
<option value=\"2\">2</option>\n\
</select>\n\
<td style=\"text-align:left;\">�������� ����\n\n\
<tr><td class=\"f\">\n\
<select name=\"flow_control\" disabled=\"on\">\n\
<option value=\"0\">RTS/CTS</option>\n\
<option value=\"1\" selected=\"on\">XON/XOFF</option>\n\
</select>\n\
<td style=\"text-align:left;\">�������� ��������\n\n\
<tr><td colspan=\"2\">\n\
<input type=\"submit\" name=\"serial_port_settings_apply\" value=\"���������\">\n\
<input type=\"reset\" value=\"������\">\n\n\
</table>\n\
</form></div>\n");

return;
}
///-----------------------------------------------------------
void show_interface_clients(u8 P)
{

	if((shared_memory[P].modbus_mode!=GATEWAY_SIMPLE)&&(shared_memory[P].modbus_mode!=BRIDGE_SIMPLE)) return;

  char pstatus[128], pmode[32], row_class[32];
  int i;
  
	if(shared_memory[P].modbus_mode==BRIDGE_SIMPLE) {
	  strcpy(pmode, "�������");
	  strcpy(pstatus, "\n<th rowspan=\"2\">�����<br />modbus\n<th rowspan=\"2\">�����<br />������<br />������\n");
	  } else {
	  	strcpy(pmode, "�������");
	  	strcpy(pstatus, "\n");
	    }

printf("\
<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" align=\"center\" id=\"p_clients\">\n\
<caption>Modbus-TCP %s</caption>\n\
<tr>\n\
<th rowspan=\"2\">����� IP%s\
<th rowspan=\"2\">���������\n\
<th colspan=\"2\" class=\"internal\">����������\n\
<th rowspan=\"2\">��������<br />��������, ��\n\
<th rowspan=\"2\">������<br />������, ��\n\
<tr>\n\
<th>�������\n\
<th>������\n\
\n", pmode, pstatus);

for(i=0; i<MAX_TCP_CLIENTS_PER_PORT; i++) {

  if((
  	 (shared_memory[P].clients[i].connection_status!=MB_CONNECTION_ESTABLISHED)&&
  	 (shared_memory[P].modbus_mode!=BRIDGE_SIMPLE)	)||(
  	 (shared_memory[P].modbus_mode==BRIDGE_SIMPLE)&&
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
		if(shared_memory[P].modbus_mode==BRIDGE_SIMPLE)
		  printf("<td>&nbsp;\n<td>&nbsp;\n");

  	continue;
  	}

	int diff=	shared_memory[P].clients[i].connection_time==0?0:\
						difftime(moment, shared_memory[P].clients[i].connection_time);

	sprintf(pmode, "%3.3d.%3.3d.%3.3d.%3.3d",	shared_memory[P].clients[i].ip >> 24,
																(shared_memory[P].clients[i].ip >> 16) & 0xff,
																(shared_memory[P].clients[i].ip >> 8) & 0xff,
																shared_memory[P].clients[i].ip & 0xff);
																
	if(shared_memory[P].modbus_mode==BRIDGE_SIMPLE)
		sprintf(pstatus, "%d", shared_memory[P].clients[i].mb_slave);
		else strcpy(pstatus, "N/A");
		
	strcpy(row_class, "p_on");
	if((shared_memory[P].modbus_mode==BRIDGE_SIMPLE)&&
		 (shared_memory[P].clients[i].connection_status!=MB_CONNECTION_ESTABLISHED)) strcpy(row_class, "p_err");

printf("\
<tr class=\"%s\">\n\
<td style=\"text-align:left;\">%s:%d\n\
",	row_class, pmode, shared_memory[P].clients[i].port);
  		
	if(shared_memory[P].modbus_mode==BRIDGE_SIMPLE) {
	  if(shared_memory[P].clients[i].address_shift!=MB_ADDRESS_NO_SHIFT)
		  sprintf(pmode, "%+d", shared_memory[P].clients[i].address_shift);
		  else sprintf(pmode, "���");
		printf("\
<td>%s\n\
<td>%s\n\
",	pstatus, pmode);
	  }
			
  if(shared_memory[P].clients[i].stat.scan_rate<MB_SCAN_RATE_INFINITE)
    sprintf(pstatus, "%d", shared_memory[P].clients[i].stat.scan_rate);
    else sprintf(pstatus, "---");

printf("\
<td style=\"white-space:nowrap;\">%3.3d� %2.2d:%2.2d:%2.2d\n\
<td>%d\n\
<td>%d\n\
<td>%d\n\
<td>%s\n\
",	  (diff/86400)%1000, (diff/3600)%24, (diff/60)%60, diff%60,
  		shared_memory[P].clients[i].stat.sended,
  		shared_memory[P].clients[i].stat.errors,
  		shared_memory[P].clients[i].stat.request_time,
  		pstatus);
  		
	}

pmode[0]=shared_memory[P].modbus_mode==BRIDGE_SIMPLE?'4':'2';

printf("\
<tr><td colspan=\"%c\">&nbsp;<td colspan=\"2\" style=\"text-align:center;\">\n\
<form action=\"\" method=\"post\" name=\"port_clients\" style=\"padding:0; margin:0; border:none;\">\n\
<input type=\"hidden\" name=\"p_num\" value=\"1\">\n\
<input type=\"submit\" name=\"port_clients_reset_counters\" value=\"��������\">\n\
</form>\n\
<td colspan=\"2\">&nbsp;\n\
</table>\n\
\n", pmode[0]);

return;
}
///-----------------------------------------------------------
int process_input_data()
	{
	int i, contlen;
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
������ ��� ������� ���������� �����.\
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
			gate->halt=1;
			printf("Location: http://%s/index.shtml\r\n\r\n", srv);
//			printf("\
Content-Type: text/html\r\n\r\n\
<div class=\"err_block\">Modbus-���� ����������. ��������� �� �������</div>\n");
//printf("HELLO WORLD!");
			break;

		case FORM_ACTION_APPLY:

			f=0;
			for(i=0; i<MAX_FORM_FIELDS; i++)
				if(strlen(form_field[i])>0)
					if(strcmp(form_field[i], "backlight_enabled")==0) f=1;
			gate->back_light=f;

			f=0;
			for(i=0; i<MAX_FORM_FIELDS; i++)
				if(strlen(form_field[i])>0)
					if(strcmp(form_field[i], "buzzer_enabled")==0) f=1;
			gate->use_buzzer=f;

		default: printf("Location: http://%s/index.shtml?SETTINGS\r\n\r\n", srv);
		}

//printf("HELLO WORLD!!!");

	return 0;
	}
///-----------------------------------------------------------
