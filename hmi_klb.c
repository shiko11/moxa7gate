/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///**** МОДУЛЬ МОНИТОРИНГА И УПРАВЛЕНИЯ РАБОТОЙ ШЛЮЗА СРЕДСТВАМИ KEYPAD-LCM ****

///=== HMI_KLB_H IMPLEMENTATION

#include <pthread.h>

#include "hmi_klb.h"
#include "hmi_web.h"
#include "interfaces.h"
#include "moxagate.h"
#include "messages.h"

///=== HMI_KLB_H private variables

int rc;
pthread_t tstTH;
unsigned int i, j, k;
char marker[8];

unsigned menu_actions_amount[LCM_MAIN_SETTINGS+1];
char menu_tpl[LCM_MAIN_SETTINGS+1][LCM_MENU_MAX_ACTIONS][LCM_MENU_ACTDEFLEN];

///=== HMI_KLB_H private functions

void init_menu_tpl();

///---------------------------------------------------------------
void init_menu_tpl()
  {

  menu_actions_amount[LCM_MAIN_IFRTU1]=2;
  strcpy(menu_tpl[LCM_MAIN_IFRTU1][0], "PAGE UP  ");
  strcpy(menu_tpl[LCM_MAIN_IFRTU1][1], "STAT DETL");

  menu_actions_amount[LCM_MAIN_IFRTU2]=2;
  strcpy(menu_tpl[LCM_MAIN_IFRTU2][0], "PAGE UP  ");
  strcpy(menu_tpl[LCM_MAIN_IFRTU2][1], "STAT DETL");

  menu_actions_amount[LCM_MAIN_IFTCP]=2;
  strcpy(menu_tpl[LCM_MAIN_IFTCP][0], "PAGE UP  ");
  strcpy(menu_tpl[LCM_MAIN_IFTCP][1], "STAT DETL");

  menu_actions_amount[LCM_MAIN_MOXAGATE]=3;
  strcpy(menu_tpl[LCM_MAIN_MOXAGATE][0], "PAGE UP  ");
  strcpy(menu_tpl[LCM_MAIN_MOXAGATE][1], "STAT DETL");
  strcpy(menu_tpl[LCM_MAIN_MOXAGATE][2], "EXIT APPL");

  menu_actions_amount[LCM_MAIN_EVENTLOG]=2;
  strcpy(menu_tpl[LCM_MAIN_EVENTLOG][0], "PAGE UP  ");
  strcpy(menu_tpl[LCM_MAIN_EVENTLOG][1], "SHOW LOG ");

  menu_actions_amount[LCM_MAIN_SYSINFO]=3;
  strcpy(menu_tpl[LCM_MAIN_SYSINFO][0], "PAGE UP  ");
  strcpy(menu_tpl[LCM_MAIN_SYSINFO][1], "BCK LIGHT");
  strcpy(menu_tpl[LCM_MAIN_SYSINFO][2], "ABOUT    ");

  menu_actions_amount[LCM_MAIN_SETTINGS]=3;
  strcpy(menu_tpl[LCM_MAIN_SETTINGS][0], "PAGE UP  ");
  strcpy(menu_tpl[LCM_MAIN_SETTINGS][1], "ENTER    ");
  strcpy(menu_tpl[LCM_MAIN_SETTINGS][2], "ABOUT    ");

  return;
  }
