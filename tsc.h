/***********   XXXXXXXXX   *************
        MODBUS XXXXXXX SOFTWARE         
                    VERSION X.X         
        AO NPO SPETSELECTROMEHANIKA     
               BRYANSK 2015                 
***************************************/

#ifndef TSC_H
#define TSC_H

///******************* МОДУЛЬ TSC ********************************

#include <sys/time.h>
#include <time.h>

#include "modbus.h"

///=== TSC_H constants

// конфигурационные константы времени компиляции

#define HAGENT_HEADER_LEN   8 // длина заголовка буфера истории в регистрах
#define HAGENT_PARAMS_MAX  32 // максимальное количество параметров в буфере истории
#define HAGENT_BUFEIP_MAX 256 // максимальный размер буфера истории в блоках ЕИП
#define HAGENT_METRIC_LEN  16 // длина текстовой строки с метрикой параметра истории
#define HAGENT_TAGS_LEN    32 // длина текстовой строки с тегами   параметра истории
///=== TSC_H data types

typedef struct { // ЗАГОЛОВОК БУФЕРА ИСТОРИИ
  unsigned int header; // Указатель на первый свободный блок ЕИП
  unsigned int cycles; // Количество циклов перезаписи буфера
  
  unsigned int min  ;  // Метка времени, минута
  unsigned int hour ;  // Метка времени, час
  unsigned int day  ;  // Метка времени, день
  unsigned int month;  // Метка времени, месяц
  unsigned int year ;  // Метка времени, год
  
  unsigned int prmnum; // Статус буфера истории, количество параметров в группе
  unsigned int msprec; // Статус буфера истории, точность (0 - сек; 1 - мс)
  
  unsigned long fault; // Слово флагов достоверности параметров
  unsigned long group; // Слово группировки параметров

  struct tm      tm;   // Метка времени в формате Linux
  struct timeval tv;   // Метка времени в формате Linux
  
	} HB_Header;

typedef struct { // ХАРАКТЕРИСТИКИ ПАРАМЕТРА ИСТОРИИ
  char         metric[HAGENT_METRIC_LEN]; // метрика для записи в openTSDB
  char         tags  [HAGENT_TAGS_LEN  ]; // теги    для записи в openTSDB
  unsigned int adc_min; // начало диапазона измерения в кодах АЦП
  unsigned int adc_max; // конец  диапазона измерения в кодах АЦП
  float        eng_min; // начало диапазона измерения в инженерных единицах
  float        eng_max; // конец  диапазона измерения в инженерных единицах
	} HB_Parameter;

typedef struct { // БЛОК ЕДИНИЧНОГО ИЗМЕРЕНИЯ ПАРАМЕТРОВ (ЕИП)
  unsigned int validity;               // признак корректности метки времени
  unsigned int time_mark;              // метка времени в формате HAGENT
  unsigned int  ms;                    // метка времени, милисекунды
  unsigned int sec;                    // метка времени, секунды
  unsigned int min;                    // метка времени, минуты
  struct timeval tv;                   // метка времени в формате Linux
  unsigned int prm[HAGENT_PARAMS_MAX]; // массив единичных измерений
	} HB_EIP_Block;

///=== TSC_H public variables

// максимум четыре буфера истории, относящиеся соответственно к портам T01..T04
extern HB_Parameter HB_Param[4][HAGENT_PARAMS_MAX]; // массив характеристик параметров истории

///=== TSC_H public functions

// условно конструктор
int init_tsc_h(int port);

int  check_header(u8 *rsp_adu, HB_Header *hdr);
void  copy_header(HB_Header *src, HB_Header *dst);
void  cross_params(int port, unsigned long group);

///*************************************************************************************

#endif  /* TSC_H */
