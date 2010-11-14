/*
MODBUS GATEWAY SOFTWARE
MOXA7GATE VERSION 1.2
SEM-ENGINEERING
					BRYANSK 2010
*/

#ifndef GLOBAL_H
#define GLOBAL_H

#include <pthread.h>
#include <sys/time.h>
#include <sys/socket.h>

#include <errno.h>
#include <fcntl.h>

#include <netinet/in.h>

//#include <signal.h>
//#include <sys/ipc.h>
//#include <sys/types.h>

#define DEVICE_NAME_LENGTH	64
#define GATE_WEB_INTERFACE_TIMEOUT	2

///=== ВКЛЮЧАЕМ МОДУЛИ ПРОГРАММЫ

#include "modbus.h"
#include "modbus_rtu.h"
#include "hmi_keypad_lcm.h"
#include "mxlib/mxwdg.h"
#include "messages.h"
#include "statistics.h"
#include "clients.h"
#include "interfaces.h"
#include "moxagate.h"
#include "hmi_web.h"

#define MAX_VIRTUAL_SLAVES 128
/// пока определия этих блоков адресов относится к Holding-регистрам modbus и функции 06
typedef struct {			// блок регистров внутреннего адресного пространства шлюза
	unsigned start;			// начальный регистр диапазона адресного пространства шлюза
	unsigned length;		// количество регистров дипазона адресного пространства шлюза
	unsigned port;			// последовательный порт шлюза для перенаправления запроса
	unsigned device;		// адрес устройства в сети modbus для перенаправления запроса

  unsigned modbus_table; // одна из 4-х стандартных таблиц протокола MODBUS (см. файл modbus_rtu.h)
  unsigned address_shift;  // смещение в адресном пространстве устройства modbus (чтобы читать регистры не с нуля)
	char device_name[DEVICE_NAME_LENGTH]; // наименование устройства
  } RT_Table_Entry;

#define MAX_QUERY_ENTRIES 128
typedef struct {			// modbus-запрос для циклического опроса ведомого устройства в режиме PROXY
	unsigned start;			// начальный регистр диапазона адресного пространства ведомого устройства
	unsigned length;		// количество регистров в диапазоне адресов ведомого устройства
  unsigned offset;  // номер регистра (смещение) в адресном пространстве шлюза, с которого запивывать полученные данные
	unsigned port;			// последовательный порт шлюза для опроса ведомого устройства
	unsigned device;		// адрес ведомого устройства в сети modbus

  unsigned mbf; 			// одна из стандартных функций протокола MODBUS (см. файл modbus_rtu.h)
  unsigned delay;  // задержка перед отправкой запроса при работе в режиме PROXY в милисекундах
	unsigned critical;			// количество ошибок до изменения бита статуса связи
  unsigned err_counter;  // счетчик текущего количества ошибок
  unsigned status_bit;  // бит статуса связи (первый в переменной)
	char device_name[DEVICE_NAME_LENGTH]; // наименование устройства
  } Query_Table_Entry;

//unsigned int buzzer_flag; // зуммер дает 1, 2 и 3 гудка в зависимости от количества ошибок: <15, 15-30, >30
struct timeval tv;
struct timeval tv_mem;
struct timezone tz;
unsigned int p_errors[MAX_MOXA_PORTS];

// переменные мультиплексированного ввода
fd_set watchset;
fd_set inset;

RT_Table_Entry vslave[MAX_VIRTUAL_SLAVES];
Query_Table_Entry query_table[MAX_QUERY_ENTRIES];

// массив исключительных ситуаций служит для устранения проблем при обмене, вызванных особенностями
// конечных устройств modbus-slave. он содержит набор флагов, включающих определенные алгоритмы в
// определенных ситуациях.

// исключение для СКС-07, параметр - битовый массив номеров последовательных портов, к которым подключены диогены
#define EXCEPTION_DIOGEN 0x00000001

unsigned int exceptions; // массив из 16 флагов
unsigned int except_prm[16]; // параметр исключения

#define   MAX_KEYS                 64        //максимальное количество параметров командной строки с префиксом "--"
//ошибки интерпретации командной строки
#define   CL_ERR_NONE_PARAM        -1    //командная строка пуста
#define   CL_INFO                  -5
#define   CL_ERR_PORT_WORD         -2   //ошибка в ключевых словах // keyword PORT absent
#define   CL_ERR_IN_STRUCT         -3   //ошибка в данных командной строки
#define   CL_ERR_IN_PORT_SETT      -4   //ошибка в данных командной строки
#define   CL_ERR_GATEWAY_MODE      -6
#define   CL_ERR_IN_MAP						 -7
#define   CL_ERR_MIN_PARAM				 -8
#define   CL_ERR_MUTEX_PARAM			 -9
#define   CL_ERR_VSLAVES_CFG			 -10
#define   CL_ERR_QT_CFG						 -11
#define   CL_ERR_TCPSRV_CFG				 -12
#define   CL_ERR_NOT_ALLOWED			 -13
#define   CL_OK                    0    //усе нормуль

//#define   CL_ERR_MIN_PARAM         -1   //менее 9 параметров
//#define   CL_ERR_NUM_STAT_BIT			 -4 //номер бита больше 16 (диапазон 1-16)

int get_command_line (int 	argc,
											char	*argv[],
											input_cfg *ptr_iDATA,
											input_cfg_502 *ptr_gate502,
											RT_Table_Entry *vslave,
											Query_Table_Entry *query_table,
											GW_TCP_Server *tcp_servers
											);

//void sigpipe_handler();
//void sigio_handler();

int gateway_common_processing();
int gateway_single_port_processing();
int query_translating();

int get_ip_from_string(char *str, unsigned int *ip, unsigned int *port);

#endif  /* GLOBAL_H */
