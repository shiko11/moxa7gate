/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///**** ÌÎÄÓËÜ ÌÎÍÈÒÎÐÈÍÃÀ È ÓÏÐÀÂËÅÍÈß ÐÀÁÎÒÎÉ ØËÞÇÀ ÑÐÅÄÑÒÂÀÌÈ KEYPAD-LCM ****

///=== HMI_KLB_H IMPLEMENTATION

#include "hmi_klb.h"
#include "interfaces.h"
#include "moxagate.h"
#include "messages.h"

///=== HMI_KLB_H private variables

unsigned int i, j, k;
char marker[8];

unsigned menu_actions_amount[LCM_NOT_IMPLEMENTED];
char     menu_tpl           [LCM_NOT_IMPLEMENTED][LCM_MENU_MAX_ACTIONS][LCM_MENU_ACTDEFLEN];

///=== HMI_KLB_H private functions

void init_menu_tpl();

///---------------------------------------------------------------
void init_menu_tpl()
  {
  memset(menu_actions_amount, 0, sizeof(menu_actions_amount));
  memset(menu_tpl,            0, sizeof(menu_tpl));

  menu_actions_amount[LCM_MAIN_IFRTU1]=5;
  strcpy(menu_tpl[LCM_MAIN_IFRTU1][0], "page up  ");
  strcpy(menu_tpl[LCM_MAIN_IFRTU1][1], "home     ");
  strcpy(menu_tpl[LCM_MAIN_IFRTU1][2], "stat detl");
  strcpy(menu_tpl[LCM_MAIN_IFRTU1][3], "about    ");
  strcpy(menu_tpl[LCM_MAIN_IFRTU1][4], "settings ");

  menu_actions_amount[LCM_MAIN_IFRTU2]=5;
  strcpy(menu_tpl[LCM_MAIN_IFRTU2][0], "page up  ");
  strcpy(menu_tpl[LCM_MAIN_IFRTU2][1], "home     ");
  strcpy(menu_tpl[LCM_MAIN_IFRTU2][2], "stat detl");
  strcpy(menu_tpl[LCM_MAIN_IFRTU2][3], "about    ");
  strcpy(menu_tpl[LCM_MAIN_IFRTU2][4], "settings ");

  menu_actions_amount[LCM_MAIN_IFTCP]=5;
  strcpy(menu_tpl[LCM_MAIN_IFTCP][0], "page up  ");
  strcpy(menu_tpl[LCM_MAIN_IFTCP][1], "home     ");
  strcpy(menu_tpl[LCM_MAIN_IFTCP][2], "stat detl");
  strcpy(menu_tpl[LCM_MAIN_IFTCP][3], "about    ");
  strcpy(menu_tpl[LCM_MAIN_IFTCP][4], "settings ");

  menu_actions_amount[LCM_MAIN_MOXAGATE]=6;
  strcpy(menu_tpl[LCM_MAIN_MOXAGATE][0], "page up  ");
  strcpy(menu_tpl[LCM_MAIN_MOXAGATE][1], "home     ");
  strcpy(menu_tpl[LCM_MAIN_MOXAGATE][2], "stat detl");
  strcpy(menu_tpl[LCM_MAIN_MOXAGATE][3], "about    ");
  strcpy(menu_tpl[LCM_MAIN_MOXAGATE][4], "settings ");
  strcpy(menu_tpl[LCM_MAIN_MOXAGATE][5], "exit appl");

  menu_actions_amount[LCM_MAIN_EVENTLOG]=4;
  strcpy(menu_tpl[LCM_MAIN_EVENTLOG][0], "page up  ");
  strcpy(menu_tpl[LCM_MAIN_EVENTLOG][1], "home     ");
  strcpy(menu_tpl[LCM_MAIN_EVENTLOG][2], "show log ");
  strcpy(menu_tpl[LCM_MAIN_EVENTLOG][3], "about    ");

  menu_actions_amount[LCM_MAIN_SYSINFO]=4;
  strcpy(menu_tpl[LCM_MAIN_SYSINFO][0], "page up  ");
  strcpy(menu_tpl[LCM_MAIN_SYSINFO][1], "home     ");
  strcpy(menu_tpl[LCM_MAIN_SYSINFO][2], "bck light");
  strcpy(menu_tpl[LCM_MAIN_SYSINFO][3], "about    ");

  menu_actions_amount[LCM_MAIN_SETTINGS]=4;
  strcpy(menu_tpl[LCM_MAIN_SETTINGS][0], "page up  ");
  strcpy(menu_tpl[LCM_MAIN_SETTINGS][1], "home     ");
  strcpy(menu_tpl[LCM_MAIN_SETTINGS][2], "enter    ");
  strcpy(menu_tpl[LCM_MAIN_SETTINGS][3], "about    ");

  return;
  }