///---------------------------------------------------------------
int init_hmi_keypad_lcm_h()
  {

  mxlcm_handle=-1;
  mxkpd_handle=-1;
  mxbzr_handle=-1;

	mxkpd_handle=keypad_open();
  if(mxkpd_handle<0) {
	  sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_FTL|GATEWAY_HMI, HMI_KLB_INIT_KEYPAD, 0, 0, 0, 0);
    return HMI_KLB_INIT_KEYPAD;
    }

	mxlcm_handle = mxlcm_open();
  if(mxlcm_handle<0) {
	  sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_FTL|GATEWAY_HMI, HMI_KLB_INIT_LCM, 0, 0, 0, 0);
    return HMI_KLB_INIT_LCM;
    }
  mxlcm_control(mxlcm_handle, IOCTL_LCM_AUTO_SCROLL_OFF);

	mxbzr_handle = mxbuzzer_open();
  if(mxbzr_handle<0) {
	  sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_FTL|GATEWAY_HMI, HMI_KLB_INIT_BUZZER, 0, 0, 0, 0);
    return HMI_KLB_INIT_BUZZER;
    }

	screen.current_screen=LCM_MAIN_IFRTU1;

  main_scr_rtu = GATEWAY_P1;      // LCM_MAIN_IFRTU1, LCM_MAIN_IFRTU2
  main_scr_tcp = 0;      // LCM_MAIN_IFTCP
  main_scr_tcppage = 1;  // LCM_MAIN_IFTCP
  main_scr_eventlog=0; // LCM_MAIN_EVENTLOG
  main_menu_start;   // LCM_MAIN_MENU
  main_menu_current; // LCM_MAIN_MENU
  main_scr_sett=0;     // LCM_MAIN_SETTINGS

  screen.back_light=1;
  //screen.max_tcp_clients_per_com=8;
  screen.watch_dog_control=0;
  screen.buzzer_control=1;
  //screen.secr_scr_changes_was_made=0;

  // запускаем поток для обработки нажатий клавиш
  // и вывода на дисплей информации для мониторинга

	rc = pthread_create(
		&tstTH,
		NULL,
		mx_keypad_lcm,
		NULL);

  if(rc!=0) {
	  sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_FTL|GATEWAY_HMI, HMI_KLB_INIT_THREAD, 0, 0, 0, 0);
    return HMI_KLB_INIT_THREAD;
    }

  init_menu_tpl();

  ///!!! сохранение в специальном массиве исходного текста на дисплее

  return 0;
  }

int clear_hmi_keypad_lcm_h()
  {

  ///!!! для эффективного управления потоками программы нужно изучить модель для них в Linux

  mxlcm_close(mxlcm_handle);
  keypad_close(mxkpd_handle);
  mxbuzzer_close(mxbzr_handle);

  mxlcm_handle=-1;
  mxkpd_handle=-1;
  mxbzr_handle=-1;

  return 0;
  }
///---------------------------------------------------------------
void *mx_keypad_lcm(void *arg)
  {
  int i;
  
  while(1) {

    if( mxlcm_handle==-1 ||
        mxkpd_handle==-1 ||
        mxbzr_handle==-1
      ) i=-1;
      else i=keypad_get_pressed_key(mxkpd_handle); // usually -1
	  
    if(i>=0) { // if key was pressed
	  if(	screen.current_screen==LCM_MAIN_IFRTU1				||
	  		screen.current_screen==LCM_MAIN_IFRTU2				||
	  		screen.current_screen==LCM_MAIN_IFTCP		||
	  		screen.current_screen==LCM_MAIN_MOXAGATE				||
	  		screen.current_screen==LCM_MAIN_EVENTLOG			||
	  		screen.current_screen==LCM_MAIN_SYSINFO	||
	  		screen.current_screen==LCM_MAIN_SETTINGS
	  		) process_key_main(i);
//	  else
//	  if(	screen.current_screen==LCM_SCREEN_MENU				||
//	  		screen.current_screen==LCM_SCREEN_HELP_MENU		||
//	  		screen.current_screen==LCM_SCREEN_PORT1				||
//	  		screen.current_screen==LCM_SCREEN_PORT2				||
//	  		screen.current_screen==LCM_SCREEN_PORT3				||
//	  		screen.current_screen==LCM_SCREEN_PORT4				||
//	  		screen.current_screen==LCM_SCREEN_PORT5				||
//	  		screen.current_screen==LCM_SCREEN_PORT6				||
//	  		screen.current_screen==LCM_SCREEN_PORT7				||
//	  		screen.current_screen==LCM_SCREEN_PORT8 			||
//	  		screen.current_screen==LCM_SCREEN_LAN					||
//	  		screen.current_screen==LCM_CONFIRM_REBOOT
//	  		) process_key_menu(i);
//	  else
//	  if(	screen.current_screen==LCM_SCREEN_SECURITY			||
//	  		screen.current_screen==LCM_SCREEN_HELP_SECURITY	||
//	  		screen.current_screen==LCM_SCREEN_STUB_SECURITY ||
//	  		screen.current_screen==LCM_CONFIRM_SECR_CHANGES
//	  		) process_key_security(i);
    }

		// refresh_shm(&IfaceRTU); // необходимо заменить явный перенос значений вызовами специальных фнукций
	  usleep(LCM_SCREEN_UPDATE_RATE);

    if(!(
        mxlcm_handle==-1 ||
        mxkpd_handle==-1 ||
        mxbzr_handle==-1
      ))
	  if(	screen.current_screen >= LCM_MAIN_IFRTU1  &&
	  		screen.current_screen <= LCM_NOT_IMPLEMENTED
	  		) show_screen(screen.current_screen);
	  }

  return;
  }
