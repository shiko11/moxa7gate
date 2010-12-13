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
unsigned int i, j, k, menu_prev;

///=== HMI_KLB_H private functions

///---------------------------------------------------------------
int init_hmi_klb_h()
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
	screen.prev_screen=   LCM_MAIN_IFRTU1;

  screen.main_scr_rtu = GATEWAY_P1;      // LCM_MAIN_IFRTU1, LCM_MAIN_IFRTU2
  screen.main_tcp_start=0;    // LCM_MAIN_IFTCP
  screen.main_tcp_current=0;  // LCM_MAIN_IFTCP
  screen.main_scr_eventlog=0; // LCM_MAIN_EVENTLOG
  screen.main_scr_sett=0;     // LCM_MAIN_SETTINGS

  screen.main_menu_start=0;   // LCM_MAIN_MENU
  screen.main_menu_current=0; // LCM_MAIN_MENU

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

int clear_hmi_klb_h()
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
	  if(	screen.current_screen==LCM_MAIN_IFRTU1	 ||
	  		screen.current_screen==LCM_MAIN_IFRTU2	 ||
	  		screen.current_screen==LCM_MAIN_IFTCP		 ||
	  		screen.current_screen==LCM_MAIN_MOXAGATE ||
	  		screen.current_screen==LCM_MAIN_EVENTLOG ||
	  		screen.current_screen==LCM_MAIN_SYSINFO  ||
	  		screen.current_screen==LCM_MAIN_SETTINGS ||
	  		screen.current_screen==LCM_MAIN_HELP     ||
	  		screen.current_screen==LCM_NOT_IMPLEMENTED
	  		) process_key_main(i);
    else
    if(screen.current_screen==LCM_MAIN_MENU ||
       screen.current_screen==LCM_MAIN_ABOUT
      ) process_key_menu(i);
	  else
	  if( screen.current_screen==LCM_SETT_PORT1 ||
        screen.current_screen==LCM_SETT_PORT2 ||
        screen.current_screen==LCM_SETT_PORT3 ||
        screen.current_screen==LCM_SETT_PORT4 ||
        screen.current_screen==LCM_SETT_PORT5 ||
        screen.current_screen==LCM_SETT_PORT6 ||
        screen.current_screen==LCM_SETT_PORT7 ||
        screen.current_screen==LCM_SETT_PORT8
      ) process_key_settings(i);
    }

    if(!(
        mxlcm_handle==-1 ||
        mxkpd_handle==-1 ||
        mxbzr_handle==-1
      ))
	  if(	screen.current_screen >= LCM_MAIN_IFRTU1  &&
	  		screen.current_screen <= LCM_NOT_IMPLEMENTED
	  		) show_screen(screen.current_screen);

    // refresh_shm(&IfaceRTU); // необходимо заменить явный перенос значений вызовами специальных фнукций
    usleep(LCM_SCREEN_UPDATE_RATE);

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

		case LCM_MAIN_HELP:     show_main_help();     break;
		case LCM_MAIN_MENU:     show_main_menu();     break;
		case LCM_MAIN_ABOUT:    show_about_screen();  break;

    case LCM_NOT_IMPLEMENTED: show_stub_screen(); break;

    case LCM_SETT_PORT1:
    case LCM_SETT_PORT2:
    case LCM_SETT_PORT3:
    case LCM_SETT_PORT4:
    case LCM_SETT_PORT5:
    case LCM_SETT_PORT6:
    case LCM_SETT_PORT7:
    case LCM_SETT_PORT8:
      show_uart_settings(display-LCM_SETT_PORT1);
      break;

		default: return;
	  }

//	if(display==LCM_SCREEN_HELP_MAIN ||
//	   display==LCM_CONFIRM_RESETALL ||
//	   (display==LCM_SCREEN_MENU &&
//	    (screen.current_screen==LCM_SCREEN_MAIN || screen.current_screen==LCM_SCREEN_MAIN2))
//	  ) screen.prev_screen=screen.current_screen;

  //screen.current_screen=display;
  return;
  }