/*//----------------------------------------------------------------------------
LCM_MAIN_IFRTU1
LCM_MAIN_IFRTU2

-----------------     -----------------
|   INTERFACES   |    |   INTERFACES   |
|----------------|    |----------------|
|SL Msgs Succ MOD|    |SL Msgs Succ MOD|
|P1>0123 4567 SRV|    |P5>0123 4567 INI|
|P2 8901 2345 MAS|    |P6 8901 2345 IDL|
|P3 6789 0123 SLV|    |P7 6789 0123 ERR|
|P4 4567 8901 OFF|    |P8 4567 8901 OFF|
|F2-PGDN  F4-DOWN|    |F2-PGDN  F4-DOWN|
-----------------     -----------------

*///----------------------------------------------------------------------------
void show_main_ifrtu1()
  {
  if(screen.main_scr_rtu>3) screen.main_scr_rtu = screen.main_scr_rtu%4;
    
  marker[0] = marker[1] = marker[2] = marker[3] = ' ';
  marker[screen.main_scr_rtu]='>';
  
	strcpy (screen.text[0], "   INTERFACES   ");
	strcpy (screen.text[1], "----------------");
	strcpy (screen.text[2], "SL Msgs Succ MOD");
	sprintf(screen.text[3], "P1%c%4.4d %4.4d %s", marker[0],
                                                IfaceRTU[GATEWAY_P1].stat.accepted%10000,
                                                IfaceRTU[GATEWAY_P1].stat.sended%10000,
                                                IfaceRTU[GATEWAY_P1].bridge_status);
	sprintf(screen.text[4], "P2%c%4.4d %4.4d %s", marker[1],
                                                IfaceRTU[GATEWAY_P2].stat.accepted%10000,
                                                IfaceRTU[GATEWAY_P2].stat.sended%10000,
                                                IfaceRTU[GATEWAY_P2].bridge_status);
	sprintf(screen.text[5], "P3%c%4.4d %4.4d %s", marker[2],
                                                IfaceRTU[GATEWAY_P3].stat.accepted%10000,
                                                IfaceRTU[GATEWAY_P3].stat.sended%10000,
                                                IfaceRTU[GATEWAY_P3].bridge_status);
	sprintf(screen.text[6], "P4%c%4.4d %4.4d %s", marker[3],
                                                IfaceRTU[GATEWAY_P4].stat.accepted%10000,
                                                IfaceRTU[GATEWAY_P4].stat.sended%10000,
                                                IfaceRTU[GATEWAY_P4].bridge_status);
	strcpy (screen.text[7], "F2-PGDN  F4-DOWN");

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }

void show_main_ifrtu2()
  {
  if((screen.main_scr_rtu<4)||(screen.main_scr_rtu>7))
    screen.main_scr_rtu = screen.main_scr_rtu%4+4;
    
  marker[0] =  marker[1] = marker[2] = marker[3] = ' ';
  marker[screen.main_scr_rtu-4]='>';
  	
	strcpy (screen.text[0], "   INTERFACES   ");
	strcpy (screen.text[1], "----------------");
	strcpy (screen.text[2], "SL Msgs Succ MOD");
	sprintf(screen.text[3], "P5%c%4.4d %4.4d %s", marker[0],
                                                IfaceRTU[GATEWAY_P5].stat.accepted%10000,
                                                IfaceRTU[GATEWAY_P5].stat.sended%10000,
                                                IfaceRTU[GATEWAY_P5].bridge_status);
	sprintf(screen.text[4], "P6%c%4.4d %4.4d %s", marker[1],
                                                IfaceRTU[GATEWAY_P6].stat.accepted%10000,
                                                IfaceRTU[GATEWAY_P6].stat.sended%10000,
                                                IfaceRTU[GATEWAY_P6].bridge_status);
	sprintf(screen.text[5], "P7%c%4.4d %4.4d %s", marker[2],
                                                IfaceRTU[GATEWAY_P7].stat.accepted%10000,
                                                IfaceRTU[GATEWAY_P7].stat.sended%10000,
                                                IfaceRTU[GATEWAY_P7].bridge_status);
	sprintf(screen.text[6], "P8%c%4.4d %4.4d %s", marker[3],
                                                IfaceRTU[GATEWAY_P8].stat.accepted%10000,
                                                IfaceRTU[GATEWAY_P8].stat.sended%10000,
                                                IfaceRTU[GATEWAY_P8].bridge_status);
	strcpy (screen.text[7], "F2-PGDN  F4-DOWN");

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }
/*//----------------------------------------------------------------------------
LCM_MAIN_IFTCP

-----------------
|   INTERFACES   |
|----------------|
|TCP Msgs Succ MR|
|T01>0123 4567 + |
|T11 8901 2345 +-|
|T21 6789 0123 + |
|T32 4567 8901 -+|
|F2-PGDN  F4-DOWN|
-----------------

*///----------------------------------------------------------------------------
void show_main_iftcp()
  {
	strcpy (screen.text[0], "   INTERFACES   ");
	strcpy (screen.text[1], "----------------");
	strcpy (screen.text[2], "TCP Msgs Succ MR");
	strcpy (screen.text[3], "                ");
	strcpy (screen.text[4], "                ");
	strcpy (screen.text[5], "                ");
	strcpy (screen.text[6], "                ");
	strcpy (screen.text[7], "F2-PGDN  F4-DOWN");
																												
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

  
	  sprintf(screen.text[i-screen.main_tcp_start+3], "T%s%4.4d %4.4d%s", marker,
                                                  IfaceTCP[Security.TCPIndex[i]].stat.accepted%10000,
                                                  IfaceTCP[Security.TCPIndex[i]].stat.sended%10000,
                                                  IfaceTCP[Security.TCPIndex[i]].bridge_status);
  }
	 
  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }
/*//----------------------------------------------------------------------------
LCM_MAIN_MOXAGATE
-----------------
| GATEWAY DEVICE |
|----------------|
|Clients  con  23|
|Rejected con  01|
|Accepted con0023|
|Messages    0123|
|Valid Resp  0123|
|F2-PGDN  F5-MENU|
-----------------

*///----------------------------------------------------------------------------
void show_main_moxagate()
  {
	strcpy (screen.text[0], " GATEWAY DEVICE ");
	strcpy (screen.text[1], "----------------");
	sprintf(screen.text[2], "Clients       %2.2d", Security.current_connections_number);
	sprintf(screen.text[3], "Rejected      %2.2d", Security.rejected_connections_number);
	sprintf(screen.text[4], "Accepted    %4.4d", Security.accepted_connections_number);
	sprintf(screen.text[5], "Messages    %4.4d", MoxaDevice.stat.accepted);
	sprintf(screen.text[6], "Valid Resp  %4.4d", MoxaDevice.stat.sended);
	strcpy (screen.text[7], "F2-PGDN  F5-MENU");

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
|F2-PGDN  F4-DOWN|
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
                                                EventLog.inf_msgs_amount+
                                                EventLog.wrn_msgs_amount+
                                                EventLog.err_msgs_amount);
	sprintf(screen.text[3], "%cINF Events %4.4d", marker[1], EventLog.inf_msgs_amount);
	sprintf(screen.text[4], "%cWRN Events %4.4d", marker[2], EventLog.wrn_msgs_amount);
	sprintf(screen.text[5], "%cERR Events %4.4d", marker[3], EventLog.err_msgs_amount);
//  if(Security.show_sys_messages)
//	       sprintf(screen.text[6], "Debug Events  On");
//    else sprintf(screen.text[6], "Debug Events Off");
  strcpy (screen.text[6], "                ");
	strcpy (screen.text[7], "F2-PGDN  F4-DOWN");

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