///---------------------------------------------------------------
void show_screen(int display)
  {
	switch(display) {

		case LCM_MAIN_IFRTU1:   show_main_ifrtu1();   break;
		case LCM_MAIN_IFRTU2:   show_main_ifrtu2();   break;
		case LCM_MAIN_IFTCP:    show_main_iftcp();    break;
		case LCM_MAIN_MOXAGATE: show_main_moxagate(); break;
		case LCM_MAIN_EVENTLOG: show_main_eventlog(); break;
		case LCM_MAIN_SYSINFO:  show_system_info();   break;
		case LCM_MAIN_SETTINGS: show_main_settings(); break;

		default: return;
	  }

//	if(display==LCM_SCREEN_HELP_MAIN ||
//	   display==LCM_CONFIRM_RESETALL ||
//	   (display==LCM_SCREEN_MENU &&
//	    (screen.current_screen==LCM_SCREEN_MAIN || screen.current_screen==LCM_SCREEN_MAIN2))
//	  ) screen.prev_screen=screen.current_screen;

  screen.current_screen=display;
  return;
  }
///---------------------------------------------------------------
void process_key_main(int key)
  {
  static int new_screen;
  new_screen=-1;
  	
  switch(key) {
  	case KEY_F1: ///------------------
  	  if(	screen.current_screen==LCM_SCREEN_MAIN ||
  	  		screen.current_screen==LCM_SCREEN_MAIN2)
  	    new_screen=LCM_SCREEN_HELP_MAIN;
  	  if(new_screen!=-1) show_screen(new_screen);
  		break;
  	case KEY_F2: ///------------------
  	  if(screen.current_screen==LCM_SCREEN_MAIN )	{
  	  	screen.main_scr_mode+=4;
  	  	new_screen=LCM_SCREEN_MAIN2;
  	    }
  	  if(screen.current_screen==LCM_SCREEN_MAIN2)	{
  	  	screen.main_scr_mode-=4;
  	  	new_screen=LCM_SCREEN_MAIN;
  	    }
  	  if(screen.current_screen==LCM_SCREEN_HELP_MAIN ||
  	     screen.current_screen==LCM_SCREEN_ABOUT     ||
  	     screen.current_screen==LCM_SCREEN_SYSINFO   ||
  	     screen.current_screen==LCM_CONFIRM_RESETALL)
  	    new_screen=screen.prev_screen;
  	  if(screen.current_screen>=LCM_SCREEN_PORT1_STAT &&
  	     screen.current_screen<=LCM_SCREEN_PORT4_STAT)
  	    new_screen=LCM_SCREEN_MAIN;
  	  if(screen.current_screen>=LCM_SCREEN_PORT5_STAT &&
  	     screen.current_screen<=LCM_SCREEN_PORT8_STAT)
  	    new_screen=LCM_SCREEN_MAIN2;
  	  if(new_screen!=-1) show_screen(new_screen);
  		break;
  	case KEY_F3: ///------------------
  	  if(	screen.current_screen==LCM_SCREEN_MAIN ||
  	  		screen.current_screen==LCM_SCREEN_MAIN2)
  	    new_screen=LCM_SCREEN_PORT1_STAT+screen.main_scr_mode-1;
  	  if(screen.current_screen>=LCM_SCREEN_PORT1_STAT &&
  	     screen.current_screen<=LCM_SCREEN_PORT8_STAT) {
  	     ctrl_reset_port_counters(screen.current_screen-LCM_SCREEN_PORT1_STAT);
  	     new_screen=screen.current_screen;
  	     }
  	  if(new_screen!=-1) show_screen(new_screen);
  		break;
  	case KEY_F4: ///------------------
  	  if(screen.current_screen==LCM_SCREEN_MAIN )	{
  	  	screen.main_scr_mode=screen.main_scr_mode<4?screen.main_scr_mode+1:1;
  	  	new_screen=screen.current_screen;
  	    }
  	  if(screen.current_screen==LCM_SCREEN_MAIN2)	{
  	  	screen.main_scr_mode=screen.main_scr_mode<8?screen.main_scr_mode+1:5;
  	  	new_screen=screen.current_screen;
  	    }
  	  if(screen.current_screen==LCM_SCREEN_HELP_MAIN) new_screen=LCM_SCREEN_SYSINFO;
  	  if(screen.current_screen==LCM_SCREEN_SYSINFO  ) new_screen=LCM_SCREEN_ABOUT;
  	  if(screen.current_screen>=LCM_SCREEN_PORT1_STAT &&
  	     screen.current_screen<=LCM_SCREEN_PORT8_STAT)
  	    new_screen=LCM_CONFIRM_RESETALL;
  	  if(screen.current_screen==LCM_CONFIRM_RESETALL) {
  	     ctrl_reset_all_counters();
  	     new_screen=screen.prev_screen;
  	     }
  	  if(new_screen!=-1) show_screen(new_screen);
  		break;
  	case KEY_F5: ///------------------
  	  if(	screen.current_screen==LCM_SCREEN_MAIN ||
  	  		screen.current_screen==LCM_SCREEN_MAIN2) {
  	  	screen.menu_scr_mode=1;
  	    new_screen=LCM_SCREEN_MENU;
//printf("internal %d, external %d\n", t_tcpsrv[0].port, tcp_servers[0].port);
//printf("internal %d, external %d\n", t_tcpsrv[1].port, tcp_servers[1].port);
//printf("internal %d, external %d\n", t_tcpsrv[2].port, tcp_servers[2].port);
  	  	}
  	  if(new_screen!=-1) show_screen(new_screen);
  		break;
  	default:;    ///------------------
    };

  return;
  }
