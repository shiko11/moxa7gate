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

//unsigned int buzzer_flag; // зуммер дает 1, 2 и 3 гудка в зависимости от количества ошибок: <15, 15-30, >30
//struct timeval tv;

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

	screen.current_screen=LCM_MAIN_MOXAGATE;
	screen.prev_screen=   LCM_MAIN_MOXAGATE;

  screen.main_scr_rtu = GATEWAY_P1;      // LCM_MAIN_IFRTU1, LCM_MAIN_IFRTU2
  screen.main_tcp_start=0;    // LCM_MAIN_IFTCP
  screen.main_tcp_current=0;  // LCM_MAIN_IFTCP
  screen.main_scr_eventlog=0; // LCM_MAIN_EVENTLOG

  screen.back_light=1;
  //screen.buzzer_control=1;

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

int clear_hmi_klb_h()
  {

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
  int i, static_display=0;
  unsigned int old_display;
	struct timeval tv, tvlcm;
	struct timezone tz;
  
  gettimeofday(&tvlcm, &tz);

  while(1) {

		gettimeofday(&tv, &tz);
		old_display=screen.current_screen;

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
	  		screen.current_screen==LCM_MAIN_HELP     ||
        screen.current_screen==LCM_MAIN_ABOUT    ||
        screen.current_screen==LCM_CONFIRM_HALT
	  		) process_key_main(i);
	  else
	  if( (screen.current_screen>=LCM_SETT_PORT1 && screen.current_screen<=LCM_SETT_PORT8) ||
	      (screen.current_screen>=LCM_STAT_PORT1 && screen.current_screen<=LCM_STAT_PORT8) ||
	      (screen.current_screen==LCM_CONFIRM_RESET_RTU)
      ) process_key_rtu_details(i);
	  else
	  if( (screen.current_screen==LCM_STAT_LANTCP) ||
	      (screen.current_screen==LCM_SETT_LANTCP) ||
	      (screen.current_screen==LCM_CONFIRM_RESET_TCP)
      ) process_key_tcp_details(i);
	  else
	  if( (screen.current_screen==LCM_STAT_MOXAGATE) ||
	      (screen.current_screen==LCM_SETT_MOXAGATE) ||
	      (screen.current_screen==LCM_CONFIRM_RESET_M7G)
      ) process_key_moxagate(i);
    else
    if(screen.current_screen==LCM_EVENTLOG_DETAILS) process_key_event(i);
    
		if(Security.use_buzzer==1) mxbuzzer_beep(mxbzr_handle, 80);
    }

		// с установленной периодичностью обновляем данные на экране LCM
    if( (((tv.tv_sec-tvlcm.tv_sec)*1000000+(tv.tv_usec-tvlcm.tv_usec)) >= LCM_SCREEN_UPDATE_RATE) ||
        (old_display!=screen.current_screen)
      ) {

			if(old_display!=screen.current_screen) static_display=0;

	    if(!(
	        mxlcm_handle==-1 ||
	        mxkpd_handle==-1 ||
	        mxbzr_handle==-1
	      )) if(static_display==0) show_screen(screen.current_screen);
	
	    static_display= \
				(screen.current_screen==LCM_MAIN_MOXAGATE) || \
				(screen.current_screen>=LCM_SETT_PORT1 && screen.current_screen<=LCM_SETT_PORT8) \
				?1:0;

			gettimeofday(&tvlcm, &tz);
      }

    // обновляем динамические данные для web-интерфейса
    refresh_shm();
    // обновляем динамические данные блока диагностики шлюза
    refresh_status_info();
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

		case LCM_MAIN_HELP:     show_main_help();     break;
		case LCM_MAIN_ABOUT:    show_about_screen();  break;
		case LCM_CONFIRM_HALT:  show_confirm_halt();  break;

    case LCM_STAT_PORT1:
    case LCM_STAT_PORT2:
    case LCM_STAT_PORT3:
    case LCM_STAT_PORT4:
    case LCM_STAT_PORT5:
    case LCM_STAT_PORT6:
    case LCM_STAT_PORT7:
    case LCM_STAT_PORT8:
      show_ifrtu_statistics(display-LCM_STAT_PORT1);
      break;

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

    case LCM_CONFIRM_RESET_RTU:
    case LCM_CONFIRM_RESET_TCP:
    case LCM_CONFIRM_RESET_M7G:
    	show_confirm_reset_counters();
    	break;

    case LCM_STAT_LANTCP: show_iftcp_statistics(); break;
    case LCM_SETT_LANTCP: show_lantcp_settings(); break;

    case LCM_STAT_MOXAGATE: show_stat_moxagate(); break;
    case LCM_SETT_MOXAGATE: show_sett_moxagate(); break;

    case LCM_EVENTLOG_DETAILS: show_log_event();  break;

		default: return;
	  }

  return;
  }
