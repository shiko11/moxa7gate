/***********   XXXXXXXXX   *************
        MODBUS XXXXXXX SOFTWARE         
                    VERSION X.X         
        AO NPO SPETSELECTROMEHANIKA     
               BRYANSK 2015                 
***************************************/

///******************* МОДУЛЬ TSC ********************************

///=== TSC_H IMPLEMENTATION

#include <string.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <pthread.h>

#include "tsc.h"
#include "interfaces.h"

///=== TSC_H public variables

// максимум четыре буфера истории, относящиеся соответственно к портам T01..T04
HB_Parameter HB_Param[4][HAGENT_PARAMS_MAX]; // массив характеристик параметров истории

///=== TSC_H private variables

HB_Parameter HB_Param_Cross[150]; // сквозной массив характеристик параметров истории

///=== TSC_H private functions

void cpprm(int port, int src, int dst);

//******************************************************************************
int init_tsc_h(int port)
  {
  int i=port-GATEWAY_T01;
  char metric[128];
  char tags  [128];

  if(port<GATEWAY_T01 || port>GATEWAY_T04) return 1;

  switch(port) {

      // тест, ед. изм.
//   	strcpy(metric, "metric1");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
//   	strcpy(tags  , " tagk=tagv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
//   	strcpy(HB_Param[i][0].metric, metric);
//   	strcpy(HB_Param[i][0].tags  , tags  );
//   	HB_Param[i][0].adc_min=0; HB_Param[i][0].adc_max=35;
//   	HB_Param[i][0].eng_min=0; HB_Param[i][0].eng_max=66535.5;

//15	LT653, мм	Уровень в емкости ЕП-40 (нефть)	3605.0	0.0	65535	0	level	node=ep40

//      // \2, \3
//   	  strcpy(metric, "\8");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
//   	  strcpy(tags  , " \9"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
//   	  strcpy(HB_Param[i][\1].metric, metric);
//   	  strcpy(HB_Param[i][\1].tags  , tags  );
//   	  HB_Param[i][\1].adc_min=\7; HB_Param[i][\1].adc_max=\6;
//   	  HB_Param[i][\1].eng_min=\5; HB_Param[i][\1].eng_max=\4;

    case GATEWAY_T01:
      // PT106, МА-21. Давление воздуха в зоне клапана продувки ЭД (кПа)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma21 sys=ptx06 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][0].metric, metric);
   	  strcpy(HB_Param[i][0].tags  , tags  );
   	  HB_Param[i][0].adc_min=4000; HB_Param[i][0].adc_max=20000;
   	  HB_Param[i][0].eng_min=0.0; HB_Param[i][0].eng_max=6.0;
      // PT104, МА-21. Давление воздуха в корпусе ЭД (точка 1) (кПа)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma21 sys=ptx04 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][1].metric, metric);
   	  strcpy(HB_Param[i][1].tags  , tags  );
   	  HB_Param[i][1].adc_min=4000; HB_Param[i][1].adc_max=20000;
   	  HB_Param[i][1].eng_min=0.0; HB_Param[i][1].eng_max=2.50;
      // PT105, МА-21. Давление воздуха в корпусе ЭД (точка 2) (кПа)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma21 sys=ptx05 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][2].metric, metric);
   	  strcpy(HB_Param[i][2].tags  , tags  );
   	  HB_Param[i][2].adc_min=4000; HB_Param[i][2].adc_max=20000;
   	  HB_Param[i][2].eng_min=0.0; HB_Param[i][2].eng_max=2.50;
      // PT107, МА-21. Давление воздуха возбудителя ЭД (кПа)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma21 sys=ptx07 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][3].metric, metric);
   	  strcpy(HB_Param[i][3].tags  , tags  );
   	  HB_Param[i][3].adc_min=4000; HB_Param[i][3].adc_max=20000;
   	  HB_Param[i][3].eng_min=0.0; HB_Param[i][3].eng_max=2.50;
      // FT641, ПЧ подпорного вентилятора П23А. Частота на выходе ПЧ (Гц)
   	  strcpy(metric, "freq");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma21 sys=ft64x dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][4].metric, metric);
   	  strcpy(HB_Param[i][4].tags  , tags  );
   	  HB_Param[i][4].adc_min=0; HB_Param[i][4].adc_max=10000;
   	  HB_Param[i][4].eng_min=0.0; HB_Param[i][4].eng_max=50.0;
      // CV106, МА-21. Давление контура регулирования (кПа)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma21 sys=auto dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][5].metric, metric);
   	  strcpy(HB_Param[i][5].tags  , tags  );
   	  HB_Param[i][5].adc_min=0; HB_Param[i][5].adc_max=60000;
   	  HB_Param[i][5].eng_min=0.0; HB_Param[i][5].eng_max=6.0;
      // SP1A6, МА-21. Уставка контура регулирования (алгоритм) (кПа)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma21 sys=auto dp=sp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][6].metric, metric);
   	  strcpy(HB_Param[i][6].tags  , tags  );
   	  HB_Param[i][6].adc_min=0; HB_Param[i][6].adc_max=60000;
   	  HB_Param[i][6].eng_min=0.0; HB_Param[i][6].eng_max=6.0;
      // SP1M6, МА-21. Уставка контура регулирования (оператор) (кПа)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma21 sys=pust dp=sp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][7].metric, metric);
   	  strcpy(HB_Param[i][7].tags  , tags  );
   	  HB_Param[i][7].adc_min=0; HB_Param[i][7].adc_max=60000;
   	  HB_Param[i][7].eng_min=0.0; HB_Param[i][7].eng_max=6.0;
      // PT206, МА-22. Давление воздуха в зоне клапана продувки ЭД (кПа)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma22 sys=ptx06 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][8].metric, metric);
   	  strcpy(HB_Param[i][8].tags  , tags  );
   	  HB_Param[i][8].adc_min=4000; HB_Param[i][8].adc_max=20000;
   	  HB_Param[i][8].eng_min=0.0; HB_Param[i][8].eng_max=6.0;
      // PT204, МА-22. Давление воздуха в корпусе ЭД (точка 1) (кПа)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma22 sys=ptx04 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][9].metric, metric);
   	  strcpy(HB_Param[i][9].tags  , tags  );
   	  HB_Param[i][9].adc_min=4000; HB_Param[i][9].adc_max=20000;
   	  HB_Param[i][9].eng_min=0.0; HB_Param[i][9].eng_max=2.50;
      // PT205, МА-22. Давление воздуха в корпусе ЭД (точка 2) (кПа)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma22 sys=ptx05 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][10].metric, metric);
   	  strcpy(HB_Param[i][10].tags  , tags  );
   	  HB_Param[i][10].adc_min=4000; HB_Param[i][10].adc_max=20000;
   	  HB_Param[i][10].eng_min=0.0; HB_Param[i][10].eng_max=2.50;
      // PT207, МА-22. Давление воздуха возбудителя ЭД (кПа)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma22 sys=ptx07 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][11].metric, metric);
   	  strcpy(HB_Param[i][11].tags  , tags  );
   	  HB_Param[i][11].adc_min=4000; HB_Param[i][11].adc_max=20000;
   	  HB_Param[i][11].eng_min=0.0; HB_Param[i][11].eng_max=2.50;
      // FT642, ПЧ подпорного вентилятора П23B. Частота на выходе ПЧ (Гц)
   	  strcpy(metric, "freq");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma22 sys=ft64x dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][12].metric, metric);
   	  strcpy(HB_Param[i][12].tags  , tags  );
   	  HB_Param[i][12].adc_min=0; HB_Param[i][12].adc_max=10000;
   	  HB_Param[i][12].eng_min=0.0; HB_Param[i][12].eng_max=50.0;
      // CV206, МА-22. Давление контура регулирования (кПа)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma22 sys=auto dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][13].metric, metric);
   	  strcpy(HB_Param[i][13].tags  , tags  );
   	  HB_Param[i][13].adc_min=0; HB_Param[i][13].adc_max=60000;
   	  HB_Param[i][13].eng_min=0.0; HB_Param[i][13].eng_max=6.0;
      // SP2A6, МА-22. Уставка контура регулирования (алгоритм) (кПа)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma22 sys=auto dp=sp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][14].metric, metric);
   	  strcpy(HB_Param[i][14].tags  , tags  );
   	  HB_Param[i][14].adc_min=0; HB_Param[i][14].adc_max=60000;
   	  HB_Param[i][14].eng_min=0.0; HB_Param[i][14].eng_max=6.0;
      // SP2M6, МА-22. Уставка контура регулирования (оператор) (кПа)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma22 sys=pust dp=sp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][15].metric, metric);
   	  strcpy(HB_Param[i][15].tags  , tags  );
   	  HB_Param[i][15].adc_min=0; HB_Param[i][15].adc_max=60000;
   	  HB_Param[i][15].eng_min=0.0; HB_Param[i][15].eng_max=6.0;

    	break;

    case GATEWAY_T02:
      // PT306, МА-23. Давление воздуха в зоне клапана продувки ЭД (кПа)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma23 sys=ptx06 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][0].metric, metric);
   	  strcpy(HB_Param[i][0].tags  , tags  );
   	  HB_Param[i][0].adc_min=4000; HB_Param[i][0].adc_max=20000;
   	  HB_Param[i][0].eng_min=0.0; HB_Param[i][0].eng_max=6.0;
      // PT304, МА-23. Давление воздуха в корпусе ЭД (точка 1) (кПа)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma23 sys=ptx04 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][1].metric, metric);
   	  strcpy(HB_Param[i][1].tags  , tags  );
   	  HB_Param[i][1].adc_min=4000; HB_Param[i][1].adc_max=20000;
   	  HB_Param[i][1].eng_min=0.0; HB_Param[i][1].eng_max=2.50;
      // PT305, МА-23. Давление воздуха в корпусе ЭД (точка 2) (кПа)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma23 sys=ptx05 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][2].metric, metric);
   	  strcpy(HB_Param[i][2].tags  , tags  );
   	  HB_Param[i][2].adc_min=4000; HB_Param[i][2].adc_max=20000;
   	  HB_Param[i][2].eng_min=0.0; HB_Param[i][2].eng_max=2.50;
      // PT307, МА-23. Давление воздуха возбудителя ЭД (кПа)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma23 sys=ptx07 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][3].metric, metric);
   	  strcpy(HB_Param[i][3].tags  , tags  );
   	  HB_Param[i][3].adc_min=4000; HB_Param[i][3].adc_max=20000;
   	  HB_Param[i][3].eng_min=0.0; HB_Param[i][3].eng_max=2.50;
      // FT643, ПЧ подпорного вентилятора П23C. Частота на выходе ПЧ (Гц)
   	  strcpy(metric, "freq");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma23 sys=ft64x dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][4].metric, metric);
   	  strcpy(HB_Param[i][4].tags  , tags  );
   	  HB_Param[i][4].adc_min=0; HB_Param[i][4].adc_max=10000;
   	  HB_Param[i][4].eng_min=0.0; HB_Param[i][4].eng_max=50.0;
      // CV306, МА-23. Давление контура регулирования (кПа)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma23 sys=auto dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][5].metric, metric);
   	  strcpy(HB_Param[i][5].tags  , tags  );
   	  HB_Param[i][5].adc_min=0; HB_Param[i][5].adc_max=60000;
   	  HB_Param[i][5].eng_min=0.0; HB_Param[i][5].eng_max=6.0;
      // SP3A6, МА-23. Уставка контура регулирования (алгоритм) (кПа)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma23 sys=auto dp=sp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][6].metric, metric);
   	  strcpy(HB_Param[i][6].tags  , tags  );
   	  HB_Param[i][6].adc_min=0; HB_Param[i][6].adc_max=60000;
   	  HB_Param[i][6].eng_min=0.0; HB_Param[i][6].eng_max=6.0;
      // SP3M6, МА-23. Уставка контура регулирования (оператор) (кПа)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma23 sys=pust dp=sp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][7].metric, metric);
   	  strcpy(HB_Param[i][7].tags  , tags  );
   	  HB_Param[i][7].adc_min=0; HB_Param[i][7].adc_max=60000;
   	  HB_Param[i][7].eng_min=0.0; HB_Param[i][7].eng_max=6.0;
      // PT406, МА-24. Давление воздуха в зоне клапана продувки ЭД (кПа)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma24 sys=ptx06 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][8].metric, metric);
   	  strcpy(HB_Param[i][8].tags  , tags  );
   	  HB_Param[i][8].adc_min=4000; HB_Param[i][8].adc_max=20000;
   	  HB_Param[i][8].eng_min=0.0; HB_Param[i][8].eng_max=6.0;
      // PT404, МА-24. Давление воздуха в корпусе ЭД (точка 1) (кПа)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma24 sys=ptx04 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][9].metric, metric);
   	  strcpy(HB_Param[i][9].tags  , tags  );
   	  HB_Param[i][9].adc_min=4000; HB_Param[i][9].adc_max=20000;
   	  HB_Param[i][9].eng_min=0.0; HB_Param[i][9].eng_max=2.50;
      // PT405, МА-24. Давление воздуха в корпусе ЭД (точка 2) (кПа)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma24 sys=ptx05 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][10].metric, metric);
   	  strcpy(HB_Param[i][10].tags  , tags  );
   	  HB_Param[i][10].adc_min=4000; HB_Param[i][10].adc_max=20000;
   	  HB_Param[i][10].eng_min=0.0; HB_Param[i][10].eng_max=2.50;
      // PT407, МА-24. Давление воздуха возбудителя ЭД (кПа)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma24 sys=ptx07 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][11].metric, metric);
   	  strcpy(HB_Param[i][11].tags  , tags  );
   	  HB_Param[i][11].adc_min=4000; HB_Param[i][11].adc_max=20000;
   	  HB_Param[i][11].eng_min=0.0; HB_Param[i][11].eng_max=2.50;
      // FT644, ПЧ подпорного вентилятора П23D. Частота на выходе ПЧ (Гц)
   	  strcpy(metric, "freq");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma24 sys=ft64x dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][12].metric, metric);
   	  strcpy(HB_Param[i][12].tags  , tags  );
   	  HB_Param[i][12].adc_min=0; HB_Param[i][12].adc_max=10000;
   	  HB_Param[i][12].eng_min=0.0; HB_Param[i][12].eng_max=50.0;
      // CV406, МА-24. Давление контура регулирования (кПа)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma24 sys=auto dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][13].metric, metric);
   	  strcpy(HB_Param[i][13].tags  , tags  );
   	  HB_Param[i][13].adc_min=0; HB_Param[i][13].adc_max=60000;
   	  HB_Param[i][13].eng_min=0.0; HB_Param[i][13].eng_max=6.0;
      // SP4A6, МА-24. Уставка контура регулирования (алгоритм) (кПа)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma24 sys=auto dp=sp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][14].metric, metric);
   	  strcpy(HB_Param[i][14].tags  , tags  );
   	  HB_Param[i][14].adc_min=0; HB_Param[i][14].adc_max=60000;
   	  HB_Param[i][14].eng_min=0.0; HB_Param[i][14].eng_max=6.0;
      // SP4M6, МА-24. Уставка контура регулирования (оператор) (кПа)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ma24 sys=pust dp=sp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][15].metric, metric);
   	  strcpy(HB_Param[i][15].tags  , tags  );
   	  HB_Param[i][15].adc_min=0; HB_Param[i][15].adc_max=60000;
   	  HB_Param[i][15].eng_min=0.0; HB_Param[i][15].eng_max=6.0;

    	break;

    case GATEWAY_T03:
    	break;

    case GATEWAY_T04:
    	break;
										
    default:;
    }

  // инициализация сквозного массива происходит 4 раза в каждой потоковой функции

  // PT591, МПа, Давление на входе ПНС-1