///---------------------------------------------------------------
void process_key_menu(int key)
  {
  static int new_screen;
  new_screen=-1;
  	
  switch(key) {
  	case KEY_F1: ///------------------
  	  if(screen.current_screen==LCM_SCREEN_MENU) new_screen=LCM_SCREEN_HELP_MENU;
  	  if(new_screen!=-1) show_screen(new_screen);
  		break;
  	case KEY_F2: ///------------------
  	  if(screen.current_screen==LCM_SCREEN_MENU) {
  	  	screen.menu_scr_mode=screen.menu_scr_mode>1?screen.menu_scr_mode-1:12;
  	  	new_screen=screen.current_screen;
  	    }
  	  if((screen.current_screen>=LCM_SCREEN_PORT1 &&
  	      screen.current_screen<=LCM_SCREEN_PORT8)		||
  	      screen.current_screen==LCM_SCREEN_HELP_MENU ||
  	      screen.current_screen==LCM_SCREEN_LAN 			||
  	      screen.current_screen==LCM_CONFIRM_REBOOT)
  	    new_screen=LCM_SCREEN_MENU;
  	  if(new_screen!=-1) show_screen(new_screen);
  		break;
  	case KEY_F3: ///------------------
  	  switch(screen.menu_scr_mode) {
  	  	case 1:
  	  	case 2:
  	  	case 3:
  	  	case 4:
  	  	case 5:
  	  	case 6:
  	  	case 7:
  	  	case 8:
  	  		new_screen=screen.menu_scr_mode+LCM_SCREEN_PORT1-1;
  	  		break;
  	  	case 9:
  	  		new_screen=LCM_SCREEN_LAN;
  	  		break;
  	  	case 10:
  	  		screen.secr_scr_mode=1;
  	  		new_screen=LCM_SCREEN_SECURITY;
  	  		break;
  	  	case 11:
  	  		new_screen=LCM_CONFIRM_REBOOT;
  	  		break;
  	  	case 12:
  	  		new_screen=screen.prev_screen;
  	  		break;
  	  	default:;
  	    }
  	  if(new_screen!=-1) show_screen(new_screen);
  		break;
  	case KEY_F4: ///------------------
  	  if(screen.current_screen==LCM_SCREEN_MENU) {
  	  	screen.menu_scr_mode=screen.menu_scr_mode<12?screen.menu_scr_mode+1:1;
  	  	new_screen=screen.current_screen;
  	    }
  	  if(screen.current_screen==LCM_CONFIRM_REBOOT) {
  	  	 mxlcm_write(mxlcm_handle, 0, 6, " program stopped", MAX_LCM_COLS); // 16 characters
  	     ctrl_reboot_system();
  	     }
  	  if(new_screen!=-1) show_screen(new_screen);
  		break;
  	case KEY_F5: ///------------------
		  if(screen.back_light) {	screen.back_light=0;	mxlcm_control(mxlcm_handle, IOCTL_LCM_BACK_LIGHT_OFF);
		    } else { 							screen.back_light=1;	mxlcm_control(mxlcm_handle, IOCTL_LCM_BACK_LIGHT_ON);}
  		break;
  	default:;    ///------------------
    };

  return;
  }
