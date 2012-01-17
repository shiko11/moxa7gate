/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///**** МОДУЛЬ МОНИТОРИНГА И УПРАВЛЕНИЯ РАБОТОЙ ШЛЮЗА СРЕДСТВАМИ KEYPAD-LCM ****

///=== HMI_KLB_H IMPLEMENTATION

#include "hmi_klb.h"
#include "interfaces.h"
#include "moxagate.h"
#include "messages.h"

///=== HMI_KLB_H private variables

unsigned int i, j, k;
char marker[EVENT_MESSAGE_LENGTH+EVENT_MESSAGE_PREFIX];
struct tm *tmd;

//unsigned menu_actions_amount[LCM_NOT_IMPLEMENTED];
//char     menu_tpl           [LCM_NOT_IMPLEMENTED][LCM_MENU_MAX_ACTIONS][LCM_MENU_ACTDEFLEN];

///=== HMI_KLB_H private functions

/*//----------------------------------------------------------------------------
LCM_MAIN_IFRTU1
LCM_MAIN_IFRTU2

-----------------     -----------------
| RTU INTERFACES |    | RTU INTERFACES |
|----------------|    |----------------|
|SL Msgs Succ MOD|    |SL Msgs Succ MOD|
|P1>0123 4567 SRV|    |P5>0123 4567 INI|
|P2 8901 2345 MAS|    |P6 8901 2345 IDL|
|P3 6789 0123 SLV|    |P7 6789 0123 ERR|
|P4 4567 8901 OFF|    |P8 4567 8901 OFF|
|F2-PGDN  F4-PGUP|    |F2-PGDN  F4-PGUP|
-----------------     -----------------

*///----------------------------------------------------------------------------
void show_main_ifrtu1()
  {
  if(screen.main_scr_rtu>3) screen.main_scr_rtu = screen.main_scr_rtu%4;
    
  marker[0] = marker[1] = marker[2] = marker[3] = ' ';
  marker[screen.main_scr_rtu]='>';
  
	strcpy (screen.text[0], " RTU INTERFACES ");
	strcpy (screen.text[1], "----------------");
	strcpy (screen.text[2], "SL Msgs Succ MOD");
	sprintf(screen.text[3], "P1%c%4.4d %4.4d %s", marker[0],
                                                IfaceRTU[GATEWAY_P1].stat.accepted%10000,
                                                (IfaceRTU[GATEWAY_P1].stat.sended+
                                                 IfaceRTU[GATEWAY_P1].stat.frwd_p+
                                                 IfaceRTU[GATEWAY_P1].stat.frwd_a+
                                                 IfaceRTU[GATEWAY_P1].stat.frwd_r)%10000,
                                                IfaceRTU[GATEWAY_P1].bridge_status);
	sprintf(screen.text[4], "P2%c%4.4d %4.4d %s", marker[1],
                                                IfaceRTU[GATEWAY_P2].stat.accepted%10000,
                                                (IfaceRTU[GATEWAY_P2].stat.sended+
                                                 IfaceRTU[GATEWAY_P2].stat.frwd_p+
                                                 IfaceRTU[GATEWAY_P2].stat.frwd_a+
                                                 IfaceRTU[GATEWAY_P2].stat.frwd_r)%10000,
                                                IfaceRTU[GATEWAY_P2].bridge_status);
	sprintf(screen.text[5], "P3%c%4.4d %4.4d %s", marker[2],
                                                IfaceRTU[GATEWAY_P3].stat.accepted%10000,
                                                (IfaceRTU[GATEWAY_P3].stat.sended+
                                                 IfaceRTU[GATEWAY_P3].stat.frwd_p+
                                                 IfaceRTU[GATEWAY_P3].stat.frwd_a+
                                                 IfaceRTU[GATEWAY_P3].stat.frwd_r)%10000,
                                                IfaceRTU[GATEWAY_P3].bridge_status);
	sprintf(screen.text[6], "P4%c%4.4d %4.4d %s", marker[3],
                                                IfaceRTU[GATEWAY_P4].stat.accepted%10000,
                                                (IfaceRTU[GATEWAY_P4].stat.sended+
                                                 IfaceRTU[GATEWAY_P4].stat.frwd_p+
                                                 IfaceRTU[GATEWAY_P4].stat.frwd_a+
                                                 IfaceRTU[GATEWAY_P4].stat.frwd_r)%10000,
                                                IfaceRTU[GATEWAY_P4].bridge_status);
	strcpy (screen.text[7], "F2-PGDN  F4-PGUP");

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }

void show_main_ifrtu2()
  {
  if((screen.main_scr_rtu<4)||(screen.main_scr_rtu>7))
    screen.main_scr_rtu = screen.main_scr_rtu%4+4;
    
  marker[0] =  marker[1] = marker[2] = marker[3] = ' ';
  marker[screen.main_scr_rtu-4]='>';
  	
	strcpy (screen.text[0], " RTU INTERFACES ");
	strcpy (screen.text[1], "----------------");
	strcpy (screen.text[2], "SL Msgs Succ MOD");
	sprintf(screen.text[3], "P5%c%4.4d %4.4d %s", marker[0],
                                                IfaceRTU[GATEWAY_P5].stat.accepted%10000,
                                                (IfaceRTU[GATEWAY_P5].stat.sended+
                                                 IfaceRTU[GATEWAY_P5].stat.frwd_p+
                                                 IfaceRTU[GATEWAY_P5].stat.frwd_a+
                                                 IfaceRTU[GATEWAY_P5].stat.frwd_r)%10000,
                                                IfaceRTU[GATEWAY_P5].bridge_status);
	sprintf(screen.text[4], "P6%c%4.4d %4.4d %s", marker[1],
                                                IfaceRTU[GATEWAY_P6].stat.accepted%10000,
                                                (IfaceRTU[GATEWAY_P6].stat.sended+
                                                 IfaceRTU[GATEWAY_P6].stat.frwd_p+
                                                 IfaceRTU[GATEWAY_P6].stat.frwd_a+
                                                 IfaceRTU[GATEWAY_P6].stat.frwd_r)%10000,
                                                IfaceRTU[GATEWAY_P6].bridge_status);
	sprintf(screen.text[5], "P7%c%4.4d %4.4d %s", marker[2],
                                                IfaceRTU[GATEWAY_P7].stat.accepted%10000,
                                                (IfaceRTU[GATEWAY_P7].stat.sended+
                                                 IfaceRTU[GATEWAY_P7].stat.frwd_p+
                                                 IfaceRTU[GATEWAY_P7].stat.frwd_a+
                                                 IfaceRTU[GATEWAY_P7].stat.frwd_r)%10000,
                                                IfaceRTU[GATEWAY_P7].bridge_status);
	sprintf(screen.text[6], "P8%c%4.4d %4.4d %s", marker[3],
                                                IfaceRTU[GATEWAY_P8].stat.accepted%10000,
                                                (IfaceRTU[GATEWAY_P8].stat.sended+
                                                 IfaceRTU[GATEWAY_P8].stat.frwd_p+
                                                 IfaceRTU[GATEWAY_P8].stat.frwd_a+
                                                 IfaceRTU[GATEWAY_P8].stat.frwd_r)%10000,
                                                IfaceRTU[GATEWAY_P8].bridge_status);
	strcpy (screen.text[7], "F2-PGDN  F4-PGUP");

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }
/*//----------------------------------------------------------------------------
LCM_MAIN_IFTCP

-----------------
| TCP INTERFACES |
|----------------|
|EL Msgs Succ MOD|
|01>0123 4567  OK|
|11 8901 2345 ERR|
|32 4567 8901 MRG|
|                |
|F2-PGDN  F4-PGUP|
-----------------

*///----------------------------------------------------------------------------
void show_main_iftcp()
  {
	strcpy (screen.text[0], " TCP INTERFACES ");
	strcpy (screen.text[1], "----------------");
	strcpy (screen.text[2], "EL Msgs Succ MOD");
	strcpy (screen.text[3], "                ");
	strcpy (screen.text[4], "                ");
	strcpy (screen.text[5], "                ");
	strcpy (screen.text[6], "                ");
	strcpy (screen.text[7], "F2-PGDN  F4-PGUP");
																												
  if(Security.TCPIndex[MAX_TCP_SERVERS]==0) {
	  strcpy (screen.text[4], "THERE ARE NO ONE");
	  strcpy (screen.text[5], "CONFIGURED IFACE");

    mxlcm_write_screen(mxlcm_handle, screen.text);
    return;
    }

  if(screen.main_tcp_current >= Security.TCPIndex[MAX_TCP_SERVERS])
     screen.main_tcp_current %= Security.TCPIndex[MAX_TCP_SERVERS];

  if(screen.main_tcp_current < screen.main_tcp_start)
    screen.main_tcp_start = screen.main_tcp_current;
  else if(screen.main_tcp_current - screen.main_tcp_start > 3)
    screen.main_tcp_start = screen.main_tcp_current - 3;

  for(i=screen.main_tcp_start; i < screen.main_tcp_start+4; i++) {

    if(Security.TCPIndex[i]==GATEWAY_NONE) continue;

    if(i == screen.main_tcp_current)
           sprintf(marker, "%2.2d>", Security.TCPIndex[i]+1);
      else sprintf(marker, "%2.2d ", Security.TCPIndex[i]+1);

  
	  sprintf(screen.text[i-screen.main_tcp_start+3], "%s%4.4d %4.4d %s", marker,
                                                  IfaceTCP[Security.TCPIndex[i]].stat.accepted%10000,
	                                                (IfaceTCP[Security.TCPIndex[i]].stat.sended+
	                                                 IfaceTCP[Security.TCPIndex[i]].stat.frwd_p+
	                                                 IfaceTCP[Security.TCPIndex[i]].stat.frwd_a+
	                                                 IfaceTCP[Security.TCPIndex[i]].stat.frwd_r)%10000,
                                                  IfaceTCP[Security.TCPIndex[i]].bridge_status);
  }
	 
  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }
/*//----------------------------------------------------------------------------

-----------------
| GATEWAY DEVICE |
|----------------|
|moxa_ms689_90a4 |
|conf ver. 12.000|
|03.12.2011 14:32|
| 010.000.006.037|
|tcp port     502|
|F2-PGDN  F4-PGUP|
-----------------

LCM_MAIN_MOXAGATE
*///--------------
void show_main_moxagate()
  {
  char c, tmp[MAX_LCM_COLS];
  int i;
  	
	strcpy(screen.text[0], " GATEWAY DEVICE ");
	strcpy(screen.text[1], "----------------");

  c=Security.NetworkName[MAX_LCM_COLS];
  Security.NetworkName[MAX_LCM_COLS]=0;
	strcpy(screen.text[2], Security.NetworkName);
  Security.NetworkName[MAX_LCM_COLS]=c;

	strcpy(screen.text[3], "conf ver.       ");
	i=strlen(Security.VersionNumber);
	if(i<8) strcpy(&screen.text[3][MAX_LCM_COLS-i], Security.VersionNumber);

  c=Security.VersionTime[MAX_LCM_COLS];
  Security.VersionTime[MAX_LCM_COLS]=0;
	strcpy(screen.text[4], Security.VersionTime);
  Security.VersionTime[MAX_LCM_COLS]=c;

	sprintf(screen.text[5], " %3.3d.%3.3d.%3.3d.%3.3d",
		(Security.LAN1Address >> 24) & 0xff,
		(Security.LAN1Address >> 16) & 0xff,
		(Security.LAN1Address >>  8) & 0xff,
		 Security.LAN1Address        & 0xff
		);

	strcpy(screen.text[6], "tcp port        ");
	sprintf(tmp, "%d", Security.tcp_port);
	i=strlen(tmp);
  if(i<9) strcpy(&screen.text[6][MAX_LCM_COLS-i], tmp);

	strcpy(screen.text[7], "F2-PGDN  F4-PGUP");

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }
/*//----------------------------------------------------------------------------
LCM_MAIN_EVENTLOG
-----------------
|EVENT LOG [9999]|
|----------------|
| ALL Events 9999|
|>INF Events 8888|
| WRN Events   99|
| ERR Events    0|
|                |
|F2-PGDN  F4-PGUP|
-----------------

*///---------------------------------------------------------------
void show_main_eventlog()
  {
  if(screen.main_scr_eventlog >= 4)
     screen.main_scr_eventlog %= 4;
    
  marker[0]=' '; marker[1]=' '; marker[2]=' '; marker[3]=' ';
  marker[screen.main_scr_eventlog]='>';
  
	sprintf(screen.text[0], "EVENT LOG [%4.4d]", EVENT_LOG_LENGTH);
	strcpy (screen.text[1], "----------------");
	sprintf(screen.text[2], "%cALL Events %4.4d", marker[0],
                                                EventLog.app_log_entries_total);
	sprintf(screen.text[3], "%cINF Events %4.4d", marker[1],
                                                EventLog.type_msgs_amount[EVENT_TYPE_ORD(EVENT_TYPE_INF)]);
	sprintf(screen.text[4], "%cWRN Events %4.4d", marker[2],
                                                EventLog.type_msgs_amount[EVENT_TYPE_ORD(EVENT_TYPE_WRN)]);
	sprintf(screen.text[5], "%cERR Events %4.4d", marker[3],
                                                EventLog.type_msgs_amount[EVENT_TYPE_ORD(EVENT_TYPE_ERR)]);
//  if(Security.show_sys_messages)
//	       sprintf(screen.text[6], "Debug Events  On");
//    else sprintf(screen.text[6], "Debug Events Off");
  strcpy (screen.text[6], "                ");
	strcpy (screen.text[7], "F2-PGDN  F4-PGUP");

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }

/*//----------------------------------------------------------------------------
-----------------
|LOG EVENTS <ALL>|
|1234 INF   PORT1|
|03 DEC  14:32:00|
|- COMMAND LINE P|
|ARSED SUCCESSFUL|
|LY              |
|                |
|F2-PREV  F4-NEXT|
-----------------
F1-HELP (Контекстная справка)
F3-BACK
F5-MENU (Порядок сортировки, возрастание, убывание, форма фильтра по источникам)
*///----------------------------------------------------------------------------
void show_log_event()
  {
	// текущая реализация функции отображает только самое последнее сообщение

  strcpy(screen.text[4], "                ");
  strcpy(screen.text[5], "                ");
  strcpy(screen.text[6], "                ");

  if(EventLog.app_log_entries_total==0) {
    strcpy(screen.text[0], "LOG EVENTS <ALL>");
    strcpy(screen.text[1], "                ");
    strcpy(screen.text[2], "                ");
    strcpy(screen.text[3], " EVENT LOG HAVE ");
    strcpy(screen.text[4], "   NO ENTRIES   ");
    strcpy(screen.text[7], "F3-BACK         ");

    mxlcm_write_screen(mxlcm_handle, screen.text);
    return;
    }
  
	strcpy (screen.text[0], "LOG EVENTS <ALL>");

  k = screen.eventlog_current;

  get_msgtype_str(EventLog.app_log[k].msgtype, marker);
  get_msgsrc_str( EventLog.app_log[k].msgtype, EventLog.app_log[k].prm[0], &marker[8]);
	sprintf(screen.text[1], "%3.3d %s %s", k+1, marker, &marker[8]);

	tmd=gmtime(&EventLog.app_log[k].time);
  strftime(marker, 16, " %b", tmd);
	sprintf(screen.text[2], "%2.2d%s  %2.2d:%2.2d:%2.2d", tmd->tm_mday,
                                       marker,
                                       tmd->tm_hour,
                                       tmd->tm_min,
                                       tmd->tm_sec);

	marker[0]=0;
	make_msgstr(EventLog.app_log[k].msgcode, marker, EventLog.app_log[k].prm[0],
                                                   EventLog.app_log[k].prm[1],
                                                   EventLog.app_log[k].prm[2],
                                                   EventLog.app_log[k].prm[3]);
  j=strlen(marker);
  for(i=0; i<j; i++) marker[i]=tolower(marker[i]);

  i=marker[14];
  marker[14]=0;
	sprintf(screen.text[3], "- %s", &marker[0]);
  screen.text[3][0]=26;
  marker[14]=i;

  if(j>14) {
    i=marker[30];
    marker[30]=0;
  	sprintf(screen.text[4], "%s", &marker[14]);
    marker[30]=i;
    }

  if(j>30) {
    i=marker[46];
    marker[46]=0;
  	sprintf(screen.text[5], "%s", &marker[30]);
    marker[46]=i;
    }

  if(j>46) {
    i=marker[62];
    marker[62]=0;
  	sprintf(screen.text[6], "%s", &marker[46]);
    marker[62]=i;
    }

	strcpy (screen.text[7], "F2-PREV  F4-NEXT");

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }
/*//----------------------------------------------------------------------------
LCM_MAIN_HELP						 
IFACES                MOXADEVICE            EVENT LOG             SYSINFO
-----------------     -----------------     -----------------     -----------------     -----------------
|  CONTEXT HELP  |    |  CONTEXT HELP  |    |  CONTEXT HELP  |    |  CONTEXT HELP  |    |  CONTEXT HELP  |
|                |    |                |    |                |    |                |    |                |
|F2-Page down    |    |F2-Page down    |    |F2-Page down    |    |This is last pa-|    |F2-Move up      |
|F3-Enter detail |    |F3-Enter detail |    |F3-Goto selected|    |ge of main level|    |F3-Select action|
|statistic screen|    |statistic screen|    |   events detail|    |F2-Home page    |    |F4-Move down    |
|F4-Move cursor  |    |F5-Menu         |    |F4-Move cursor  |    |F5-Menu         |    |F5-Go back      |
|F5-Menu         |    |                |    |F5-Menu         |    |                |    |                |
|         F1-BACK|    |         F1-BACK|    |         F1-BACK|    |         F1-BACK|    |         F1-BACK|
-----------------     -----------------     -----------------     -----------------     -----------------

*///----------------------------------------------------------------------------
void show_main_help()
  {
	strcpy(screen.text[0], "  CONTEXT HELP  ");
	strcpy(screen.text[1], "                ");
  strcpy(screen.text[2], "                ");
  strcpy(screen.text[3], "                ");
  strcpy(screen.text[4], "                ");
  strcpy(screen.text[5], "                ");
  strcpy(screen.text[6], "                ");
	strcpy(screen.text[7], "F1-BACK F3-ABOUT");

  switch(screen.prev_screen) {

    case LCM_MAIN_IFRTU1:
    case LCM_MAIN_IFRTU2:
    case LCM_MAIN_IFTCP:
    strcpy(screen.text[2], "F2-Page down    ");
    strcpy(screen.text[3], "F3-Statistics   ");
    strcpy(screen.text[4], "F4-Page up      ");
    strcpy(screen.text[5], "F5-Move cursor  ");
    break;

    case LCM_MAIN_MOXAGATE:
    strcpy(screen.text[2], "F2-Page down    ");
    strcpy(screen.text[3], "F3-Statistics   ");
    strcpy(screen.text[4], "F4-Page up      ");
    strcpy(screen.text[5], "F5-Stop gateway ");
    //strcpy(screen.text[6], "");
    break;

    case LCM_MAIN_EVENTLOG:
    strcpy(screen.text[2], "F2-Page down    ");
    strcpy(screen.text[3], "F3-Goto eventlog");
    strcpy(screen.text[4], "F4-Page up      ");
    strcpy(screen.text[5], "F5-Move cursor  ");
    break;

/*    case LCM_MAIN_SYSINFO:
    strcpy(screen.text[2], "F2-Page down    ");
    strcpy(screen.text[3], "F5-Menu         ");
    //strcpy(screen.text[6], "");
    break;

    case LCM_MAIN_SETTINGS:
    strcpy(screen.text[2], "F2-Home         ");
    strcpy(screen.text[3], "F3-Enter        ");
    strcpy(screen.text[4], "F4-Move cursor  ");
    strcpy(screen.text[5], "F5-Menu         ");
    break;

	*/
    case LCM_STAT_PORT1:
    case LCM_STAT_PORT2:
    case LCM_STAT_PORT3:
    case LCM_STAT_PORT4:
    case LCM_STAT_PORT5:
    case LCM_STAT_PORT6:
    case LCM_STAT_PORT7:
    case LCM_STAT_PORT8:
    strcpy(screen.text[2], "F2-GO BACK      ");
    strcpy(screen.text[3], "F3-RTU SETTINGS ");
    strcpy(screen.text[4], "F4-RESET COUNTER");
    break;

    default:;
    }

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }
/*//----------------------------------------------------------------------------
LCM_MAIN_ABOUT
-----------------
| MOXA7GATE V1.2 |
| Modbus gateway |
|      software  |
|SEM-ENGINEERING |
|+7(4832)41-88-23|
| www.semgroup.ru|
|  Bryansk 2010  |
|         F5-BACK|
-----------------

*///----------------------------------------------------------------------------
void show_about_screen()
  {

	strcpy(screen.text[0], " MOXA7GATE V1.2 ");
	strcpy(screen.text[1], " Modbus gateway ");
	strcpy(screen.text[2], "      software  ");
	strcpy(screen.text[3], "SEM-ENGINEERING ");
	strcpy(screen.text[4], "+7(4832)41-88-23");
	strcpy(screen.text[5], " www.semgroup.ru");
	strcpy(screen.text[6], "  Bryansk 2010  ");
	strcpy(screen.text[7], "         F3-BACK");

  if(screen.prev_screen==LCM_MAIN_ABOUT)
	  strcpy(screen.text[7], "                ");

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }

/*
PORT1-------------
|IFACE RTU P1 00G|
|----------------|
|mode     RS4852W|
|speed       9600|
|parity      none|
|timeout, ms 2000|
|tcp server   502|
|         F2-BACK|
------------------
LCM_SETT_PORT1
LCM_SETT_PORT2
LCM_SETT_PORT3
LCM_SETT_PORT4
LCM_SETT_PORT5
LCM_SETT_PORT6
LCM_SETT_PORT7
LCM_SETT_PORT8
*///---------------------------------------------------------------
void show_uart_settings(int uart)
  {
  
  char tmp[MAX_LCM_COLS];
  int i;

	sprintf(screen.text[0], "IFACE RTU P%d %s", uart+1, IfaceRTU[uart].bridge_status);
	strcpy (screen.text[1], "----------------");
	sprintf(screen.text[2], "mode            ");
	sprintf(screen.text[3], "speed           ");
	sprintf(screen.text[4], "parity          ");
	sprintf(screen.text[5], "timeout, ms     ");
	sprintf(screen.text[6], "                ");
	strcpy (screen.text[7], "         F2-BACK");

  if(IfaceRTU[uart].modbus_mode!=IFACE_OFF) {

	  i=strlen(IfaceRTU[uart].serial.p_mode);
	  if(i<MAX_LCM_COLS) strcpy(&screen.text[2][MAX_LCM_COLS-i], IfaceRTU[uart].serial.p_mode);
	  if(screen.text[2][13]=='_') screen.text[2][13]=' ';
	
	  i=strlen(IfaceRTU[uart].serial.speed);
	  if(i<MAX_LCM_COLS) strcpy(&screen.text[3][MAX_LCM_COLS-i], IfaceRTU[uart].serial.speed);
	  screen.text[3][0]='s';
	
	  i=strlen(IfaceRTU[uart].serial.parity);
	  if(i<MAX_LCM_COLS) strcpy(&screen.text[4][MAX_LCM_COLS-i], IfaceRTU[uart].serial.parity);
	  screen.text[4][0]='p';
	
		sprintf(tmp, "%d", IfaceRTU[uart].serial.timeout/1000);
	  i=strlen(tmp);
	  if(i<MAX_LCM_COLS) strcpy(&screen.text[5][MAX_LCM_COLS-i], tmp);
	  screen.text[5][0]='t';
	
		sprintf(tmp, "%d", IfaceRTU[uart].Security.tcp_port);
	  i=strlen(tmp);
		switch(IfaceRTU[uart].modbus_mode) {
			case IFACE_TCPSERVER: strcpy(screen.text[6], "tcp server      ");
			  if(i<MAX_LCM_COLS) strcpy(&screen.text[6][MAX_LCM_COLS-i], tmp);
			  break;
			case IFACE_RTUMASTER: strcpy(screen.text[6], "rtu master      "); break;
			case IFACE_RTUSLAVE:  strcpy(screen.text[6], "rtu slave       "); break;
		  }
    }

	screen.text[7][0]=' ';

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }
/*//----------------------------------------------------------------------------
DETAIL------------
|IFACE RTU P1 00G|
|----------------|
|sended      0123|
|timeouts    0123|
|crc errors  0123|
|errors      0123|
|dalay, ms   0123|
|F2-BACK  F3-SETT|
------------------
LCM_STAT_PORT1
LCM_STAT_PORT2
LCM_STAT_PORT3
LCM_STAT_PORT4
LCM_STAT_PORT5
LCM_STAT_PORT6
LCM_STAT_PORT7
LCM_STAT_PORT8
*///---------------------------------------------------------------
void show_ifrtu_statistics(int uart)
  {
	sprintf(screen.text[0], "IFACE RTU P%d %s", uart+1, IfaceRTU[uart].bridge_status);
	strcpy (screen.text[1], "----------------");
	sprintf(screen.text[2], "sended      %4.4d", IfaceRTU[uart].stat.sended%10000);
	sprintf(screen.text[3], "timeouts    %4.4d", IfaceRTU[uart].stat.rtu_rsp_timeout%10000);
	sprintf(screen.text[4], "crc errors  %4.4d", (IfaceRTU[uart].stat.rtu_req_crc + IfaceRTU[uart].stat.rtu_rsp_crc)%10000);
	sprintf(screen.text[5], "errors      %4.4d", IfaceRTU[uart].stat.errors%10000);
	sprintf(screen.text[6], "dalay, ms   %4.4d", IfaceRTU[uart].stat.proc_time);
	strcpy (screen.text[7], "F2-BACK  F3-SETT");

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }
/*//----------------------------------------------------------------------------
-----------------
| MODBUS GATEWAY |
|                |
|  ARE YOU SURE  |
|  TO RESET ALL  |
|    COUNTERS?   |
|                |
| F2-NO   F4-YES |
|                |
-----------------
LCM_CONFIRM_RESET
*///----------------------------------------------------------------------------
void show_confirm_reset_counters()
  {
	strcpy(screen.text[0], " MODBUS GATEWAY ");
	strcpy(screen.text[1], "                ");
	strcpy(screen.text[2], "  ARE YOU SURE  ");
	strcpy(screen.text[3], "  TO RESET ALL  ");
	strcpy(screen.text[4], "    COUNTERS?   ");
	strcpy(screen.text[5], "                ");
	strcpy(screen.text[6], " F2-NO   F4-YES ");
	strcpy(screen.text[7], "                ");

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }

/*//----------------------------------------------------------------------------
CONFIRMATION------
| MODBUS GATEWAY |
|                |
|  ARE YOU SURE  |
|  TO SHUTDOWN?  |
|                |
|                |
| F2-NO   F4-YES |
|                |
------------------
LCM_CONFIRM_HALT
*///---------------------------------------------------------------
void show_confirm_halt()
  {
	strcpy(screen.text[0], " MODBUS GATEWAY ");
	strcpy(screen.text[1], "                ");
	strcpy(screen.text[2], "  ARE YOU SURE  ");
	strcpy(screen.text[3], "  TO SHUTDOWN?  ");
	strcpy(screen.text[4], "                ");
	strcpy(screen.text[5], "                ");
	strcpy(screen.text[6], " F2-NO   F4-YES ");
	strcpy(screen.text[7], "                ");

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }
/*//----------------------------------------------------------------------------
LCM_STAT_MOXAGATE
-----------------
|GATEWAY CLIENTS |
|----------------|
|active        23|
|accepted con 023|
|rejected con 001|
|tcp server   008|
|tcp master   012|
|F2-BACK  F3-SETT|
-----------------

*///----------------------------------------------------------------------------
void show_stat_moxagate()
  {
  int i, tcpsrv=0, master=0;

  for(i=0; i<MOXAGATE_CLIENTS_NUMBER; i++) {
    if(Client[i].status==GW_CLIENT_TCP_GWS) tcpsrv++;
    if(Client[i].status==GW_CLIENT_TCP_502) master++;
    }
  	
	strcpy (screen.text[0], "GATEWAY CLIENTS ");
	strcpy (screen.text[1], "----------------");
	sprintf(screen.text[2], "active        %2.2d", Security.current_connections_number);
	sprintf(screen.text[3], "accepted con %3.3d", Security.accepted_connections_number);
	sprintf(screen.text[4], "rejected con %3.3d", Security.rejected_connections_number);
	sprintf(screen.text[5], "tcp server   %3.3d", tcpsrv);
	sprintf(screen.text[6], "tcp master   %3.3d", master);
	strcpy (screen.text[7], "F2-BACK  F3-SETT");

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }
/*//----------------------------------------------------------------------------

-----------------
| GATEWAY DEVICE |
|----------------|
|modbus addr  247|
|status reg 65535|
|watchdog     off|
|map 2x:4x    off|
|uptime    000.0d|
|         F2-BACK|
-----------------

LCM_SETT_MOXAGATE
*///--------------
void show_sett_moxagate()
  {
  char tmp[MAX_LCM_COLS];
  int i;
  	
	strcpy(screen.text[0], " GATEWAY DEVICE ");
	strcpy(screen.text[1], "----------------");

	sprintf(screen.text[2], "modbus addr  %3.3d", MoxaDevice.modbus_address);

	strcpy(screen.text[3], "status reg      ");
	sprintf(tmp, "%d", MoxaDevice.status_info+1);
	i=strlen(tmp);
  if(i<6) strcpy(&screen.text[3][MAX_LCM_COLS-i], tmp);

  if(Security.watchdog_timer!=1) strcpy(screen.text[4], "watchdog     off");
    else strcpy(screen.text[4], "watchdog      on");

  if(MoxaDevice.map2Xto4X!=1) strcpy(screen.text[5], "map 2x:4x    off");
    else strcpy(screen.text[5], "map 2x:4x    on");

  time_t moment;
  time(&moment);
  int diff=difftime(moment, MoxaDevice.start_time);
	sprintf(screen.text[6], "uptime   %3.2fd", diff/86400);

	strcpy(screen.text[7], "         F2-BACK");

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }

/*//----------------------------------------------------------------------------
-----------------
|IFACE TCP 32 MRG|
|----------------|
|sended      0123|
|timeouts    0123|
|errors      0123|
|dalay, ms   0123|
|active conn    1|
|F2-BACK  F3-SETT|
-----------------
LCM_STAT_LANTCP
*///---------------------------------------------------------------
void show_iftcp_statistics()
  {
  int tcp;
  tcp=Security.TCPIndex[screen.main_tcp_current];
  	
	sprintf(screen.text[0], "IFACE TCP %2.2d %s", tcp+1, IfaceTCP[tcp].bridge_status);
	strcpy (screen.text[1], "----------------");
	sprintf(screen.text[2], "sended      %4.4d", IfaceTCP[tcp].stat.sended%10000);
	sprintf(screen.text[3], "timeouts    %4.4d", IfaceTCP[tcp].stat.tcp_rsp_timeout%10000);
	sprintf(screen.text[4], "errors      %4.4d", IfaceTCP[tcp].stat.errors%10000);
	sprintf(screen.text[5], "dalay, ms   %4.4d", IfaceTCP[tcp].stat.proc_time);
	sprintf(screen.text[6], "active conn    %d", IfaceTCP[tcp].ethernet.active_connection);
	strcpy (screen.text[7], "F2-BACK  F3-SETT");

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }
/*//---------------------------------------------------------------
-----------------
|IFACE TCP 01 MRG|
|----------------|
|modbus addr   52|
|server1 tcp  502|
| 192.168.005.252|
|server2 tcp  502|
| 192.168.006.252|
|         F2-BACK|
-----------------
LCM_SETT_LANTCP
*///---------------------------------------------------------------
void show_lantcp_settings()
  {
  int i, tcp;
  char tmp[9];

  tcp=Security.TCPIndex[screen.main_tcp_current];
  
	sprintf(screen.text[0], "IFACE TCP %2.2d %s", tcp+1, IfaceTCP[tcp].bridge_status);
	strcpy (screen.text[1], "----------------");
	sprintf(screen.text[2], "modbus addr  %3.3d", IfaceTCP[tcp].ethernet.unit_id);

	strcpy(screen.text[3], "server1 tcp     ");
	sprintf(tmp, "%d", IfaceTCP[tcp].ethernet.port);
	i=strlen(tmp);
  if(i<9) strcpy(&screen.text[3][MAX_LCM_COLS-i], tmp);

	sprintf(screen.text[4], " %3.3d.%3.3d.%3.3d.%3.3d",
		(IfaceTCP[tcp].ethernet.ip >> 24) & 0xff,
		(IfaceTCP[tcp].ethernet.ip >> 16) & 0xff,
		(IfaceTCP[tcp].ethernet.ip >>  8) & 0xff,
		 IfaceTCP[tcp].ethernet.ip        & 0xff
		);

	strcpy(screen.text[5], "server2 tcp     ");
	sprintf(tmp, "%d", IfaceTCP[tcp].ethernet.port2);
	i=strlen(tmp);
  if(i<9) strcpy(&screen.text[5][MAX_LCM_COLS-i], tmp);

	sprintf(screen.text[6], " %3.3d.%3.3d.%3.3d.%3.3d",
		(IfaceTCP[tcp].ethernet.ip2 >> 24) & 0xff,
		(IfaceTCP[tcp].ethernet.ip2 >> 16) & 0xff,
		(IfaceTCP[tcp].ethernet.ip2 >>  8) & 0xff,
		 IfaceTCP[tcp].ethernet.ip2        & 0xff
		);

	strcpy (screen.text[7], "         F2-BACK");

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }

/*//----------------------------------------------------------------------------
LCM_MAIN_SYSINFO
-----------------
|  MOXA UC-7410  |
|----------------|
|057d 11h 35m 01s|
|                |
|CPU        1.25%|
|RAM          20%|
|FLASH        18%|
|F2-PGDN  F5-MENU|
-----------------

///----------------------------------------------------------------------------
void show_system_info()
  {
	time_t moment;

	time(&moment);
	int diff=difftime(moment, Security.start_time);
	
	strcpy (screen.text[0], "  MOXA UC-7410  ");
	strcpy (screen.text[1], "----------------");
	sprintf(screen.text[2], "%3.3dd %2.2dh %2.2dm %2.2ds", (diff/86400)%1000,
                                                         (diff/3600)%24,
                                                         (diff/60)%60,
                                                          diff%60);
	strcpy (screen.text[3], "                ");
	strcpy (screen.text[4], "CPU          --%");
	strcpy (screen.text[5], "RAM          --%");
	strcpy (screen.text[6], "FLASH        --%");
	strcpy (screen.text[7], "F2-PGDN  F5-MENU");

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }
*/
/*//----------------------------------------------------------------------------
LCM_MAIN_SETTINGS
-----------------
|GATEWAY SETTINGS|
|----------------|
| >Ifaces RTU    |
|  Ifaces TCP    |
|  Moxa Gateway  |
|  Security      |
|  Network       |
|F2-HOME  F4-DOWN|
-----------------

///----------------------------------------------------------------------------
void show_main_settings()
  {
  if(screen.main_scr_sett >= 5)
     screen.main_scr_sett %= 5;
    
  marker[0] = marker[1] = marker[2] = marker[3] = marker[4] = ' ';
  marker[screen.main_scr_sett]='>';
  

	strcpy (screen.text[0], "GATEWAY SETTINGS");
	strcpy (screen.text[1], "----------------");
	sprintf(screen.text[2], " %cIfaces RTU    ", marker[0]);
	sprintf(screen.text[3], " %cIfaces TCP    ", marker[1]);
	sprintf(screen.text[4], " %cMoxa Gateway  ", marker[2]);
	sprintf(screen.text[5], " %cSecurity      ", marker[3]);
	sprintf(screen.text[6], " %cNetwork       ", marker[4]);
	strcpy (screen.text[7], "F2-HOME  F4-DOWN");

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }
*//*/----------------------------------------------------------------------------
LAN SETTINGS------
| MODBUS GATEWAY |
|LAN1 IP config: |
| 010.000.006.037|
| 255.255.240.000|
|LAN2 IP config: |
| 192.168.001.127|
| 255.255.255.000|
|         F2-BACK|
------------------
LCM_SCREEN_LAN
///---------------------------------------------------------------
void show_inet_settings()
  {
	strcpy(screen.text[0], " MODBUS GATEWAY ");
	strcpy(screen.text[1], "LAN1 IP config: ");
	strcpy(screen.text[2], " 000.000.000.000");
	strcpy(screen.text[3], " 255.255.255.255");
	strcpy(screen.text[4], "LAN2 IP config: ");
	strcpy(screen.text[5], " 000.000.000.000");
	strcpy(screen.text[6], " 255.255.255.255");
	strcpy(screen.text[7], "         F2-BACK");

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }
*///----------------------------------------------------------------------------