*///----------------------------------------------------------------------------
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
	strcpy(screen.text[7], "         F1-BACK");

  switch(screen.prev_screen) {

    case LCM_MAIN_IFRTU1:
    case LCM_MAIN_IFRTU2:
    case LCM_MAIN_IFTCP:
    strcpy(screen.text[2], "F2-Page down    ");
    strcpy(screen.text[3], "F3-Enter detail ");
    strcpy(screen.text[4], "statistic screen");
    strcpy(screen.text[5], "F4-Move cursor  ");
    strcpy(screen.text[6], "F5-Menu        ");
    screen.text[3][5]=9;
    break;

    case LCM_MAIN_MOXAGATE:
    strcpy(screen.text[2], "F2-Page down    ");
    strcpy(screen.text[3], "F3-Enter detail ");
    strcpy(screen.text[4], "statistic screen");
    strcpy(screen.text[5], "F5-Menu         ");
    //strcpy(screen.text[6], "");
    break;

    case LCM_MAIN_EVENTLOG:
    strcpy(screen.text[2], "F2-Page down    ");
    strcpy(screen.text[3], "F3-Goto selected");
    strcpy(screen.text[4], "   events detail");
    strcpy(screen.text[5], "F4-Move cursor  ");
    strcpy(screen.text[6], "F5-Menu        ");
    break;

    case LCM_MAIN_SYSINFO:
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

    case LCM_MAIN_MENU:
    strcpy(screen.text[2], "F2-Move up      ");
    strcpy(screen.text[3], "F3-Select       ");
    strcpy(screen.text[4], "F4-Move down    ");
    strcpy(screen.text[5], "F5-Cancel       ");
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
	strcpy(screen.text[7], "         F5-BACK");

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }
/*//----------------------------------------------------------------------------
LCM_MAIN_MENU						 

-----------------
| GATEWAY DEVICE |
|---menu---------|
|CP| PAGE DOWN|5%|
|RA|>STAT DETL|0%|
|FL| MOVE CURS|8%|
|Sy| HELP     |  |
|05 ---------- 1s|
|F3-ENTER F4-DOWN|
-----------------

*///----------------------------------------------------------------------------
void show_main_menu()
  {				
  marker[0]=screen.text[1][14];
	strcpy(&screen.text[1][2], "-menu-------");
  screen.text[1][14]=marker[0];

  marker[0]=screen.text[2][14];
  strcpy(&screen.text[2][2], "l          l");
  screen.text[2][14]=marker[0];

  marker[0]=screen.text[3][14];
  strcpy(&screen.text[3][2], "l          l");
  screen.text[3][14]=marker[0];

  marker[0]=screen.text[4][14];
  strcpy(&screen.text[4][2], "l          l");
  screen.text[4][14]=marker[0];

  marker[0]=screen.text[5][14];
  strcpy(&screen.text[5][2], "l          l");
  screen.text[5][14]=marker[0];

  //marker[0]=screen.text[6][14];
  //strcpy(&screen.text[6][2], " ---------- ");
  for(i=3; i<=12; i++) screen.text[6][i]=22;
  screen.text[6][2]=screen.text[6][13]='l';
  //screen.text[6][14]=marker[0];

	strcpy(screen.text[7], "F3-ENTER F4-DOWN");
  screen.text[7][0]=screen.text[7][9]=0xF;
  screen.text[7][1 ]=3;
  screen.text[7][10]=4;

  if(menu_actions_amount[screen.prev_screen]==0) {
    mxlcm_write_screen(mxlcm_handle, screen.text);
    return;
    }

  if(screen.main_menu_action==LCM_MENU_MOVE_DOWN)
    if(screen.main_menu_current < menu_actions_amount[screen.prev_screen]-1)
           screen.main_menu_current++;
      else screen.main_menu_current=0;

  if(screen.main_menu_action==LCM_MENU_MOVE_UP)
    if(screen.main_menu_current > 0)
           screen.main_menu_current--;
      else screen.main_menu_current=menu_actions_amount[screen.prev_screen]-1;

  if(screen.main_menu_current < screen.main_menu_start)
    screen.main_menu_start = screen.main_menu_current;
  else if(screen.main_menu_current - screen.main_menu_start > 3)
    screen.main_menu_start = screen.main_menu_current - 3;

  for(i = screen.main_menu_start; i < screen.main_menu_start+4; i++) {
    if( (i >= menu_actions_amount[screen.prev_screen]) ||
        (menu_tpl[screen.prev_screen][i][0]==0)
      ) continue;
    strcpy(marker, &screen.text[i-screen.main_menu_start+2][13]);
    sprintf(&screen.text[i-screen.main_menu_start+2][3], " %s%s",
      menu_tpl[screen.prev_screen][i], marker);
    if(i == screen.main_menu_current)
      screen.text[i-screen.main_menu_start+2][3]=16; // ñòðåëêà âèäà ->
    }

  screen.main_menu_action=LCM_MENU_NOACTION;

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }
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

