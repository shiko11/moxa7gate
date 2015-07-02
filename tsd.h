/***********   XXXXXXXXX   *************
        MODBUS XXXXXXX SOFTWARE         
                    VERSION X.X         
        AO NPO SPETSELECTROMEHANIKA     
               BRYANSK 2015                 
***************************************/

#ifndef TSD_H
#define TSD_H

///******************* МОДУЛЬ TSD ********************************

#include "modbus.h"

///=== TSD_H constants

// конфигурационные константы времени компиляции

#define HAGENT_HEADER_LEN   8 // длина заголовка буфера истории в регистрах
#define HAGENT_PARAMS_MAX  32 // максимальное количество параметров в буфере истории
#define HAGENT_METRIC_LEN  16 // длина текстовой строки с метрикой параметра истории

///=== TSD_H data types

typedef struct { // ЗАГОЛОВОК БУФЕРА ИСТОРИИ
  unsigned int header; // Указатель на первый свободный блок ЕИП
  unsigned int cycles; // Количество циклов перезаписи буфера
  
  unsigned int min  ;  // Метка времени заголовка буфера истории, минута
  unsigned int hour ;  // Метка времени заголовка буфера истории, час
  unsigned int day  ;  // Метка времени заголовка буфера истории, день
  unsigned int month;  // Метка времени заголовка буфера истории, месяц
  unsigned int year ;  // Метка времени заголовка буфера истории, год
  
  unsigned int prmnum; // Статус буфера истории, количество параметров в группе
  unsigned int msprec; // Статус буфера истории, точность (0 - сек; 1 - мс)
  
  unsigned int fault1; // Слово 1 флагов достоверности параметров
  unsigned int fault2; // Слово 2 флагов достоверности параметров
  unsigned int group1; // Слово 1 группировки параметров
  unsigned int group2; // Слово 2 группировки параметров
  
	} HB_Header;

typedef struct { // ПАРАМЕТР ИСТОРИИ
  char         metric[HAGENT_METRIC_LEN]; // метрика для записи в openTSDB
  unsigned int adc_min; // начало диапазона измерения в кодах АЦП
  unsigned int adc_max; // конец  диапазона измерения в кодах АЦП
  float        eng_min; // начало диапазона измерения в инженерных единицах
  float        eng_max; // конец  диапазона измерения в инженерных единицах
	} HB_Parameter;

///=== TSD_H public variables

// максимум четыре буфера истории, относящиеся соответственно к портам T01..T04
HB_Parameter HB_Param[4][HAGENT_PARAMS_MAX];

///=== TSD_H public functions

// условно конструктор
int init_tsd_h(int port);

int  check_header(u8 *rsp_adu, HB_Header *hbhdr);
void  copy_header(HB_Header *src, HB_Header *dst);

///*************************************************************************************

#endif  /* TSD_H */