//  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
//  strcpy(tags  , " node=pt591"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
//  strcpy(HB_Param_Cross[0].metric, metric);
//  strcpy(HB_Param_Cross[0].tags  , tags  );
//  HB_Param_Cross[0].adc_min=4000; HB_Param_Cross[0].adc_max=20000;
//  HB_Param_Cross[0].eng_min=0.0; HB_Param_Cross[0].eng_max=1.6;

  return 0;
  }

///-----------------------------------------------------------------------------------------------------------------
int check_header(u8 *rsp_adu, HB_Header *hdr)
  {
  hdr->header =  (rsp_adu[TCPADU_START_LO    ]<<8)|rsp_adu[TCPADU_START_LO + 1];
  hdr->cycles =  (rsp_adu[TCPADU_START_LO + 2]<<8)|rsp_adu[TCPADU_START_LO + 3];
  
  hdr->min    = ((rsp_adu[TCPADU_START_LO + 4]<<8)|rsp_adu[TCPADU_START_LO + 5]) & 0x003f;
  hdr->hour   =(((rsp_adu[TCPADU_START_LO + 4]<<8)|rsp_adu[TCPADU_START_LO + 5]) & 0x07c0) >>  6;
  hdr->day    =(((rsp_adu[TCPADU_START_LO + 4]<<8)|rsp_adu[TCPADU_START_LO + 5]) & 0xf800) >> 11;

  hdr->month  = ((rsp_adu[TCPADU_START_LO + 6]<<8)|rsp_adu[TCPADU_START_LO + 7]) & 0x000f;
  hdr->year   =(((rsp_adu[TCPADU_START_LO + 6]<<8)|rsp_adu[TCPADU_START_LO + 7]) & 0x03f0) >>  4;
  hdr->prmnum =(((rsp_adu[TCPADU_START_LO + 6]<<8)|rsp_adu[TCPADU_START_LO + 7]) & 0x7c00) >> 10;
  hdr->msprec =(((rsp_adu[TCPADU_START_LO + 6]<<8)|rsp_adu[TCPADU_START_LO + 7]) & 0x8000)>0 ? 1:0;
  
  hdr->fault  = ((rsp_adu[TCPADU_START_LO + 8]<<8)|rsp_adu[TCPADU_START_LO + 9]) |
                ((rsp_adu[TCPADU_START_LO +10]<<8)|rsp_adu[TCPADU_START_LO +11]) << 16;
  hdr->group  = ((rsp_adu[TCPADU_START_LO +12]<<8)|rsp_adu[TCPADU_START_LO +13]) |
                ((rsp_adu[TCPADU_START_LO +14]<<8)|rsp_adu[TCPADU_START_LO +15]) << 16;

  // проверка на корректность значений в заголовке буфера истории
  if((hdr->header<0 || hdr->header>=HAGENT_BUFEIP_MAX) ||
     (hdr->min   <0 || hdr->min   > 59) ||
     (hdr->hour  <0 || hdr->hour  > 23) ||
     (hdr->day   <1 || hdr->day   > 31) ||
     (hdr->month <1 || hdr->month > 12) ||
     (hdr->year  <0 || hdr->year  > 63) ||
     (hdr->prmnum<0 || hdr->prmnum> 31)  ) return 1;
     
  hdr->prmnum++;

  // формирование метки времени заголовка в формате Linux (без секунд)
  hdr->tm.tm_sec  = 0;
  hdr->tm.tm_min	= hdr->min;
  hdr->tm.tm_hour = hdr->hour;
  hdr->tm.tm_mday = hdr->day;
  hdr->tm.tm_mon  = hdr->month - 1;
  hdr->tm.tm_year = hdr->year  + 100;
  hdr->tm.tm_wday = 0;
  hdr->tm.tm_yday = 0;
  hdr->tm.tm_isdst= 0;

  hdr->tv.tv_sec = mktime(&hdr->tm);
  hdr->tv.tv_usec= 0;

  return 0;
  }
