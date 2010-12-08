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

#define KEY_F1 0
#define KEY_F2 1
#define KEY_F3 2
#define KEY_F4 3
#define KEY_F5 4

#define	LCM_SCREEN_MAIN				1
#define	LCM_SCREEN_MAIN2			2
#define	LCM_SCREEN_HELP_MAIN	3
#define	LCM_SCREEN_ABOUT			6
#define	LCM_SCREEN_SYSINFO		17
#define	LCM_SCREEN_PORT1_STAT	18
#define	LCM_SCREEN_PORT2_STAT	19
#define	LCM_SCREEN_PORT3_STAT	20
#define	LCM_SCREEN_PORT4_STAT	21
#define	LCM_SCREEN_PORT5_STAT	22
#define	LCM_SCREEN_PORT6_STAT	23
#define	LCM_SCREEN_PORT7_STAT	24
#define	LCM_SCREEN_PORT8_STAT	25
#define	LCM_CONFIRM_RESETALL	27

#define	LCM_SCREEN_MENU				4
#define	LCM_SCREEN_HELP_MENU	5
#define	LCM_SCREEN_PORT1			7
#define	LCM_SCREEN_PORT2			8
#define	LCM_SCREEN_PORT3			9
#define	LCM_SCREEN_PORT4			10
#define	LCM_SCREEN_PORT5			11
#define	LCM_SCREEN_PORT6			12
#define	LCM_SCREEN_PORT7		 	13
#define	LCM_SCREEN_PORT8			14
#define	LCM_SCREEN_LAN				15
#define	LCM_CONFIRM_REBOOT		26

#define	LCM_SCREEN_UPDATE_RATE 800000

#define	LCM_SCREEN_SECURITY					16
#define	LCM_SCREEN_HELP_SECURITY		28
#define	LCM_SCREEN_STUB_SECURITY 		29
#define	LCM_CONFIRM_SECR_CHANGES 		30

#define	LCM_BUZZER_CONTROL_PERIOD 15
#define	LCM_BUZZER_CONTROL_ERRORS 1

// коды возврата ошибок
#define HMI_KLB_INIT_KEYPAD 1
#define HMI_KLB_INIT_LCM    2
#define HMI_KLB_INIT_BUZZER 3
#define HMI_KLB_INIT_THREAD 4

///=== HMI_KEYPAD_LCM_H data types

typedef struct { // net initsializatsii polej structury
	unsigned int current_screen;
	unsigned int prev_screen;

  unsigned int main_scr_mode;
  unsigned int menu_scr_mode;
  unsigned int secr_scr_mode;
  unsigned int back_light;
  
  unsigned int secr_scr_changes_was_made;

  char text[MAX_LCM_ROWS][MAX_LCM_COLS];

  unsigned int max_tcp_clients_per_com;
  unsigned int watch_dog_control;

  unsigned int buzzer_control;

	} GW_Display;

///=== HMI_KEYPAD_LCM_H public variables
GW_Display screen;

int mxlcm_handle;
int mxkpd_handle;
int mxbzr_handle;

///=== HMI_KEYPAD_LCM_H public functions

int init_hmi_keypad_lcm_h();  // условно конструктор
int clear_hmi_keypad_lcm_h(); // условно деструктор

///--- main thread function
void *mx_keypad_lcm(void *arg);

///--- display functions
void show_main_screen();
void show_main_screen2();
void show_inet_settings();
void show_uart_settings(int uart);
void show_security_settings();
void show_system_info();
void show_uart_detail(int uart);
void show_main_help_screen();
void show_security_help_screen();
void show_menu_help_screen();
void show_about_screen();
void show_confirmation_reset_all();
void show_confirmation_reboot();
void show_stub_screen();
void show_menu_screen();
void show_confirmation_secr_changes();

///--- keypad processing functions
void show_screen(int display);
void process_key_main(int key);
void process_key_menu(int key);
void process_key_security(int key);

///--- control functions
int ctrl_reset_all_counters();
int ctrl_reset_port_counters(int port);
int ctrl_reboot_system();
int ctrl_change_security_settings();

#endif  /* HMI_KEYPAD_LCM_H */