///---------------------------------------------------------------
void process_key_main(int key)
  {
  	
  switch(key) {

  	case KEY_F1: // вывод контекстной справки

  	  //*((int *)(0))=1; // вызывается фатальная ошибка для проверки watch-dog таймера

      if((screen.current_screen==LCM_MAIN_ABOUT)||
         (screen.current_screen==LCM_CONFIRM_HALT)) break;
      if(screen.current_screen == LCM_MAIN_HELP) {
        screen.current_screen=screen.prev_screen;
        screen.prev_screen=LCM_MAIN_HELP;
        } else {
          screen.prev_screen=screen.current_screen;
          screen.current_screen=LCM_MAIN_HELP;
          }
  		break;

  	case KEY_F2: // выполняем переход к следующему экрану уровня MAIN
      if((screen.current_screen==LCM_MAIN_HELP) ||
         (screen.current_screen==LCM_MAIN_ABOUT)
        ) break;
      if(screen.current_screen==LCM_CONFIRM_HALT) {screen.current_screen=screen.prev_screen; break;}
      screen.prev_screen=screen.current_screen;
      screen.current_screen = screen.current_screen >= LCM_MAIN_EVENTLOG ?
                              LCM_MAIN_MOXAGATE : screen.current_screen+1;
  		break;

  	case KEY_F3: // выполняем переход к ассоциированному экрану уровня STAT
      if(screen.current_screen==LCM_CONFIRM_HALT) break;

      if(screen.current_screen==LCM_MAIN_HELP) {
      	screen.current_screen=LCM_MAIN_ABOUT;
      	break;
        }

      if(screen.current_screen==LCM_MAIN_ABOUT) {
        screen.current_screen=screen.prev_screen;
        screen.prev_screen=LCM_MAIN_ABOUT;
      	break;
      	}

      if((screen.current_screen==LCM_MAIN_IFRTU1) || (screen.current_screen==LCM_MAIN_IFRTU2)) {
        screen.prev_screen = screen.current_screen;
        screen.current_screen = LCM_STAT_PORT1+screen.main_scr_rtu;
        break;
        }

      if(screen.current_screen==LCM_MAIN_IFTCP) {
        screen.prev_screen = screen.current_screen;
        screen.current_screen = LCM_STAT_LANTCP;
        break;
        }

      if(screen.current_screen==LCM_MAIN_MOXAGATE) {
        screen.prev_screen = screen.current_screen;
        screen.current_screen = LCM_STAT_MOXAGATE;
        break;
        }

      if(screen.current_screen==LCM_MAIN_EVENTLOG) {
        screen.prev_screen=screen.current_screen;
        screen.current_screen=LCM_EVENTLOG_DETAILS;
        screen.eventlog_current = EventLog.app_log_current_entry==0 ?
                                  EVENT_LOG_LENGTH-1 : EventLog.app_log_current_entry-1;
        break;
        }

  		break;

  	case KEY_F4: // выполняем переход к предыдущему экрану уровня MAIN
      if((screen.current_screen==LCM_MAIN_HELP) ||
         (screen.current_screen==LCM_MAIN_ABOUT)
        ) break;
      if(screen.current_screen==LCM_CONFIRM_HALT) {
      	screen.current_screen=LCM_MAIN_ABOUT;
      	screen.prev_screen=   LCM_MAIN_ABOUT;
      	show_screen(screen.current_screen);
      	Security.halt=1;
      	break;
      	}
      screen.prev_screen=screen.current_screen;
      screen.current_screen = screen.current_screen <= LCM_MAIN_MOXAGATE ?
                              LCM_MAIN_EVENTLOG : screen.current_screen-1;
  		break;

  	case KEY_F5: // перемещаем курсор текущего экрана на одну позицию
      if(screen.current_screen==LCM_CONFIRM_HALT) break;
      if(screen.current_screen==LCM_MAIN_IFRTU1)   screen.main_scr_rtu++;
      if(screen.current_screen==LCM_MAIN_IFRTU2)   screen.main_scr_rtu++;
      if(screen.current_screen==LCM_MAIN_IFTCP)    screen.main_tcp_current++;
      if(screen.current_screen==LCM_MAIN_EVENTLOG) screen.main_scr_eventlog++;
      if(screen.current_screen==LCM_MAIN_MOXAGATE) {
	      screen.prev_screen=screen.current_screen;
	      screen.current_screen = LCM_CONFIRM_HALT;
        }
      if((screen.current_screen==LCM_MAIN_HELP) || (screen.current_screen==LCM_MAIN_ABOUT)) {
				if((screen.back_light==1)) mxlcm_back_light_off(mxlcm_handle);
				  else mxlcm_back_light_on(mxlcm_handle);
				screen.back_light=(screen.back_light+1)%2;
        }
  		break;

  	default:;
    };

  return;
  }