///-----------------------------------------------------------------------------------------------------------------
void  copy_header(HB_Header *src, HB_Header *dst)
  {
  dst->header = src->header;
  dst->cycles = src->cycles;
  
  dst->min   = src->min  ;
  dst->hour  = src->hour ;
  dst->day   = src->day  ;
  dst->month = src->month;
  dst->year  = src->year ;
  
  dst->prmnum = src->prmnum;
  dst->msprec = src->msprec;
  
  dst->fault  = src->fault;
  dst->group  = src->group;
  
  dst->tv.tv_sec  = src->tv.tv_sec;
  dst->tv.tv_usec = src->tv.tv_usec;
  
  return;
	}
///-----------------------------------------------------------------------------------------------------------------
void cpprm(int port, int src, int dst)
  {
	if(src==999) { // обнуление
    strcpy(HB_Param[port][dst].metric, "");
    strcpy(HB_Param[port][dst].tags  , "");
    HB_Param[port][dst].adc_min=       0;
    HB_Param[port][dst].adc_max=       0;
		HB_Param[port][dst].eng_min=       0.0;
    HB_Param[port][dst].eng_max=       0.0;
	  } else {
      strcpy(HB_Param[port][dst].metric, HB_Param_Cross[src].metric);
      strcpy(HB_Param[port][dst].tags  , HB_Param_Cross[src].tags  );
      HB_Param[port][dst].adc_min=       HB_Param_Cross[src].adc_min;
      HB_Param[port][dst].adc_max=       HB_Param_Cross[src].adc_max;
      HB_Param[port][dst].eng_min=       HB_Param_Cross[src].eng_min;
      HB_Param[port][dst].eng_max=       HB_Param_Cross[src].eng_max;
		  }

	return;
	}
