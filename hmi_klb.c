/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///**** МОДУЛЬ МОНИТОРИНГА И УПРАВЛЕНИЯ РАБОТОЙ ШЛЮЗА СРЕДСТВАМИ KEYPAD-LCM ****

///=== HMI_KEYPAD_LCM_H IMPLEMENTATION

#include <pthread.h>

#include "hmi_klb.h"
#include "hmi_web.h"
#include "interfaces.h"
#include "moxagate.h"
#include "messages.h"

///=== HMI_KEYPAD_LCM_H private variables

int rc;
pthread_t tstTH;

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

	screen.current_screen=LCM_SCREEN_MAIN;

  screen.main_scr_mode=1;
  screen.menu_scr_mode=1;
  screen.secr_scr_mode=1;

  screen.back_light=1;
  screen.max_tcp_clients_per_com=8;
  screen.watch_dog_control=0;
  screen.buzzer_control=1;
  screen.secr_scr_changes_was_made=0;

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
	  if(	screen.current_screen==LCM_SCREEN_MAIN				||
	  		screen.current_screen==LCM_SCREEN_MAIN2				||
	  		screen.current_screen==LCM_SCREEN_HELP_MAIN		||
	  		screen.current_screen==LCM_SCREEN_ABOUT				||
	  		screen.current_screen==LCM_SCREEN_SYSINFO			||
	  		screen.current_screen==LCM_SCREEN_PORT1_STAT	||
	  		screen.current_screen==LCM_SCREEN_PORT2_STAT	||
	  		screen.current_screen==LCM_SCREEN_PORT3_STAT	||
	  		screen.current_screen==LCM_SCREEN_PORT4_STAT	||
	  		screen.current_screen==LCM_SCREEN_PORT5_STAT	||
	  		screen.current_screen==LCM_SCREEN_PORT6_STAT	||
	  		screen.current_screen==LCM_SCREEN_PORT7_STAT	||
	  		screen.current_screen==LCM_SCREEN_PORT8_STAT	||
	  		screen.current_screen==LCM_CONFIRM_RESETALL
	  		) process_key_main(i);
	  else
	  if(	screen.current_screen==LCM_SCREEN_MENU				||
	  		screen.current_screen==LCM_SCREEN_HELP_MENU		||
	  		screen.current_screen==LCM_SCREEN_PORT1				||
	  		screen.current_screen==LCM_SCREEN_PORT2				||
	  		screen.current_screen==LCM_SCREEN_PORT3				||
	  		screen.current_screen==LCM_SCREEN_PORT4				||
	  		screen.current_screen==LCM_SCREEN_PORT5				||
	  		screen.current_screen==LCM_SCREEN_PORT6				||
	  		screen.current_screen==LCM_SCREEN_PORT7				||
	  		screen.current_screen==LCM_SCREEN_PORT8 			||
	  		screen.current_screen==LCM_SCREEN_LAN					||
	  		screen.current_screen==LCM_CONFIRM_REBOOT
	  		) process_key_menu(i);
	  else
	  if(	screen.current_screen==LCM_SCREEN_SECURITY			||
	  		screen.current_screen==LCM_SCREEN_HELP_SECURITY	||
	  		screen.current_screen==LCM_SCREEN_STUB_SECURITY ||
	  		screen.current_screen==LCM_CONFIRM_SECR_CHANGES
	  		) process_key_security(i);
    }

		refresh_shm(&IfaceRTU);
	  usleep(LCM_SCREEN_UPDATE_RATE);

    if(!(
        mxlcm_handle==-1 ||
        mxkpd_handle==-1 ||
        mxbzr_handle==-1
      ))
	  if(	screen.current_screen==LCM_SCREEN_MAIN				||
	  		screen.current_screen==LCM_SCREEN_MAIN2				||
	  		screen.current_screen==LCM_SCREEN_SYSINFO			||
	  		screen.current_screen==LCM_SCREEN_PORT1_STAT	||
	  		screen.current_screen==LCM_SCREEN_PORT2_STAT	||
	  		screen.current_screen==LCM_SCREEN_PORT3_STAT	||
	  		screen.current_screen==LCM_SCREEN_PORT4_STAT	||
	  		screen.current_screen==LCM_SCREEN_PORT5_STAT	||
	  		screen.current_screen==LCM_SCREEN_PORT6_STAT	||
	  		screen.current_screen==LCM_SCREEN_PORT7_STAT	||
	  		screen.current_screen==LCM_SCREEN_PORT8_STAT
	  		) show_screen(screen.current_screen);
	  }

  return;
  }
