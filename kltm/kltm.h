/***********   K M - 4 0 0   *************
      КОММУНИКАЦИОННЫЙ МОДУЛЬ                                
                              ВЕРСИЯ 1.0
      ООО "БЗПА"
               БРЯНСК 2013                 
*****************************************/

#ifndef KLTM_H
#define KLTM_H

///**** МОДУЛЬ ДЛЯ РАБОТЫ С КОНТРОЛЛЕРОМ ТЕЛЕМЕХАНИКИ (M340) ПО ЧАСТНОМУ ПРОТОКОЛУ
///**** ("ПРОТОКОЛ ПЕРЕДАЧИ ПАКЕТОВ", РАБОТАЮЩИЙ ПОВЕРХ MODBUS RTU/MODBUS TCP)
///**** С ОДНОЙ СТОРОНЫ И МАСТЕРОМ ТЕЛЕМЕХАНИКИ (РДП) - С ДРУГОЙ

/* Предполагается использование этого модуля в отдельном потоке программы,
   требуется настроенный интерфейс типа IFACE_RTUMASTER или IFACE_TCPMASTER
   для циклического опроса и записи значений переменных PLC и CM в КЛТМ */

#define KLTM_DATA_FIELD_SIZE 114
#define KLTM_SCOM_STRUCT_SIZE 2+KLTM_DATA_FIELD_SIZE/2

typedef struct { // структура пакета данных, используемого для обмена между КЛТМ и КМ-400
	unsigned char R_PAC_NUM;  // номер пакета принятого
	unsigned char T_PAC_NUM;  // номер пакета отправленного
	unsigned char T_PAC_LEN;  // актуальная длина блока данных в поле T_DATA
	unsigned char T_PAC_CODE; // код пакета (операции)
	unsigned char T_DATA[KLTM_DATA_FIELD_SIZE]; // данные
  } kltm_sCom;

///=== KLTM_H public variables

unsigned char kltm_port;   // идентификатор интерфейса шлюза для работы с КЛТМ
int           kltm_client; // идентификатор клиентского соединения

// по окончании тестирования перенести в файл kltm.c
int       kltm_CM;    // индекс в таблице виртуальных устройств, соответствующий переменной CM
int       kltm_PLC;   // индекс в таблице опроса,                соответствующий переменной PLC

///=== KLTM_H public functions

void *kltm_main(void *arg); /// потоковая функция главного потока КМ-400

int init_kltm_h();  // функция для проверки корректности конфигурации Modbus-интерфейса

int kltm_getPLC(kltm_sCom *plc); // получить от КЛТМ запрос (актуальное значение переменной PLC)
int kltm_setCM (kltm_sCom *cm ); // записать в  КЛТМ ответ  (новое значение переменной CM)

/// функции для работы с модемом (тестовая версия)

int init_modem();
int set_rts(int val);
void show_modem_lines();
void print_buffer(unsigned char *buf, unsigned char len);
int get_modem_data(char *at);
int close_modem();

#endif /* KLTM_H */