///-----------------------------------------------------------------------------------------------------------------
void  cross_params(int port, unsigned long group)
  {
  int i=port-GATEWAY_T01;
 
  if(port<GATEWAY_T01 || port>GATEWAY_T04) return;

  switch(port) {

    case GATEWAY_T01:
    break; // GATEWAY_T01

    case GATEWAY_T02:
    break; // GATEWAY_T02

    case GATEWAY_T03:
    break; // GATEWAY_T03

    case GATEWAY_T04:
/* switch(group&0xff) {
       case  0: cpprm(i,999, 0); cpprm(i,999, 1); cpprm(i,999, 2); cpprm(i,999, 3);
                cpprm(i,999, 4); cpprm(i,999, 5); cpprm(i,999, 6); cpprm(i,999, 7);
                cpprm(i,999, 8); cpprm(i,999, 9); cpprm(i,999,10); cpprm(i,999,11);
                cpprm(i,999,12); cpprm(i,999,13); break;
       case  5: cpprm(i, 32, 0); cpprm(i, 33, 1); cpprm(i, 34, 2); cpprm(i, 35, 3);
                cpprm(i, 36, 4); cpprm(i, 37, 5); cpprm(i, 38, 6); cpprm(i, 39, 7);
                cpprm(i, 40, 8); cpprm(i, 41, 9); cpprm(i, 42,10); cpprm(i, 43,11);
                cpprm(i, 44,12); cpprm(i, 45,13); break;
       case 25: cpprm(i, 48, 0); cpprm(i, 49, 1); cpprm(i, 50, 2); cpprm(i, 51, 3);
                cpprm(i, 52, 4); cpprm(i, 53, 5); cpprm(i, 54, 6); cpprm(i, 55, 7);
                cpprm(i, 56, 8); cpprm(i, 57, 9); cpprm(i, 58,10); cpprm(i, 59,11);
                cpprm(i, 60,12); cpprm(i, 61,13); break;
       case 26: cpprm(i, 80, 0); cpprm(i, 81, 1); cpprm(i, 82, 2); cpprm(i, 83, 3);
                cpprm(i, 84, 4); cpprm(i, 85, 5); cpprm(i, 86, 6); cpprm(i, 87, 7);
                cpprm(i, 88, 8); cpprm(i, 89, 9); cpprm(i, 90,10); cpprm(i, 91,11);
                cpprm(i, 92,12); cpprm(i, 93,13); break;
       case 27: cpprm(i, 94, 0); cpprm(i, 95, 1); cpprm(i, 96, 2); cpprm(i, 97, 3);
                cpprm(i, 98, 4); cpprm(i, 99, 5); cpprm(i,100, 6); cpprm(i,101, 7);
                cpprm(i,102, 8); cpprm(i,103, 9); cpprm(i,104,10); cpprm(i,105,11);
                cpprm(i,106,12); cpprm(i,107,13); break;
       case  6: cpprm(i,108, 0); cpprm(i,109, 1); cpprm(i,110, 2); cpprm(i,111, 3);
                cpprm(i,112, 4); cpprm(i,113, 5); cpprm(i,114, 6); cpprm(i,115, 7);
                cpprm(i,116, 8); cpprm(i,117, 9); cpprm(i,118,10); cpprm(i,119,11);
                cpprm(i,120,12); cpprm(i,121,13); break;
       case  7: cpprm(i,122, 0); cpprm(i,123, 1); cpprm(i,124, 2); cpprm(i,125, 3);
                cpprm(i,126, 4); cpprm(i,127, 5); cpprm(i,128, 6); cpprm(i,129, 7);
                cpprm(i,130, 8); cpprm(i,131, 9); cpprm(i,132,10); cpprm(i,133,11);
                cpprm(i,134,12); cpprm(i,135,13); break;
       case  8: cpprm(i,136, 0); cpprm(i,137, 1); cpprm(i,138, 2); cpprm(i,139, 3);
                cpprm(i,140, 4); cpprm(i,141, 5); cpprm(i,142, 6); cpprm(i,143, 7);
                cpprm(i,144, 8); cpprm(i,145, 9); cpprm(i,146,10); cpprm(i,147,11);
                cpprm(i,148,12); cpprm(i,149,13); break;
        }*/
    break; // GATEWAY_T04

		default:;
    } 

  return;
	}
///-----------------------------------------------------------------------------------------------------------------