///----------------------------------------------------------------------------
void process_key_rtu_details(int key)
  {

  switch(key) {

  	case KEY_F1: ///------------------
      if(screen.current_screen == LCM_MAIN_HELP) {
        screen.current_screen=screen.prev_screen;
        screen.prev_screen=LCM_MAIN_HELP;
        } else if(screen.current_screen>=LCM_STAT_PORT1 && screen.current_screen<=LCM_STAT_PORT8) {
          screen.prev_screen=screen.current_screen;
          screen.current_screen=LCM_MAIN_HELP;
          }
  		break;

  	case KEY_F2: ///------------------
  	  if( (screen.current_screen>=LCM_SETT_PORT1 && screen.current_screen<=LCM_SETT_PORT8) ||
  	      (screen.current_screen==LCM_CONFIRM_RESET_RTU)
  	    ) {
        screen.current_screen = screen.prev_screen;
        break;
  	    }

  	  if(screen.current_screen>=LCM_STAT_PORT1 && screen.current_screen<=LCM_STAT_PORT8) {
				if(screen.main_scr_rtu<3)
          screen.current_screen = LCM_MAIN_IFRTU1;
          else screen.current_screen = LCM_MAIN_IFRTU2;
  	    }
  		break;

  	case KEY_F3: ///------------------

      if(screen.current_screen==LCM_MAIN_HELP) {
      	screen.current_screen=LCM_MAIN_ABOUT;
      	break;
        }

      if(screen.current_screen==LCM_MAIN_ABOUT) {
        screen.current_screen=screen.prev_screen;
        screen.prev_screen=LCM_MAIN_ABOUT;
      	break;
      	}

  	  if((screen.current_screen>=LCM_STAT_PORT1 && screen.current_screen<=LCM_STAT_PORT8)) {
        screen.prev_screen = screen.current_screen;
        screen.current_screen = LCM_SETT_PORT1 + (screen.current_screen - LCM_STAT_PORT1);
        break;
  	    }
  		break;

  	case KEY_F4: ///------------------
  	  if((screen.current_screen>=LCM_STAT_PORT1 && screen.current_screen<=LCM_STAT_PORT8)) {
        screen.prev_screen = screen.current_screen;
        screen.current_screen = LCM_CONFIRM_RESET_RTU;
        break;
  	    }
  	  if(screen.current_screen==LCM_CONFIRM_RESET_RTU) {
  	  	clear_stat(&IfaceRTU[screen.prev_screen - LCM_STAT_PORT1].stat);
        screen.current_screen = screen.prev_screen;
        break;
  	    }
  		break;

  	case KEY_F5: ///------------------
      if((screen.current_screen==LCM_MAIN_HELP) || (screen.current_screen==LCM_MAIN_ABOUT)) {
				if((screen.back_light==1)) mxlcm_back_light_off(mxlcm_handle);
				  else mxlcm_back_light_on(mxlcm_handle);
				screen.back_light=(screen.back_light+1)%2;
        }
  		break;

  	default:;    ///------------------
    };

  return;
  }
///----------------------------------------------------------------------------
void process_key_tcp_details(int key)
  {

  switch(key) {

  	case KEY_F1: ///------------------
  		break;

  	case KEY_F2: ///------------------
  	  if(screen.current_screen==LCM_SETT_LANTCP) {
        screen.current_screen=LCM_STAT_LANTCP;
        break;
  	    }
  	  if(screen.current_screen==LCM_STAT_LANTCP) {
        screen.current_screen=LCM_MAIN_IFTCP;
        break;
  	    }
  		break;

  	case KEY_F3: ///------------------
  	  if(screen.current_screen==LCM_STAT_LANTCP) {
        screen.current_screen=LCM_SETT_LANTCP;
        break;
  	    }
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
void process_key_moxagate(int key)
  {

  switch(key) {

  	case KEY_F1: ///------------------
  		break;

  	case KEY_F2: ///------------------
  	  if(screen.current_screen==LCM_SETT_MOXAGATE) {
        screen.current_screen=LCM_STAT_MOXAGATE;
        break;
  	    }
  	  if(screen.current_screen==LCM_STAT_MOXAGATE) {
        screen.current_screen=LCM_MAIN_MOXAGATE;
        break;
  	    }
  		break;

  	case KEY_F3: ///------------------
  	  if(screen.current_screen==LCM_STAT_MOXAGATE) {
        screen.current_screen=LCM_SETT_MOXAGATE;
        break;
  	    }
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
void process_key_event(int key)
  {

  switch(key) {

  	case KEY_F1: ///------------------
  		break;

  	case KEY_F2: ///------------------
      screen.eventlog_current=\
			  screen.eventlog_current==0?\
			  EventLog.app_log_current_entry-1:screen.eventlog_current-1;
  		break;

  	case KEY_F3: ///------------------
      screen.current_screen=screen.prev_screen;
  		break;

  	case KEY_F4: ///------------------
      screen.eventlog_current=\
			  screen.eventlog_current==EventLog.app_log_current_entry-1?\
			  0:screen.eventlog_current+1;
  		break;

  	case KEY_F5: ///------------------
  		break;

  	default:;    ///------------------
    };

  return;
  }
///----------------------------------------------------------------------------