///---------------------------------------------------------------
void process_key_main(int key)
  {
  	
  switch(key) {

  	case KEY_F1: // вывод контекстной справки
      if(screen.current_screen==LCM_NOT_IMPLEMENTED) break;
      if(screen.current_screen == LCM_MAIN_HELP) {

        screen.current_screen=screen.prev_screen;

        screen.prev_screen=\
          screen.prev_screen==LCM_MAIN_MENU?
                              menu_prev:
                              LCM_MAIN_HELP;

        if(screen.prev_screen!=LCM_MAIN_HELP) 
          show_screen(menu_prev); // заполнение фона экрана меню

        } else {
          screen.prev_screen=screen.current_screen;
          screen.current_screen=LCM_MAIN_HELP;
          }
  		break;

  	case KEY_F2: // выполняем переход к следующему экрану уровня MAIN
      if((screen.current_screen==LCM_MAIN_HELP) ||
         (screen.current_screen==LCM_NOT_IMPLEMENTED)
        ) break;
      screen.prev_screen=screen.current_screen;
      screen.current_screen = screen.current_screen >= LCM_MAIN_SETTINGS ?
                              LCM_MAIN_IFRTU1 : screen.current_screen+1;
  		break;

  	case KEY_F3: // выполняем переход к ассоциированному экрану уровня STAT

      if(screen.current_screen==LCM_NOT_IMPLEMENTED) {
        screen.current_screen=screen.prev_screen;
        break;
        }

      if(screen.current_screen==LCM_MAIN_SETTINGS  ) {
        screen.prev_screen=screen.current_screen;
        screen.current_screen=LCM_NOT_IMPLEMENTED;
        }

      if(((screen.current_screen==LCM_MAIN_IFRTU1) || (screen.prev_screen==LCM_MAIN_IFRTU2))) {
        screen.prev_screen = screen.current_screen;
        screen.current_screen = LCM_SETT_PORT1+screen.main_scr_rtu;
        }

  		break;

  	case KEY_F4: // перемещаем курсор текущего экрана на одну позицию
      if(screen.current_screen==LCM_MAIN_IFRTU1)   screen.main_scr_rtu++;
      if(screen.current_screen==LCM_MAIN_IFRTU2)   screen.main_scr_rtu++;
      if(screen.current_screen==LCM_MAIN_IFTCP)    screen.main_tcp_current++;
      if(screen.current_screen==LCM_MAIN_EVENTLOG) screen.main_scr_eventlog++;
      if(screen.current_screen==LCM_MAIN_SETTINGS) screen.main_scr_sett++;
  		break;

  	case KEY_F5: // выводим меню со списком действий для конкретного экрана
      if((screen.current_screen>=LCM_MAIN_IFRTU1) &&
         (screen.current_screen<=LCM_MAIN_SETTINGS)
        ) {
        screen.prev_screen=screen.current_screen;
        screen.current_screen=LCM_MAIN_MENU;
        screen.main_menu_action=LCM_MENU_NOACTION;
        screen.main_menu_start = screen.main_menu_current = 0;
        }
  		break;

  	default:;    ///------------------
    };

  return;
  }
///---------------------------------------------------------------
void process_key_menu(int key)
  {
  	
  switch(key) {

  	case KEY_F1: // выводим контекстную справку по экрану меню
      if(screen.current_screen==LCM_MAIN_ABOUT) break;
      menu_prev=screen.prev_screen;
      screen.prev_screen=screen.current_screen;
      screen.current_screen=LCM_MAIN_HELP;
  		break;

  	case KEY_F2: // перемещаем курсор в меню на позицию вверх
      if(screen.current_screen==LCM_MAIN_ABOUT) break;
      screen.main_menu_action=LCM_MENU_MOVE_UP;
  		break;

  	case KEY_F3: ///------------------

      if(screen.current_screen==LCM_MAIN_ABOUT) break;

      if(screen.main_menu_current==0) { // PGUP
        screen.current_screen = screen.prev_screen == LCM_MAIN_IFRTU1 ?
                                LCM_MAIN_SETTINGS : screen.prev_screen-1;
        screen.prev_screen=LCM_MAIN_MENU;
        break;
        }

      if(screen.main_menu_current==1) { // HOME
        screen.current_screen = LCM_MAIN_IFRTU1;
        screen.prev_screen=LCM_MAIN_MENU;
        break;
        }

      if(screen.main_menu_current==3) { // ABOUT
        screen.current_screen = LCM_MAIN_ABOUT;
        break;
        }

      if((screen.prev_screen==LCM_MAIN_SETTINGS) &&
         (screen.main_menu_current==2)
        ) {
        screen.current_screen = LCM_NOT_IMPLEMENTED;
        break;
        }

      if(((screen.prev_screen==LCM_MAIN_IFRTU1) || (screen.prev_screen==LCM_MAIN_IFRTU2)) &&
         (screen.main_menu_current==4)
        ) {
        //screen.prev_screen = screen.current_screen;
        screen.current_screen = LCM_SETT_PORT1+screen.main_scr_rtu;
        break;
        }

  		break;

  	case KEY_F4: // перемещаем курсор в меню на позицию вниз
      if(screen.current_screen==LCM_MAIN_ABOUT) break;
      screen.main_menu_action=LCM_MENU_MOVE_DOWN;
  		break;

  	case KEY_F5: ///------------------
      screen.current_screen=screen.prev_screen;
      screen.prev_screen=LCM_MAIN_MENU;
  		break;

  	default:;    ///------------------
    };

  return;
  }
///----------------------------------------------------------------------------
void process_key_settings(int key)
  {

  switch(key) {

  	case KEY_F1: ///------------------
  		break;

  	case KEY_F2: ///------------------
      screen.current_screen=screen.prev_screen;
  		break;

  	case KEY_F3: ///------------------
  		break;

  	case KEY_F4: ///------------------
  		break;

  	case KEY_F5: ///------------------
  		break;

  	default:;    ///------------------
    };

  return;
  }
///----------------------------------------------------------------------------