*///----------------------------------------------------------------------------
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
/*//----------------------------------------------------------------------------
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
///----------------------------------------------------------------------------
PORT1-------------
| MODBUS GATEWAY |
|Serial Port   P1|
|Mode     RS4852W|
|Speed       9600|
|Parity      NONE|
|Timeout     2000|
|TCP Port     502|
|         F2-BACK|
------------------
LCM_SCREEN_PORT1
LCM_SCREEN_PORT2
LCM_SCREEN_PORT3
LCM_SCREEN_PORT4
LCM_SCREEN_PORT5
LCM_SCREEN_PORT6
LCM_SCREEN_PORT7
LCM_SCREEN_PORT8
*///---------------------------------------------------------------
void show_uart_settings(int uart)
  {
  
  char prm[5][9], tmp[9];
  int i;

  for(i=0; i<5; i++) strcpy(prm[i], "     ---");
  
  if(strcmp(IfaceRTU[uart].bridge_status, "OFF")!=0) {
	  i=strlen(IfaceRTU[uart].serial.p_mode);
	  if(i<9) strcpy(&prm[0][8-i], IfaceRTU[uart].serial.p_mode);
	  if(prm[0][5]=='_') prm[0][5]=' ';
	  i=strlen(IfaceRTU[uart].serial.speed);
	  if(i<9) strcpy(&prm[1][8-i], IfaceRTU[uart].serial.speed);
	  i=strlen(IfaceRTU[uart].serial.parity);
	  if(i<9) strcpy(&prm[2][8-i], IfaceRTU[uart].serial.parity);
		sprintf(tmp, "%d", IfaceRTU[uart].serial.timeout/1000);
	  i=strlen(tmp);
	  if(i<9) strcpy(&prm[3][8-i], tmp);
		sprintf(tmp, "%d", IfaceRTU[uart].Security.tcp_port);
	  i=strlen(tmp);
	  if(i<9) strcpy(&prm[4][8-i], tmp);
    }

	strcpy (screen.text[0], " MODBUS GATEWAY ");
	sprintf(screen.text[1], "Serial Port   P%d", uart+1);
	sprintf(screen.text[2], "Mode    %s", prm[0]);
	sprintf(screen.text[3], "Speed   %s", prm[1]);
	sprintf(screen.text[4], "Parity  %s", prm[2]);
	sprintf(screen.text[5], "Timeout %s", prm[3]);
	sprintf(screen.text[6], "TCP Port%s", prm[4]);
	strcpy (screen.text[7], "         F2-BACK");

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }
/*//----------------------------------------------------------------------------
DETAIL------------
|MODBUS PORT1 08M|
|Messages    0123|
|Timeouts    0123|
|CRC Errors  0123|
|Errors      0123|
|Time delay  0123|
|F3-Reset counter|
|F4-RSALL F2-BACK|
------------------
LCM_SCREEN_PORT1_STAT
LCM_SCREEN_PORT2_STAT
LCM_SCREEN_PORT3_STAT
LCM_SCREEN_PORT4_STAT
LCM_SCREEN_PORT5_STAT
LCM_SCREEN_PORT6_STAT
LCM_SCREEN_PORT7_STAT
LCM_SCREEN_PORT8_STAT
*///---------------------------------------------------------------
void show_uart_detail(int uart)
  {
	sprintf(screen.text[0], "MODBUS PORT%d %s", uart+1, IfaceRTU[uart].bridge_status);
	sprintf(screen.text[1], "Messages    %4.4d", IfaceRTU[uart].stat.sended%10000);
	sprintf(screen.text[2], "Timeouts    %4.4d", IfaceRTU[uart].stat.timeouts%10000);
	sprintf(screen.text[3], "CRC Errors  %4.4d", IfaceRTU[uart].stat.crc_errors%10000);
	sprintf(screen.text[4], "Errors      %4.4d", IfaceRTU[uart].stat.errors%10000);
	sprintf(screen.text[5], "Time delay  %4.4d", IfaceRTU[uart].stat.request_time);
	strcpy (screen.text[6], "F3-Reset counter");
	strcpy (screen.text[7], "F4-RSALL F2-BACK");

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }
/*//----------------------------------------------------------------------------
CONFIRMATION------
| MODBUS GATEWAY |
|                |
|  ARE YOU SURE  |
|  TO RESET ALL  |
|    COUNTERS?   |
|                |
| F2-NO   F4-YES |
|                |
------------------
LCM_CONFIRM_RESETALL
*///---------------------------------------------------------------
void show_confirmation_reset_all()
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

