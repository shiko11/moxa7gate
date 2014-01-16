/************************************************************
ОАО НПО Спецэлектромеханика Брянск 2014г.
Библиотека для работы с протоколом Элеси-МЭК 3

Константы для работы с данными от ПЛК
********************************************************** */

#ifndef _IEC870_PLC_H_
#define _IEC870_PLC_H_

//констатнты для разбора данных от ПЛК
#define PLC_MAX_PACK_LEN   125  //максимальный размер пакета данных от ПЛК
#define PLC_ADDR_LOW         1  //относительное смещение для младщей части адреса
#define PLC_ADDR_HIGT        0  //относительное смещение для старшей части адреса
#define PLC_PR               2  //относительное смещение для признака
#define PLC_LEN              3  //относительное смещение для поля длина пакета
#define PLC_DATA_ADD         4  /*относительно смещение от начала пакета поля где
 может передоваться текущее значение сигнала, если он передается без метки времени*/
#define PLC_ADD_TIME         4  //длина поля метка времени
#define PLC_TS_LEN           1  //длина данных телесигнализации
#define PLC_TI1_LEN          1  //длина данных телеизмерение первого типа
#define PLC_TI2_LEN          2  //длина данных телеизмерение второго типа
#define PLC_TI4_LEN          4  //длина данных телеизмерение четвертого типа
#define PLC_PR_LEN           1 // Длина поля с признаком
#define PLC_TIME_LEN         4 // Длина поля с меткой времени

//константы для лпределения типов данных от ПЛК
#define PLC_SIG_TYPE    0x0F00 //маска тип сигнала
#define PLC_SIG_CRIT    0x3000 //маска критерии передачи - метка времени и признак
#define PLC_SIG_PR      0x1000 //данные с признаком
#define PLC_SIG_TIME    0x2000 //данные с меткой времени
#define PLC_SIG_TS      0x0000
#define PLC_SIG_TU      0x0100
#define PLC_SIG_TI1     0x0200
#define PLC_SIG_TI2     0x0300
#define PLC_SIG_TI4     0x0400

//Константы для хранения параметров передоваемого сигнала, таких как метка времени и статус
#define PLC_SIG_WITH_NONE         0
#define PLC_SIG_WITH_TIME         1
#define PLC_SIG_WITH_PR           2
#define PLC_SIG_WITH_BOTH         3

//макрос для опроделения критериев
#define GET_PLC_KRIT(ADDR, TYPE1, TYPE2, TYPE3, TYPE4, OUT) if(((ADDR) & PLC_SIG_CRIT) == PLC_SIG_CRIT)  (OUT) = (TYPE1); else if(((ADDR) & PLC_SIG_CRIT) == PLC_SIG_PR) (OUT) = (TYPE2); else if(((ADDR) & PLC_SIG_CRIT) == PLC_SIG_TIME) (OUT) = (TYPE3); else (OUT) = (TYPE4)  

#endif
