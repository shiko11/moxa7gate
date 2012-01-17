/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010                 
***************************************/

#ifndef HMI_KEYPAD_LCM_H
#define HMI_KEYPAD_LCM_H

///**** МОДУЛЬ МОНИТОРИНГА И УПРАВЛЕНИЯ РАБОТОЙ ШЛЮЗА СРЕДСТВАМИ KEYPAD-LCM ****

#include "mxlib/mxkeypad.h"
#include "mxlib/mxlcm.h"
#include "mxlib/mxwdg.h"

///=== HMI_KEYPAD_LCM_H constants

// конфигурационные константы времени компиляции
#define	LCM_SCREEN_UPDATE_RATE 800000

// идентификаторы доступных клавиш KEYPAD
#define KEY_F1 0
#define KEY_F2 1
#define KEY_F3 2
#define KEY_F4 3
#define KEY_F5 4

// главные дисплеи LCM уровня MAIN
#define	LCM_MAIN_MOXAGATE   1
#define	LCM_MAIN_IFRTU1     2
#define	LCM_MAIN_IFRTU2     3
#define	LCM_MAIN_IFTCP      4
#define	LCM_MAIN_EVENTLOG   5

// вспомогательные дисплеи LCM уровня MAIN
#define	LCM_MAIN_HELP       8
#define	LCM_MAIN_ABOUT      9
#define	LCM_CONFIRM_HALT    10

// дисплеи LCM со статистическими данными
#define	LCM_STAT_PORT1    11
#define	LCM_STAT_PORT2    12
#define	LCM_STAT_PORT3    13
#define	LCM_STAT_PORT4    14
#define	LCM_STAT_PORT5    15
#define	LCM_STAT_PORT6    16
#define	LCM_STAT_PORT7    17
#define	LCM_STAT_PORT8    18
#define	LCM_STAT_LANTCP   19
#define	LCM_STAT_MOXAGATE 20

// дисплеи LCM со справочной информацией о настройках
#define	LCM_SETT_PORT1    21
#define	LCM_SETT_PORT2    22
#define	LCM_SETT_PORT3    23
#define	LCM_SETT_PORT4    24
#define	LCM_SETT_PORT5    25
#define	LCM_SETT_PORT6    26
#define	LCM_SETT_PORT7    27
#define	LCM_SETT_PORT8    28
#define	LCM_SETT_LANTCP   29
#define	LCM_SETT_MOXAGATE 30

// дисплеи журнала сообщений
#define LCM_EVENTLOG_DETAILS 31

// подтверждение выполнямых действий
#define	LCM_CONFIRM_RESET_RTU 33
#define	LCM_CONFIRM_RESET_TCP 34
#define	LCM_CONFIRM_RESET_M7G 35

// коды возврата ошибок из функций модуля
#define HMI_KLB_INIT_KEYPAD 62
#define HMI_KLB_INIT_LCM    63
#define HMI_KLB_INIT_BUZZER 61
#define HMI_KLB_INIT_THREAD 64

///=== HMI_KEYPAD_LCM_H data types

typedef struct { // net initsializatsii polej structury

	unsigned int current_screen;
	unsigned int prev_screen;

  unsigned int main_scr_rtu;      // LCM_MAIN_IFRTU1, LCM_MAIN_IFRTU2
  unsigned int main_tcp_start;    // LCM_MAIN_IFTCP
  unsigned int main_tcp_current;  // LCM_MAIN_IFTCP
  unsigned int main_scr_eventlog; // LCM_MAIN_EVENTLOG

  unsigned int eventlog_current;  // LCM_EVENTLOG_DETAILS

  unsigned int back_light;
  //unsigned int buzzer_control;
  
  char text[MAX_LCM_ROWS][MAX_LCM_COLS];

	// так как константа MAX_LCM_COLS содержит количество символов в одной строке LCM,
	// а фактическое количество байт в каждой строке массива должно быть больше на один
	// вследствие использования стандартных функций C++, предполагающих запись нулевого
	// значения в конце каждой строки, вводим буферную переменную для их хранения.
  char buffer[MAX_LCM_ROWS];
  
	} GW_Display;

///=== HMI_KEYPAD_LCM_H public variables
GW_Display screen;

int mxlcm_handle;
int mxkpd_handle;
int mxbzr_handle;

///=== HMI_KEYPAD_LCM_H public functions

int init_hmi_klb_h();  // условно конструктор
int clear_hmi_klb_h(); // условно деструктор

///--- main thread function
void *mx_keypad_lcm(void *arg);

///--- display functions
void show_main_ifrtu1();
void show_main_ifrtu2();
void show_main_iftcp();
void show_main_moxagate();
void show_main_eventlog();
//void show_system_info();
//void show_main_settings();

void show_main_help();
void show_about_screen();
void show_confirm_halt();

void show_uart_settings(int uart);
void show_ifrtu_statistics(int uart);
void show_confirm_reset_counters();

void show_iftcp_statistics();
void show_lantcp_settings();

void show_stat_moxagate();
void show_sett_moxagate();

void show_log_event();

///--- keypad processing functions
void show_screen(int display);

void process_key_main(int key);
void process_key_rtu_details(int key);
void process_key_tcp_details(int key);
void process_key_moxagate(int key);
void process_key_event(int key);

///--- control functions
//int ctrl_reset_all_counters();
//int ctrl_reset_port_counters(int port);

#endif  /* HMI_KEYPAD_LCM_H */