/*
CONFIRMATION------
| MODBUS GATEWAY |
|                |
|  DO YOU WANT   |
|TO SAVE CHANGES?|
|                |
|                |
| F2-NO  F4-SAVE |
|                |
------------------
LCM_CONFIRM_REBOOT
*///---------------------------------------------------------------
void show_confirmation_secr_changes()
  {
	strcpy(screen.text[0], " MODBUS GATEWAY ");
	strcpy(screen.text[1], "                ");
	strcpy(screen.text[2], "  DO YOU WANT   ");
	strcpy(screen.text[3], "TO SAVE CHANGES?");
	strcpy(screen.text[4], "                ");
	strcpy(screen.text[5], "                ");
	strcpy(screen.text[6], " F2-NO  F4-SAVE ");
	strcpy(screen.text[7], "                ");

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }
/*
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
LCM_CONFIRM_REBOOT
*///---------------------------------------------------------------
void show_confirmation_reboot()
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
/*
STUB SCREEN-------
| MODBUS GATEWAY |
|                |
|     SORRY      |
|  THIS FEATURE  |
|     ISN'T      |
|  IMPLEMENTED   |
|                |
|         F2-BACK|
------------------
LCM_SCREEN_STUB_SECURITY
*///---------------------------------------------------------------
void show_stub_screen()
  {
//	strcpy(screen.text[0], "                ");
//	strcpy(screen.text[1], "     SORRY      ");
//	strcpy(screen.text[2], "  THIS FEATURE  ");
//	strcpy(screen.text[3], "     ISN'T      ");
//	strcpy(screen.text[4], "  IMPLEMENTED   ");
//	strcpy(screen.text[5], "      YET       ");
//	strcpy(screen.text[6], "                ");
//	strcpy(screen.text[7], "         F3-BACK");
	
  k=1;
  if(screen.main_scr_sett==1) k=11;
  if(screen.main_scr_sett==2) k=42;
  if(screen.main_scr_sett==3) k=73;
  if(screen.main_scr_sett==4) k=104;
  for(i=0; i<8; i+=2) {
    for(j=0; j<16; j+=2) {
      screen.text[i][j]=32;
      screen.text[i][j+1]=k++;
      }
    screen.text[i][j]=0;
    }

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }
///----------------------------------------------------------------------------
int ctrl_reset_all_counters()
  {
  ctrl_reset_port_counters(GATEWAY_P1);
  ctrl_reset_port_counters(GATEWAY_P2);
  ctrl_reset_port_counters(GATEWAY_P3);
  ctrl_reset_port_counters(GATEWAY_P4);
  ctrl_reset_port_counters(GATEWAY_P5);
  ctrl_reset_port_counters(GATEWAY_P6);
  ctrl_reset_port_counters(GATEWAY_P7);
  ctrl_reset_port_counters(GATEWAY_P8);
   
  return 0;	
  }
int ctrl_reset_port_counters(int port)
  {
  if(port<0 || port>7) return 1;

  IfaceRTU[port].stat.accepted=\
  IfaceRTU[port].stat.errors_input_communication=\
  IfaceRTU[port].stat.errors_tcp_adu=\
  IfaceRTU[port].stat.errors_tcp_pdu=\
  IfaceRTU[port].stat.errors_serial_sending=\
  IfaceRTU[port].stat.errors_serial_accepting=\
  IfaceRTU[port].stat.timeouts=\
  IfaceRTU[port].stat.crc_errors=\
  IfaceRTU[port].stat.errors_serial_adu=\
  IfaceRTU[port].stat.errors_serial_pdu=\
  IfaceRTU[port].stat.errors_tcp_sending=\
  IfaceRTU[port].stat.errors=\
  IfaceRTU[port].stat.sended=0;

//  IfaceRTU[port].stat.latency_history[MAX_LATENCY_HISTORY_POINTS];
//  IfaceRTU[port].stat.clp; // current latensy point
  int i;
  for(i=0; i<MB_FUNCTIONS_IMPLEMENTED*2+1; i++)
  	//IfaceRTU[port].stat.input_messages[i]=\
  	IfaceRTU[port].stat.output_messages[i]=0;

  return 0;	
  }

int ctrl_reboot_system()
  {
	Security.halt=1;
  return 0;	
  }

/*
-----------------
|LOG EVENTS <ALL>|
|1234 INF   PORT1|
|03 DEC  14:32:00|
|- COMMAND LINE P|
|ARSED SUCCESSFUL|
|LY              |
|                |
|F2-BACK  F4-NEXT|
-----------------
F1-HELP (Êîíòåêñòíàÿ ñïðàâêà)
F5-OPTIONS (Ïîðÿäîê ñîðòèðîâêè)

*/