///---------------------------------------------------------------
void show_screen(int display)
  {
	switch(display) {
		case LCM_SCREEN_MAIN:			show_main_screen(); break;
		case LCM_SCREEN_MAIN2:		show_main_screen2(); break;
		case LCM_SCREEN_LAN:			show_inet_settings(); break;
		case LCM_SCREEN_PORT1:
		case LCM_SCREEN_PORT2:
		case LCM_SCREEN_PORT3:
		case LCM_SCREEN_PORT4:
		case LCM_SCREEN_PORT5:
		case LCM_SCREEN_PORT6:
		case LCM_SCREEN_PORT7:
		case LCM_SCREEN_PORT8: 		show_uart_settings(display-LCM_SCREEN_PORT1); break;
		case LCM_SCREEN_SECURITY:	show_security_settings(); break;
		case LCM_SCREEN_SYSINFO:	show_system_info(); break;
		case LCM_SCREEN_PORT1_STAT:
		case LCM_SCREEN_PORT2_STAT:
		case LCM_SCREEN_PORT3_STAT:
		case LCM_SCREEN_PORT4_STAT:
		case LCM_SCREEN_PORT5_STAT:
		case LCM_SCREEN_PORT6_STAT:
		case LCM_SCREEN_PORT7_STAT:
		case LCM_SCREEN_PORT8_STAT: 		show_uart_detail(display-LCM_SCREEN_PORT1_STAT); break;
		case LCM_SCREEN_HELP_MAIN:			show_main_help_screen(); break;
		case LCM_SCREEN_HELP_SECURITY:	show_security_help_screen(); break;
		case LCM_SCREEN_HELP_MENU:			show_menu_help_screen(); break;
		case LCM_SCREEN_ABOUT:					show_about_screen(); break;
		case LCM_CONFIRM_RESETALL:			show_confirmation_reset_all(); break;
		case LCM_CONFIRM_REBOOT:				show_confirmation_reboot(); break;
		case LCM_SCREEN_STUB_SECURITY:	show_stub_screen(); break;
		case LCM_SCREEN_MENU:						show_menu_screen(); break;
		case LCM_CONFIRM_SECR_CHANGES:	show_confirmation_secr_changes(); break;
		default: return;
	  }

	if(display==LCM_SCREEN_HELP_MAIN ||
	   display==LCM_CONFIRM_RESETALL ||
	   (display==LCM_SCREEN_MENU &&
	    (screen.current_screen==LCM_SCREEN_MAIN || screen.current_screen==LCM_SCREEN_MAIN2))
	  ) screen.prev_screen=screen.current_screen;
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
///---------------------------------------------------------------
/*
EVENTS SCREEN-----
| LOG EVENT #XXX |
|----------------|
|03DEC10 14:32:00|
|                |
|COMMAND LINE PAR|
|SED SUCCESSFULLY|
|                |
|F2-BACK  F4-NEXT|
------------------

MAIN SCREEN-------
| MODBUS GATEWAY |
|----------------|
|SL Msgs Errs MOD|
|P1 0123 4567 OFF|
|P2*8901 2345 08G|
|P3 6789 0123 ERR|
|P4 4567 8901 08M|
|F1-HELP F2-P5678|
------------------
LCM_SCREEN_MAIN
LCM_SCREEN_MAIN2
*///---------------------------------------------------------------
void show_main_screen()
  {
  static char marker[4];

  if((screen.main_scr_mode<1)||(screen.main_scr_mode>4))
    screen.main_scr_mode=screen.main_scr_mode%4+1;
    
  marker[0]=' '; marker[1]=' '; marker[2]=' '; marker[3]=' ';
  marker[screen.main_scr_mode-1]='>';
  
	strcpy (screen.text[0], " MODBUS GATEWAY ");
	strcpy (screen.text[1], "----------------");
	strcpy (screen.text[2], "SL Msgs Errs MOD");
	sprintf(screen.text[3], "P1%c%4.4d %4.4d %s", marker[0], IfaceRTU[GATEWAY_P1].stat.sended%10000, IfaceRTU[GATEWAY_P1].stat.errors%10000, IfaceRTU[GATEWAY_P1].bridge_status);
	sprintf(screen.text[4], "P2%c%4.4d %4.4d %s", marker[1], IfaceRTU[GATEWAY_P2].stat.sended%10000, IfaceRTU[GATEWAY_P2].stat.errors%10000, IfaceRTU[GATEWAY_P2].bridge_status);
	sprintf(screen.text[5], "P3%c%4.4d %4.4d %s", marker[2], IfaceRTU[GATEWAY_P3].stat.sended%10000, IfaceRTU[GATEWAY_P3].stat.errors%10000, IfaceRTU[GATEWAY_P3].bridge_status);
	sprintf(screen.text[6], "P4%c%4.4d %4.4d %s", marker[3], IfaceRTU[GATEWAY_P4].stat.sended%10000, IfaceRTU[GATEWAY_P4].stat.errors%10000, IfaceRTU[GATEWAY_P4].bridge_status);
	strcpy (screen.text[7], "F1-HELP F2-P5678");

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }

void show_main_screen2()
  {
  static char marker[4];

  if((screen.main_scr_mode<5)||(screen.main_scr_mode>8))
    screen.main_scr_mode=screen.main_scr_mode%4+5;
    
  marker[0]=' '; marker[1]=' '; marker[2]=' '; marker[3]=' ';
  marker[screen.main_scr_mode-5]='>';
  	
	strcpy (screen.text[0], " MODBUS GATEWAY ");
	strcpy (screen.text[1], "----------------");
	strcpy (screen.text[2], "SL Msgs Errs MOD");
	sprintf(screen.text[3], "P5%c%4.4d %4.4d %s", marker[0], IfaceRTU[GATEWAY_P5].stat.sended%10000, IfaceRTU[GATEWAY_P5].stat.errors%10000, IfaceRTU[GATEWAY_P5].bridge_status);
	sprintf(screen.text[4], "P6%c%4.4d %4.4d %s", marker[1], IfaceRTU[GATEWAY_P6].stat.sended%10000, IfaceRTU[GATEWAY_P6].stat.errors%10000, IfaceRTU[GATEWAY_P6].bridge_status);
	sprintf(screen.text[5], "P7%c%4.4d %4.4d %s", marker[2], IfaceRTU[GATEWAY_P7].stat.sended%10000, IfaceRTU[GATEWAY_P7].stat.errors%10000, IfaceRTU[GATEWAY_P7].bridge_status);
	sprintf(screen.text[6], "P8%c%4.4d %4.4d %s", marker[3], IfaceRTU[GATEWAY_P8].stat.sended%10000, IfaceRTU[GATEWAY_P8].stat.errors%10000, IfaceRTU[GATEWAY_P8].bridge_status);
	strcpy (screen.text[7], "F1-HELP F2-P1234");

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }
/*
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
/*
SYS INFO----------
|System started  |
|25.04.2009 23:23|
|System uptime   |
|   057d 11h 35m |
|CPU usage 75/50%|
|RAM usage 75/50%|
|FLASH usage  80%|
|F2-BACK F4-ABOUT|
------------------
LCM_SCREEN_SYSINFO
*///---------------------------------------------------------------
void show_system_info()
  {
	struct tm *tmd;
	time_t moment;

//	time_t start_time=IfaceRTU[0].start_time;
//	int i;
//  for(i=1; i<MAX_MOXA_PORTS; i++)
//  	if(start_time>IfaceRTU[i].start_time) start_time=IfaceRTU[i].start_time;

	tmd=gmtime(&MoxaDevice.start_time);

	time(&moment);
	int diff=difftime(moment, MoxaDevice.start_time);
	
	strcpy (screen.text[0], "System started  ");
	sprintf(screen.text[1], "%2.2d.%2.2d.%4.4d %2.2d:%2.2d", tmd->tm_mday, tmd->tm_mon+1, tmd->tm_year+1900, tmd->tm_hour, tmd->tm_min);
	strcpy (screen.text[2], "System uptime   ");
	sprintf(screen.text[3], "   %3.3dd %2.2dh %2.2dm ", (diff/86400)%1000, (diff/3600)%24, (diff/60)%60);
	strcpy (screen.text[4], "CPU usage    00%");
	strcpy (screen.text[5], "RAM usage    00%");
	strcpy (screen.text[6], "FLASH usage  00%");
	strcpy (screen.text[7], "F2-BACK F4-ABOUT");

  mxlcm_write_screen(mxlcm_handle, screen.text);
  return;
  }
/*
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
MAIN SCREEN HELP--
| MODBUS GATEWAY |
|F2- Switch next |
|UART Ports P1-P8|
|F3-Enter detail |
|statistic screen|
|F4-Move cursor  |
|F5-Menu         |
|F2-BACK  F4-MOXA|
------------------
LCM_SCREEN_HELP_MAIN
*///---------------------------------------------------------------
void show_main_help_screen()
  {
	strcpy(screen.text[0], " MODBUS GATEWAY ");
	strcpy(screen.text[1], "F2- Switch next ");
	strcpy(screen.text[2], "UART Ports P1-P8");
	strcpy(screen.text[3], "F3-Enter detail ");
	strcpy(screen.text[4], "statistic screen");
	strcpy(screen.text[5], "F4-Move cursor  ");
	strcpy(screen.text[6], "F5-Menu         ");
	strcpy(screen.text[7], "F2-BACK  F4-MOXA");

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
ABOUT-------------
| MOXA7GATE R1.00|
| Modbus gateway |
|      software  |
|SEM-ENGINEERING |
|+7(4832)41-88-23|
| www.semgroup.ru|
|  Bryansk 2009  |
|         F2-BACK|
------------------
LCM_SCREEN_ABOUT
*///---------------------------------------------------------------
void show_about_screen()
  {

	strcpy(screen.text[0], " MOXA7GATE R1.00");
	strcpy(screen.text[1], " Modbus gateway ");
	strcpy(screen.text[2], "      software  ");
	strcpy(screen.text[3], "SEM-ENGINEERING ");
	strcpy(screen.text[4], "+7(4832)41-88-23");
	strcpy(screen.text[5], " www.semgroup.ru");
	strcpy(screen.text[6], "  Bryansk 2009  ");
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