///---------------------------------------------------------------
void process_key_security(int key)
  {
  static int new_screen;
  new_screen=-1;
  	
  switch(key) {
  	case KEY_F1: ///------------------
  	  if(screen.current_screen==LCM_SCREEN_SECURITY) new_screen=LCM_SCREEN_HELP_SECURITY;
  	  if(new_screen!=-1) show_screen(new_screen);
  		break;
  	case KEY_F2: ///------------------
  	  if(screen.current_screen==LCM_SCREEN_SECURITY) {
  	    if(!screen.secr_scr_changes_was_made) new_screen=LCM_SCREEN_MENU;
  	    else new_screen=LCM_CONFIRM_SECR_CHANGES;
  	    }
  	  if(screen.current_screen==LCM_CONFIRM_SECR_CHANGES) {
				screen.secr_scr_changes_was_made=0;
  	  	new_screen=LCM_SCREEN_MENU;
  	    }
  	  if(screen.current_screen==LCM_SCREEN_HELP_SECURITY ||
  	     screen.current_screen==LCM_SCREEN_STUB_SECURITY
  	     ) new_screen=LCM_SCREEN_SECURITY;
  	  if(new_screen!=-1) show_screen(new_screen);
  		break;
  	case KEY_F3: ///------------------
  	  switch(screen.secr_scr_mode) {
  	  	case 1:
  	  	case 2:
  	  		new_screen=LCM_SCREEN_STUB_SECURITY;
  	  		break;
  	  	case 3:
					screen.max_tcp_clients_per_com=screen.max_tcp_clients_per_com<8?screen.max_tcp_clients_per_com+1:1;
  	  		screen.secr_scr_changes_was_made=1;
  	  		new_screen=LCM_SCREEN_SECURITY;
  	  		break;
  	  	case 4:
  	  		screen.watch_dog_control=screen.watch_dog_control==0?1:0;
  	  		screen.secr_scr_changes_was_made=1;
  	  		new_screen=LCM_SCREEN_SECURITY;
  	  		break;
  	  	case 5:
  	  		screen.buzzer_control=screen.buzzer_control==0?1:0;
  	  		if(screen.buzzer_control) mxbuzzer_beep(mxbzr_handle, 400);
  	  		screen.secr_scr_changes_was_made=1;
  	  		new_screen=LCM_SCREEN_SECURITY;
  	  		break;
  	  	default:;
  	    }
  	  if(new_screen!=-1) show_screen(new_screen);
  		break;
  	case KEY_F4: ///------------------
  	  if(screen.current_screen==LCM_SCREEN_SECURITY) {
  	  	screen.secr_scr_mode=screen.secr_scr_mode<5?screen.secr_scr_mode+1:1;
  	  	new_screen=screen.current_screen;
  	    }
  	  if(screen.current_screen==LCM_CONFIRM_SECR_CHANGES) {
  	  	///---applying changes
  	  	ctrl_change_security_settings();
  	  	screen.secr_scr_changes_was_made=0;
  	  	new_screen=LCM_SCREEN_MENU;
  	    }
  	  if(new_screen!=-1) show_screen(new_screen);
  		break;
  	case KEY_F5: ///------------------
		  if(screen.back_light) {	screen.back_light=0;	mxlcm_control(mxlcm_handle, IOCTL_LCM_BACK_LIGHT_OFF);
		    } else { 							screen.back_light=1;	mxlcm_control(mxlcm_handle, IOCTL_LCM_BACK_LIGHT_ON);}
  		break;
  	default:;    ///------------------
    };

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
  static char t_num[4];

  /// передаем в этой переменной номер экрана, который надо отобразить:
  /// screen.main_scr_tcppage
  /// каждый экран содержит только инициализированные интерфейсы IFACE_TCPMASTER по 4 шт.
  /// соответственно экран с максимальным номером содержит последний рабочий интерфейс

  if(screen.main_scr_tcp > 3) {
    screen.main_scr_tcp%=4;
    screen.main_scr_tcppage++;
    }

  if( (screen.main_scr_tcppage == 0) ||
      (screen.main_scr_tcppage > (MAX_TCP_SERVERS/4))
    ) screen.main_scr_tcppage=1;

  j = 1;
  k = 0;
  t_num[0] = t_num[1] = t_num[2] = t_num[3] = MAX_TCP_SERVERS;
  for(i=0; i<MAX_TCP_SERVERS; i++) {
    if(IfaceTCP[i].modbus_mode!=IFACE_OFF) t_num[k++]=i;
    if((k==4)&&(j==screen.main_scr_tcppage)) break;
    if (k==4) {j++; k=0; t_num[0] = t_num[1] = t_num[2] = t_num[3] = MAX_TCP_SERVERS;}
    }

	strcpy (screen.text[0], "   INTERFACES   ");
	strcpy (screen.text[1], "----------------");
	strcpy (screen.text[2], "TCP Msgs Succ MR");
	strcpy (screen.text[7], "F2-PGDN  F4-DOWN");

  for(i=0; i<4; i++) {
	  strcpy (screen.text[3+i], "                ");
    if(t_num[i]==MAX_TCP_SERVERS) continue;

    if(screen.main_scr_tcp == i)
           sprintf(marker, "%2.2d>", t_num[i]+1);
      else sprintf(marker, "%2.2d ", t_num[i]+1);

  
	  sprintf(screen.text[3+i], "T%s%4.4d %4.4d%s", marker,
                                                  IfaceTCP[t_num[i]].stat.accepted%10000,
                                                  IfaceTCP[t_num[i]].stat.sended%10000,
                                                  IfaceTCP[t_num[i]].bridge_status);
  }
	 
  if((j==1)&&(k==0)) {
	  strcpy (screen.text[4], "THERE ARE NO ONE");
	  strcpy (screen.text[5], "CONFIGURED IFACE");
    }

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }
/*//----------------------------------------------------------------------------
LCM_MAIN_MOXAGATE
-----------------
| GATEWAY DEVICE |
|----------------|
|Clients       23|
|Accepted      23|
|Rejected      01|
|Messages    0123|
|Answers     0123|
|F2-PGDN   F5-OPT|
-----------------

*///----------------------------------------------------------------------------
void show_main_moxagate()
  {
	strcpy (screen.text[0], " GATEWAY DEVICE ");
	strcpy (screen.text[1], "----------------");
	sprintf(screen.text[2], "Clients       %2.2d", Security.current_connections_number);
	sprintf(screen.text[3], "Accepted      %2.2d", Security.accepted_connections_number);
	sprintf(screen.text[4], "Rejected      %2.2d", Security.rejected_connections_number);
	sprintf(screen.text[5], "Messages    %4.4d", MoxaDevice.stat.accepted);
	sprintf(screen.text[6], "Answers     %4.4d", MoxaDevice.stat.sended);
	strcpy (screen.text[7], "F2-PGDN   F5-OPT");

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
|Debug Events  On|
|F2-PGDN  F4-DOWN|
-----------------

*///---------------------------------------------------------------
void show_main_eventlog()
  {
  if(screen.main_scr_eventlog>3)
     screen.main_scr_eventlog=0;
    
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
  if(Security.show_sys_messages)
	       sprintf(screen.text[6], "Debug Events  On");
    else sprintf(screen.text[6], "Debug Events Off");
	strcpy (screen.text[7], "F2-PGDN  F4-DOWN");

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }

/*//----------------------------------------------------------------------------
LCM_MAIN_SYSINFO
-----------------
|MOXA UC-7410(+) |
|----------------|
|CPU        1.25%|
|RAM          20%|
|FLASH        18%|
|System uptime   |
|057d 11h 35m 01s|
|F2-PGDN   F5-OPT|
-----------------

*///----------------------------------------------------------------------------
void show_system_info()
  {
	time_t moment;

	time(&moment);
	int diff=difftime(moment, Security.start_time);
	
	strcpy (screen.text[0], "MOXA UC-7410(+) ");
	strcpy (screen.text[1], "----------------");
	strcpy (screen.text[2], "CPU usage    --%");
	strcpy (screen.text[3], "RAM usage    --%");
	strcpy (screen.text[4], "FLASH usage  --%");
	strcpy (screen.text[5], "System uptime   ");
	sprintf(screen.text[6], "%3.3dd %2.2dh %2.2dm %2.2ds", (diff/86400)%1000,
                                                         (diff/3600)%24,
                                                         (diff/60)%60,
                                                          diff%60);
	strcpy (screen.text[7], "F2-PGDN   F5-OPT");

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
    strcpy(screen.text[6], "F5-Menu         ");
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
    strcpy(screen.text[6], "F5-Menu         ");
    break;

    case LCM_MAIN_SYSINFO:
    strcpy(screen.text[2], "This is last pa-");
    strcpy(screen.text[3], "ge of main level");
    strcpy(screen.text[4], "F2-Home page    ");
    strcpy(screen.text[5], "F5-Menu         ");
    //strcpy(screen.text[6], "");
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
|         F2-BACK|
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
	strcpy(screen.text[7], "         F2-BACK");

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
void show_main_menu(int action)
  {
	strcpy(screen.text[1][2], "-menu-------");
  strcpy(screen.text[2][2], "|          |");
  strcpy(screen.text[3][2], "|          |");
  strcpy(screen.text[4][2], "|          |");
  strcpy(screen.text[5][2], "|          |");
  strcpy(screen.text[6][2], " ---------- ");
	strcpy(screen.text[7], "F3-ENTER F4-DOWN");
																 
  if(action==LCM_MENU_MOVE_DOWN)
    if(screen.main_menu_current < menu_actions_amount[screen.current_screen])
           screen.main_menu_current++;
      else screen.main_menu_current=0;

  if(action==LCM_MENU_MOVE_UP)
    if(screen.main_menu_current > 0)
           screen.main_menu_current--;
      else screen.main_menu_current=menu_actions_amount[screen.current_screen];

  if(screen.main_menu_current < screen.main_menu_start)
    screen.main_menu_start = screen.main_menu_current;
  else if(screen.main_menu_current - screen.main_menu_start >= 4)
    screen.main_menu_start = screen.main_menu_current - 3;

  if((action==LCM_MENU_MOVE_DOWN)||(action==LCM_MENU_MOVE_UP))
  for(i = screen.main_menu_start; i < screen.main_menu_start+4; i++)
    sprintf(screen.text[5][3], "%c%s", marker[i-screen.main_menu_start],
                                       menu_tpl[screen.current_screen][i]);

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
  if(screen.main_scr_sett>4) screen.main_scr_sett %= 5;
    
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
*///---------------------------------------------------------------
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
/*
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
/*
SECURITY----------
| MODBUS GATEWAY |
|HTTP control  ON|
|IP Access CL>OFF|
|MAX TCP clients |
|per COM port   2|
|Watchdog Tmr OFF|
|Error Buzzer OFF|
|F1-HELP  F2-BACK|
------------------
LCM_SCREEN_SECURITY
*///---------------------------------------------------------------
void show_security_settings()
  {
  static char marker[5], param_val[5][3];

  if((screen.secr_scr_mode<1)||(screen.secr_scr_mode>5))
    screen.secr_scr_mode=screen.secr_scr_mode%5+1;
    
  marker[0]=' '; marker[1]=' '; marker[2]=' '; marker[3]=' '; marker[4]=' ';
	marker[screen.secr_scr_mode-1]='>';
	
	strcpy (param_val[0], " ON");
	strcpy (param_val[1], "OFF");
	sprintf(param_val[2], "  %d", screen.max_tcp_clients_per_com);
	if(screen.watch_dog_control) strcpy (param_val[3], " ON");
	  else strcpy (param_val[3], "OFF");
	if(screen.buzzer_control) strcpy (param_val[4], " ON");
	  else strcpy (param_val[4], "OFF");
  	
	strcpy (screen.text[0], " MODBUS GATEWAY ");
	sprintf(screen.text[1], "HTTP control%c%s", marker[0], param_val[0]);
	sprintf(screen.text[2], "IP Access CL%c%s", marker[1], param_val[1]);
	strcpy (screen.text[3], "MAX TCP clients ");
	sprintf(screen.text[4], "per COM port%c%s", marker[2], param_val[2]);
	sprintf(screen.text[5], "Watchdog Tmr%c%s", marker[3], param_val[3]);
	sprintf(screen.text[6], "Error Buzzer%c%s", marker[4], param_val[4]);
	strcpy (screen.text[7], "F1-HELP  F2-BACK");

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
/*
SECURITY HELP-----
| MODBUS GATEWAY |
|F1-Display help |
|F2-Go to menu   |
|F3-Change       |
|  selected value|
|F4-Move cursor  |
|F5-Backlight Ctl|
|         F2-BACK|
------------------
LCM_SCREEN_HELP_SECURITY
*///---------------------------------------------------------------
void show_security_help_screen()
  {
	strcpy(screen.text[0], " MODBUS GATEWAY ");
	strcpy(screen.text[1], "F1-Display help ");
	strcpy(screen.text[2], "F2-Go to menu   ");
	strcpy(screen.text[3], "F3-Change       ");
	strcpy(screen.text[4], "  selected value");
	strcpy(screen.text[5], "F4-Move cursor  ");
	strcpy(screen.text[6], "F5-Backlight Ctl");
	strcpy(screen.text[7], "         F2-BACK");

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }
/*
MENU HELP---------
| MODBUS GATEWAY |
|F1-Display help |
|F2-Move up      |
|F3-Select       |
|   menu item    |
|F4-Move down    |
|F5-Backlight Ctl|
|         F2-BACK|
------------------
LCM_SCREEN_HELP_MENU
*///---------------------------------------------------------------
void show_menu_help_screen()
  {
	strcpy(screen.text[0], " MODBUS GATEWAY ");
	strcpy(screen.text[1], "F1-Display help ");
	strcpy(screen.text[2], "F2-Move up      ");
	strcpy(screen.text[3], "F3-Select       ");
	strcpy(screen.text[4], "   menu item    ");
	strcpy(screen.text[5], "F4-Move down    ");
	strcpy(screen.text[6], "F5-Backlight Ctl");
	strcpy(screen.text[7], "         F2-BACK");

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }
/*
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
	strcpy(screen.text[0], " MODBUS GATEWAY ");
	strcpy(screen.text[1], "                ");
	strcpy(screen.text[2], "     SORRY      ");
	strcpy(screen.text[3], "  THIS FEATURE  ");
	strcpy(screen.text[4], "     ISN'T      ");
	strcpy(screen.text[5], "  IMPLEMENTED   ");
	strcpy(screen.text[6], "                ");
	strcpy(screen.text[7], "         F2-BACK");

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }
/*
MENU--------------
| MODBUS GATEWAY |
| Port1 Port7    |
|<Port2>Port8    |
| Port3 LAN Sett |
| Port4 Security |
| Port5 Shutdown |
| Port6 Exit     |
|F1-HELP  F4-DOWN|
------------------
LCM_SCREEN_MENU
*///---------------------------------------------------------------
void show_menu_screen()
  {
  static char marker[3][6], i;

  if((screen.menu_scr_mode<1)||(screen.menu_scr_mode>12))
    screen.menu_scr_mode=screen.menu_scr_mode%12+1;
    
  for(i=0; i<18; i++) marker[i/6][i%6]=' ';

  if(screen.menu_scr_mode<=6) {
    marker[0][screen.menu_scr_mode-1]='<';
    marker[1][screen.menu_scr_mode-1]='>';
    } else {
	    marker[1][screen.menu_scr_mode-7]='<';
	    marker[2][screen.menu_scr_mode-7]='>';
      }
  	
	strcpy (screen.text[0], " MODBUS GATEWAY ");
	sprintf(screen.text[1], "%cPort1%cPort7%c   ", marker[0][0], marker[1][0], marker[2][0]);
	sprintf(screen.text[2], "%cPort2%cPort8%c   ", marker[0][1], marker[1][1], marker[2][1]);
	sprintf(screen.text[3], "%cPort3%cLAN Sett%c", marker[0][2], marker[1][2], marker[2][2]);
	sprintf(screen.text[4], "%cPort4%cSecurity%c", marker[0][3], marker[1][3], marker[2][3]);
	sprintf(screen.text[5], "%cPort5%cShutdown%c", marker[0][4], marker[1][4], marker[2][4]);
	sprintf(screen.text[6], "%cPort6%cExit%c    ", marker[0][5], marker[1][5], marker[2][5]);
	strcpy (screen.text[7], "F1-HELP  F4-DOWN");

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }
///---------------------------------------------------------------
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

int ctrl_change_security_settings()
  {
   
  return 0;	
  }

/*
-----------------
|LOG EVENTS <ALL>|
|1234 INF   PORT1|
|03DEC10 14:32:00|
|- COMMAND LINE P|
|ARSED SUCCESSFUL|
|LY              |
|                |
|F2-BACK  F4-NEXT|
-----------------
F1-HELP (Контекстная справка)
F5-OPTIONS (Порядок сортировки)

*/