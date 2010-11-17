/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010                 
***************************************/

#ifndef MOXAGATE_H
#define MOXAGATE_H

///*********************** МОДУЛЬ УСТРОЙСТВА MODBUS MOXA UC-7410 ***************
///*** МЕХАНИЗМ ВНУТРЕННЕЙ ПАМЯТИ
///*** СТРУКТУРЫ ДАННЫХ ДЛЯ РЕАЛИЗАЦИИ ФУНКЦИЙ HMI-ВЗАИМОДЕЙСТВИЯ

#include "queue.h"
#include "clients.h"

///=== MOXAGATE_H constants

#define	BASIC_STAT_GATEWAY_INFO	24
#define	GATE_STATUS_BLOCK_LENGTH	BASIC_STAT_GATEWAY_INFO+MAX_QUERY_ENTRIES/16

#define MOXA_DIAPASON_UNDEFINED		0
#define MOXA_DIAPASON_INSIDE			1
#define MOXA_DIAPASON_OUTSIDE			2
#define MOXA_DIAPASON_OVERLAPPED	3

#define MAX_VIRTUAL_SLAVES 128
#define MAX_QUERY_ENTRIES 128

#define EXCEPTION_DIOGEN 0x00000001

///=== MOXAGATE_H data types

typedef struct {
	int				ssd;						// TCP-порт для приема входящих клиентских соединений
	int current_client;				// номер свободного соединения

unsigned		tcp_port;	// номер TCP-порта для работы с последовательными интерфейсами шлюза в режимах ATM, RTM
u8					modbus_address; // собственный Modbus-адрес шлюза для диагностики и управления средствами HMI систем
unsigned		status_info; // стартовый адрес блока собственных регистров шлюза (по умолчанию начинается с первого)

char object[DEVICE_NAME_LENGTH];			// наименование объекта автоматизации
char location[DEVICE_NAME_LENGTH];		// Место установки устройства MOXA UC-7410
char version[DEVICE_NAME_LENGTH];			// Версия конфигурационного файла
char networkName[DEVICE_NAME_LENGTH];	// Сетевое имя устройства
unsigned int IPAddress;								// сетевой адрес

u8 show_data_flow;		// Показывать пакеты данных в сессии Telnet
u8 show_sys_messages;	// Показывать системные сообщения в сессии Telnet
u8 watchdog_timer;		// Использовать Watchdog-таймер
u8 use_buzzer;				// Испльзовать зуммер для сигнализации ошибок
u8 back_light;				// Состояние подсветки дисплея

  pthread_mutex_t moxa_mutex;

	GW_Queue queue;

  unsigned int accepted_connections_number;
  unsigned int  current_connections_number;
  unsigned int rejected_connections_number;
  
	GW_Client	clients[MAX_MOXA_PORTS*MAX_TCP_CLIENTS_PER_PORT];
  GW_StaticData stat;

	time_t start_time;  // время запуска программы
	time_t timestamp;  // время последнего обновления данных для web-интерфейса
	unsigned app_log_current_entry, app_log_entries_total;
	unsigned msg_filter;
	char halt;

	// начало блока внутренних регистров Moxa (смещение)
	unsigned offset1xStatus, offset2xStatus, offset3xRegisters, offset4xRegisters;
	// количество регистров под каждую из таблиц
	unsigned amount1xStatus, amount2xStatus, amount3xRegisters, amount4xRegisters;
	// указатели на массивы памяти
	u8	*wData1x; //массив виртуальных 1x регистров MOXA (coil status)
	u8	*wData2x; //массив виртуальных 2x регистров MOXA (input status)
	u16	*wData3x; //массив виртуальных 3x регистров MOXA (input register)
	u16	*wData4x; //массив виртуальных 4x регистров MOXA (holding register)
	
	} input_cfg_502;

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

///=== MOXAGATE_H public variables

input_cfg_502 gate502;								 // данные и параметры устройства MOXAGATE

RT_Table_Entry vslave[MAX_VIRTUAL_SLAVES];
Query_Table_Entry query_table[MAX_QUERY_ENTRIES];

// массив исключительных ситуаций служит для устранения проблем при обмене, вызванных особенностями
// конечных устройств modbus-slave. он содержит набор флагов, включающих определенные алгоритмы в
// определенных ситуациях.

// исключение для СКС-07, параметр - битовый массив номеров последовательных портов, к которым подключены диогены
unsigned int exceptions; // массив из 16 флагов
unsigned int except_prm[16]; // параметр исключения

///=== MOXAGATE_H public functions

void *moxa_mb_thread(void *arg); /// Потоковая функция обработки запросов к MOXA

int checkDiapason(int function, int start_address, int length);
int process_moxamb_request(int client_id, u8 *adu, u16 adu_len, u8 *memory_adu, u16 *memory_adu_len);

#endif  /* MOXAGATE_H */
