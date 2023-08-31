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
      // PT591, МПа, Давление на входе ПНС-1
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pt591"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][0].metric, metric);
   	  strcpy(HB_Param[i][0].tags  , tags  );
   	  HB_Param[i][0].adc_min=4000; HB_Param[i][0].adc_max=20000;
   	  HB_Param[i][0].eng_min=0.0; HB_Param[i][0].eng_max=1.6;
      // PT594, МПа, Давление на входе ПНС-2
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pt594"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][1].metric, metric);
   	  strcpy(HB_Param[i][1].tags  , tags  );
   	  HB_Param[i][1].adc_min=4000; HB_Param[i][1].adc_max=20000;
   	  HB_Param[i][1].eng_min=0.0; HB_Param[i][1].eng_max=1.6;
      // PT592, МПа, Давление в коллекторе ПНС-1 (точка 1)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pt592"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][2].metric, metric);
   	  strcpy(HB_Param[i][2].tags  , tags  );
   	  HB_Param[i][2].adc_min=4000; HB_Param[i][2].adc_max=20000;
   	  HB_Param[i][2].eng_min=0.0; HB_Param[i][2].eng_max=1.6;
      // PT593, МПа, Давление в коллекторе ПНС-1 (точка 2)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pt593"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][3].metric, metric);
   	  strcpy(HB_Param[i][3].tags  , tags  );
   	  HB_Param[i][3].adc_min=4000; HB_Param[i][3].adc_max=20000;
   	  HB_Param[i][3].eng_min=0.0; HB_Param[i][3].eng_max=1.6;
      // PT001, МПа, Давление нефти до УРД (из ПНС+РП.САР)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pt001"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][4].metric, metric);
   	  strcpy(HB_Param[i][4].tags  , tags  );
   	  HB_Param[i][4].adc_min=4000; HB_Param[i][4].adc_max=20000;
   	  HB_Param[i][4].eng_min=0.0; HB_Param[i][4].eng_max=1.6;
      // PT002, МПа, Давление нефти после УРД (из ПНС+РП.САР)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pt002"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][5].metric, metric);
   	  strcpy(HB_Param[i][5].tags  , tags  );
   	  HB_Param[i][5].adc_min=4000; HB_Param[i][5].adc_max=20000;
   	  HB_Param[i][5].eng_min=0.0; HB_Param[i][5].eng_max=1.6;
      // PT901, МПа, Давление на входе в РП
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pt901"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][6].metric, metric);
   	  strcpy(HB_Param[i][6].tags  , tags  );
   	  HB_Param[i][6].adc_min=4000; HB_Param[i][6].adc_max=20000;
   	  HB_Param[i][6].eng_min=0.0; HB_Param[i][6].eng_max=1.6;
      // TT901, °С, Температура воздуха в резервуарном парке
   	  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][7].metric, metric);
   	  strcpy(HB_Param[i][7].tags  , tags  );
   	  HB_Param[i][7].adc_min=4000; HB_Param[i][7].adc_max=20000;
   	  HB_Param[i][7].eng_min=-50.0; HB_Param[i][7].eng_max=50.0;
      // PT905, МПа, Разрежение в резервуаре РВС-3000 №5
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rvs5"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][8].metric, metric);
   	  strcpy(HB_Param[i][8].tags  , tags  );
   	  HB_Param[i][8].adc_min=4000; HB_Param[i][8].adc_max=20000;
   	  HB_Param[i][8].eng_min=-0.4; HB_Param[i][8].eng_max=2.5;
      // PT906, МПа, Разрежение в резервуаре РВС-3000 №6
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rvs6"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][9].metric, metric);
   	  strcpy(HB_Param[i][9].tags  , tags  );
   	  HB_Param[i][9].adc_min=4000; HB_Param[i][9].adc_max=20000;
   	  HB_Param[i][9].eng_min=-0.4; HB_Param[i][9].eng_max=2.5;
      // PT907, МПа, Разрежение в резервуаре РВС-5000 №7
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rvs7"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][10].metric, metric);
   	  strcpy(HB_Param[i][10].tags  , tags  );
   	  HB_Param[i][10].adc_min=4000; HB_Param[i][10].adc_max=20000;
   	  HB_Param[i][10].eng_min=-0.4; HB_Param[i][10].eng_max=2.5;
      // PT908, МПа, Разрежение в резервуаре РВС-5000 №8
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rvs8"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][11].metric, metric);
   	  strcpy(HB_Param[i][11].tags  , tags  );
   	  HB_Param[i][11].adc_min=4000; HB_Param[i][11].adc_max=20000;
   	  HB_Param[i][11].eng_min=-0.4; HB_Param[i][11].eng_max=2.5;
      // QT671, %, Положение регулирующего затвора РД №5 (процент открытия)
   	  strcpy(metric, "pos");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd5 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][12].metric, metric);
   	  strcpy(HB_Param[i][12].tags  , tags  );
   	  HB_Param[i][12].adc_min=4000; HB_Param[i][12].adc_max=20000;
   	  HB_Param[i][12].eng_min=0.0; HB_Param[i][12].eng_max=100.0;
      // QT672, %, Положение регулирующего затвора РД №6 (процент открытия)
   	  strcpy(metric, "pos");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd6 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][13].metric, metric);
   	  strcpy(HB_Param[i][13].tags  , tags  );
   	  HB_Param[i][13].adc_min=4000; HB_Param[i][13].adc_max=20000;
   	  HB_Param[i][13].eng_min=0.0; HB_Param[i][13].eng_max=100.0;
      // FB671, %, Задание положения РД№5
   	  strcpy(metric, "pos");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd5 dp=sp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][14].metric, metric);
   	  strcpy(HB_Param[i][14].tags  , tags  );
   	  HB_Param[i][14].adc_min=4000; HB_Param[i][14].adc_max=20000;
   	  HB_Param[i][14].eng_min=0.0; HB_Param[i][14].eng_max=100.0;
      // FB672, %, Задание положения РД№6
   	  strcpy(metric, "pos");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd6 dp=sp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][15].metric, metric);
   	  strcpy(HB_Param[i][15].tags  , tags  );
   	  HB_Param[i][15].adc_min=4000; HB_Param[i][15].adc_max=20000;
   	  HB_Param[i][15].eng_min=0.0; HB_Param[i][15].eng_max=100.0;

    	break;

    case GATEWAY_T02:
      // SKUT_S1, см/ч, Скорость заполнения (опорожнения) РВСП-20000 №1 (из уровнемера)
   	  strcpy(metric, "vel");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rvsp1 sys=saab"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][0].metric, metric);
   	  strcpy(HB_Param[i][0].tags  , tags  );
   	  HB_Param[i][0].adc_min=0; HB_Param[i][0].adc_max=20000;
   	  HB_Param[i][0].eng_min=-10000.0; HB_Param[i][0].eng_max=10000.0;
      // SKUT_V1, см/ч, Скорость заполнения (опорожнения) РВСП-20000 №1
   	  strcpy(metric, "vel");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rvsp1 sys=calc"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][1].metric, metric);
   	  strcpy(HB_Param[i][1].tags  , tags  );
   	  HB_Param[i][1].adc_min=0; HB_Param[i][1].adc_max=20000;
   	  HB_Param[i][1].eng_min=-10000.0; HB_Param[i][1].eng_max=10000.0;
      // SKUT_L1, мм, Уровень РВСП-20000 №1
   	  strcpy(metric, "level");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rvsp1"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][2].metric, metric);
   	  strcpy(HB_Param[i][2].tags  , tags  );
   	  HB_Param[i][2].adc_min=0; HB_Param[i][2].adc_max=22000;
   	  HB_Param[i][2].eng_min=0.0; HB_Param[i][2].eng_max=22000.0;
      // SKUT_T1, °С, Средняя температура РВСП-20000 №1
   	  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rvsp1"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][3].metric, metric);
   	  strcpy(HB_Param[i][3].tags  , tags  );
   	  HB_Param[i][3].adc_min=0; HB_Param[i][3].adc_max=2000;
   	  HB_Param[i][3].eng_min=-100.0; HB_Param[i][3].eng_max=100.0;
      // SKUT_S2, см/ч, Скорость заполнения (опорожнения) РВСП-20000 №2 (из уровнемера)
   	  strcpy(metric, "vel");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rvsp2 sys=saab"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][4].metric, metric);
   	  strcpy(HB_Param[i][4].tags  , tags  );
   	  HB_Param[i][4].adc_min=0; HB_Param[i][4].adc_max=20000;
   	  HB_Param[i][4].eng_min=-10000.0; HB_Param[i][4].eng_max=10000.0;
      // SKUT_V2, см/ч, Скорость заполнения (опорожнения) РВСП-20000 №2
   	  strcpy(metric, "vel");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rvsp2 sys=calc"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][5].metric, metric);
   	  strcpy(HB_Param[i][5].tags  , tags  );
   	  HB_Param[i][5].adc_min=0; HB_Param[i][5].adc_max=20000;
   	  HB_Param[i][5].eng_min=-10000.0; HB_Param[i][5].eng_max=10000.0;
      // SKUT_L2, мм, Уровень РВСП-20000 №2
   	  strcpy(metric, "level");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rvsp2"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][6].metric, metric);
   	  strcpy(HB_Param[i][6].tags  , tags  );
   	  HB_Param[i][6].adc_min=0; HB_Param[i][6].adc_max=22000;
   	  HB_Param[i][6].eng_min=0.0; HB_Param[i][6].eng_max=22000.0;
      // SKUT_T2, °С, Средняя температура РВСП-20000 №2
   	  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rvsp2"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][7].metric, metric);
   	  strcpy(HB_Param[i][7].tags  , tags  );
   	  HB_Param[i][7].adc_min=0; HB_Param[i][7].adc_max=2000;
   	  HB_Param[i][7].eng_min=-100.0; HB_Param[i][7].eng_max=100.0;
      // SKUT_S3, см/ч, Скорость заполнения (опорожнения) РВСП-20000 №3 (из уровнемера)
   	  strcpy(metric, "vel");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rvsp3 sys=saab"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][8].metric, metric);
   	  strcpy(HB_Param[i][8].tags  , tags  );
   	  HB_Param[i][8].adc_min=0; HB_Param[i][8].adc_max=20000;
   	  HB_Param[i][8].eng_min=-10000.0; HB_Param[i][8].eng_max=10000.0;
      // SKUT_V3, см/ч, Скорость заполнения (опорожнения) РВСП-20000 №3
   	  strcpy(metric, "vel");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rvsp3 sys=calc"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][9].metric, metric);
   	  strcpy(HB_Param[i][9].tags  , tags  );
   	  HB_Param[i][9].adc_min=0; HB_Param[i][9].adc_max=20000;
   	  HB_Param[i][9].eng_min=-10000.0; HB_Param[i][9].eng_max=10000.0;
      // SKUT_L3, мм, Уровень РВСП-20000 №3
   	  strcpy(metric, "level");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rvsp3"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][10].metric, metric);
   	  strcpy(HB_Param[i][10].tags  , tags  );
   	  HB_Param[i][10].adc_min=0; HB_Param[i][10].adc_max=22000;
   	  HB_Param[i][10].eng_min=0.0; HB_Param[i][10].eng_max=22000.0;
      // SKUT_T3, °С, Средняя температура РВСП-20000 №3
   	  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rvsp3"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][11].metric, metric);
   	  strcpy(HB_Param[i][11].tags  , tags  );
   	  HB_Param[i][11].adc_min=0; HB_Param[i][11].adc_max=2000;
   	  HB_Param[i][11].eng_min=-100.0; HB_Param[i][11].eng_max=100.0;
      // SKUT_S4, см/ч, Скорость заполнения (опорожнения) РВСП-20000 №4 (из уровнемера)
   	  strcpy(metric, "vel");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rvsp4 sys=saab"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][12].metric, metric);
   	  strcpy(HB_Param[i][12].tags  , tags  );
   	  HB_Param[i][12].adc_min=0; HB_Param[i][12].adc_max=20000;
   	  HB_Param[i][12].eng_min=-10000.0; HB_Param[i][12].eng_max=10000.0;
      // SKUT_V4, см/ч, Скорость заполнения (опорожнения) РВСП-20000 №4
   	  strcpy(metric, "vel");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rvsp4 sys=calc"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][13].metric, metric);
   	  strcpy(HB_Param[i][13].tags  , tags  );
   	  HB_Param[i][13].adc_min=0; HB_Param[i][13].adc_max=20000;
   	  HB_Param[i][13].eng_min=-10000.0; HB_Param[i][13].eng_max=10000.0;
      // SKUT_L4, мм, Уровень РВСП-20000 №4
   	  strcpy(metric, "level");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rvsp4"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][14].metric, metric);
   	  strcpy(HB_Param[i][14].tags  , tags  );
   	  HB_Param[i][14].adc_min=0; HB_Param[i][14].adc_max=22000;
   	  HB_Param[i][14].eng_min=0.0; HB_Param[i][14].eng_max=22000.0;
      // SKUT_T4, °С, Средняя температура РВСП-20000 №4
   	  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rvsp4"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][15].metric, metric);
   	  strcpy(HB_Param[i][15].tags  , tags  );
   	  HB_Param[i][15].adc_min=0; HB_Param[i][15].adc_max=2000;
   	  HB_Param[i][15].eng_min=-100.0; HB_Param[i][15].eng_max=100.0;

    	break;

    case GATEWAY_T03:
      // XT553, мм/с, ПА-5. Вибрация верхнего подшипника ЭД (горизонтальное направление)
   	  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pa5 sys=vpdg"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][0].metric, metric);
   	  strcpy(HB_Param[i][0].tags  , tags  );
   	  HB_Param[i][0].adc_min=4000; HB_Param[i][0].adc_max=20000;
   	  HB_Param[i][0].eng_min=0.0; HB_Param[i][0].eng_max=30.0;
      // XT554, мм/с, ПА-5. Вибрация верхнего подшипника ЭД (поперечное направление)
   	  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pa5 sys=vpdp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][1].metric, metric);
   	  strcpy(HB_Param[i][1].tags  , tags  );
   	  HB_Param[i][1].adc_min=4000; HB_Param[i][1].adc_max=20000;
   	  HB_Param[i][1].eng_min=0.0; HB_Param[i][1].eng_max=30.0;
      // XT551, мм/с, ПА-5. Вибрация нижнего подшипника ЭД (горизонтальное направление)
   	  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pa5 sys=npdg"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][2].metric, metric);
   	  strcpy(HB_Param[i][2].tags  , tags  );
   	  HB_Param[i][2].adc_min=4000; HB_Param[i][2].adc_max=20000;
   	  HB_Param[i][2].eng_min=0.0; HB_Param[i][2].eng_max=30.0;
      // XT552, мм/с, ПА-5. Вибрация нижнего подшипника ЭД (поперечное направление)
   	  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pa5 sys=npdp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][3].metric, metric);
   	  strcpy(HB_Param[i][3].tags  , tags  );
   	  HB_Param[i][3].adc_min=4000; HB_Param[i][3].adc_max=20000;
   	  HB_Param[i][3].eng_min=0.0; HB_Param[i][3].eng_max=30.0;
      // XT555, мм/с, ПА-5. Вибрация опорно-упорного подшипника насоса (горизонтальное направление)
   	  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pa5 sys=opng"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][4].metric, metric);
   	  strcpy(HB_Param[i][4].tags  , tags  );
   	  HB_Param[i][4].adc_min=4000; HB_Param[i][4].adc_max=20000;
   	  HB_Param[i][4].eng_min=0.0; HB_Param[i][4].eng_max=30.0;
      // XT556, мм/с, ПА-5. Вибрация опорно-упорного подшипника насоса (поперечное направление)
   	  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pa5 sys=opnp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][5].metric, metric);
   	  strcpy(HB_Param[i][5].tags  , tags  );
   	  HB_Param[i][5].adc_min=4000; HB_Param[i][5].adc_max=20000;
   	  HB_Param[i][5].eng_min=0.0; HB_Param[i][5].eng_max=30.0;
      // PT551, МПа, ПА-5. Давление нефти на выходе ПНА
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pa5"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][6].metric, metric);
   	  strcpy(HB_Param[i][6].tags  , tags  );
   	  HB_Param[i][6].adc_min=4000; HB_Param[i][6].adc_max=20000;
   	  HB_Param[i][6].eng_min=0.0; HB_Param[i][6].eng_max=1.6;
      // FT551, A, ПА-5. Сила тока
   	  strcpy(metric, "curr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pa5"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][7].metric, metric);
   	  strcpy(HB_Param[i][7].tags  , tags  );
   	  HB_Param[i][7].adc_min=4000; HB_Param[i][7].adc_max=20000;
   	  HB_Param[i][7].eng_min=0.0; HB_Param[i][7].eng_max=600.0;
      // TT554, °С, ПА-5. Температура верхнего подшипника ЭД
   	  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pa5 sys=vpd"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][8].metric, metric);
   	  strcpy(HB_Param[i][8].tags  , tags  );
   	  HB_Param[i][8].adc_min=4000; HB_Param[i][8].adc_max=20000;
   	  HB_Param[i][8].eng_min=-50.0; HB_Param[i][8].eng_max=150.0;
      // TT555, °С, ПА-5. Температура масла в корпусе подшипникового узла насоса
   	  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pa5 sys=oil"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][9].metric, metric);
   	  strcpy(HB_Param[i][9].tags  , tags  );
   	  HB_Param[i][9].adc_min=4000; HB_Param[i][9].adc_max=20000;
   	  HB_Param[i][9].eng_min=-50.0; HB_Param[i][9].eng_max=150.0;
      // TT551, °С, ПА-5. Температура нефти на выходе насоса
   	  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pa5 sys=nft"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][10].metric, metric);
   	  strcpy(HB_Param[i][10].tags  , tags  );
   	  HB_Param[i][10].adc_min=4000; HB_Param[i][10].adc_max=20000;
   	  HB_Param[i][10].eng_min=-50.0; HB_Param[i][10].eng_max=150.0;
      // TT553, °С, ПА-5. Температура нижнего подшипника ЭД
   	  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pa5 sys=npd"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][11].metric, metric);
   	  strcpy(HB_Param[i][11].tags  , tags  );
   	  HB_Param[i][11].adc_min=4000; HB_Param[i][11].adc_max=20000;
   	  HB_Param[i][11].eng_min=-50.0; HB_Param[i][11].eng_max=150.0;
      // TT552-1, °С, ПА-5. Температура опорно-упорного подшипника насоса (точка 1)
   	  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pa5 sys=opn1"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][12].metric, metric);
   	  strcpy(HB_Param[i][12].tags  , tags  );
   	  HB_Param[i][12].adc_min=4000; HB_Param[i][12].adc_max=20000;
   	  HB_Param[i][12].eng_min=-50.0; HB_Param[i][12].eng_max=150.0;
      // TT552-2, °С, ПА-5. Температура опорно-упорного подшипника насоса (точка 2)
   	  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pa5 sys=opn2"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][13].metric, metric);
   	  strcpy(HB_Param[i][13].tags  , tags  );
   	  HB_Param[i][13].adc_min=4000; HB_Param[i][13].adc_max=20000;
   	  HB_Param[i][13].eng_min=-50.0; HB_Param[i][13].eng_max=150.0;
      // LT656, мм, Уровень в емкости для сбора утечек и дренажа №12
   	  strcpy(metric, "level");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=n12"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][14].metric, metric);
   	  strcpy(HB_Param[i][14].tags  , tags  );
   	  HB_Param[i][14].adc_min=4000; HB_Param[i][14].adc_max=20000;
   	  HB_Param[i][14].eng_min=0.0; HB_Param[i][14].eng_max=2400.0;
      // LT655, мм, Уровень в емкости для сбора утечек и дренажа №3
   	  strcpy(metric, "level");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=n3"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][15].metric, metric);
   	  strcpy(HB_Param[i][15].tags  , tags  );
   	  HB_Param[i][15].adc_min=4000; HB_Param[i][15].adc_max=20000;
   	  HB_Param[i][15].eng_min=0.0; HB_Param[i][15].eng_max=2400.0;

    	break;

    case GATEWAY_T04:
      // XT523, мм/с, ПА-25. Вибрация верхнего подшипника ЭД (горизонтальное направление)
   	  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pa25 sys=vpdg"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][0].metric, metric);
   	  strcpy(HB_Param[i][0].tags  , tags  );
   	  HB_Param[i][0].adc_min=4000; HB_Param[i][0].adc_max=20000;
   	  HB_Param[i][0].eng_min=0.0; HB_Param[i][0].eng_max=30.0;
      // XT524, мм/с, ПА-25. Вибрация верхнего подшипника ЭД (поперечное направление)
   	  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pa25 sys=vpdp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][1].metric, metric);
   	  strcpy(HB_Param[i][1].tags  , tags  );
   	  HB_Param[i][1].adc_min=4000; HB_Param[i][1].adc_max=20000;
   	  HB_Param[i][1].eng_min=0.0; HB_Param[i][1].eng_max=30.0;
      // XT521, мм/с, ПА-25. Вибрация нижнего подшипника ЭД (горизонтальное направление)
   	  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pa25 sys=npdg"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][2].metric, metric);
   	  strcpy(HB_Param[i][2].tags  , tags  );
   	  HB_Param[i][2].adc_min=4000; HB_Param[i][2].adc_max=20000;
   	  HB_Param[i][2].eng_min=0.0; HB_Param[i][2].eng_max=30.0;
      // XT522, мм/с, ПА-25. Вибрация нижнего подшипника ЭД (поперечное направление)
   	  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pa25 sys=npdp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][3].metric, metric);
   	  strcpy(HB_Param[i][3].tags  , tags  );
   	  HB_Param[i][3].adc_min=4000; HB_Param[i][3].adc_max=20000;
   	  HB_Param[i][3].eng_min=0.0; HB_Param[i][3].eng_max=30.0;
      // XT525, мм/с, ПА-25. Вибрация опорно-упорного подшипника насоса (горизонтальное направление)
   	  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pa25 sys=opng"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][4].metric, metric);
   	  strcpy(HB_Param[i][4].tags  , tags  );
   	  HB_Param[i][4].adc_min=4000; HB_Param[i][4].adc_max=20000;
   	  HB_Param[i][4].eng_min=0.0; HB_Param[i][4].eng_max=30.0;
      // XT526, мм/с, ПА-25. Вибрация опорно-упорного подшипника насоса (поперечное направление)
   	  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pa25 sys=opnp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][5].metric, metric);
   	  strcpy(HB_Param[i][5].tags  , tags  );
   	  HB_Param[i][5].adc_min=4000; HB_Param[i][5].adc_max=20000;
   	  HB_Param[i][5].eng_min=0.0; HB_Param[i][5].eng_max=30.0;
      // PT521, МПа, ПА-25. Давление нефти на выходе ПНА
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pa25"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][6].metric, metric);
   	  strcpy(HB_Param[i][6].tags  , tags  );
   	  HB_Param[i][6].adc_min=4000; HB_Param[i][6].adc_max=20000;
   	  HB_Param[i][6].eng_min=0.0; HB_Param[i][6].eng_max=1.6;
      // FT521, A, ПА-25. Сила тока
   	  strcpy(metric, "curr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pa25"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][7].metric, metric);
   	  strcpy(HB_Param[i][7].tags  , tags  );
   	  HB_Param[i][7].adc_min=4000; HB_Param[i][7].adc_max=20000;
   	  HB_Param[i][7].eng_min=0.0; HB_Param[i][7].eng_max=600.0;
      // TT524, °С, ПА-25. Температура верхнего подшипника ЭД
   	  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pa25 sys=vpd"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][8].metric, metric);
   	  strcpy(HB_Param[i][8].tags  , tags  );
   	  HB_Param[i][8].adc_min=4000; HB_Param[i][8].adc_max=20000;
   	  HB_Param[i][8].eng_min=-50.0; HB_Param[i][8].eng_max=150.0;
      // TT525, °С, ПА-25. Температура масла в корпусе подшипникового узла насоса
   	  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pa25 sys=oil"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][9].metric, metric);
   	  strcpy(HB_Param[i][9].tags  , tags  );
   	  HB_Param[i][9].adc_min=4000; HB_Param[i][9].adc_max=20000;
   	  HB_Param[i][9].eng_min=-50.0; HB_Param[i][9].eng_max=150.0;
      // TT521, °С, ПА-25. Температура нефти на выходе насоса
   	  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pa25 sys=nft"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][10].metric, metric);
   	  strcpy(HB_Param[i][10].tags  , tags  );
   	  HB_Param[i][10].adc_min=4000; HB_Param[i][10].adc_max=20000;
   	  HB_Param[i][10].eng_min=-50.0; HB_Param[i][10].eng_max=150.0;
      // TT523, °С, ПА-25. Температура нижнего подшипника ЭД
   	  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pa25 sys=npd"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][11].metric, metric);
   	  strcpy(HB_Param[i][11].tags  , tags  );
   	  HB_Param[i][11].adc_min=4000; HB_Param[i][11].adc_max=20000;
   	  HB_Param[i][11].eng_min=-50.0; HB_Param[i][11].eng_max=150.0;
      // TT522-1, °С, ПА-25. Температура опорно-упорного подшипника насоса (точка 1)
   	  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pa25 sys=opn1"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][12].metric, metric);
   	  strcpy(HB_Param[i][12].tags  , tags  );
   	  HB_Param[i][12].adc_min=4000; HB_Param[i][12].adc_max=20000;
   	  HB_Param[i][12].eng_min=-50.0; HB_Param[i][12].eng_max=150.0;
      // TT522-2, °С, ПА-25. Температура опорно-упорного подшипника насоса (точка 2)
   	  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=pa25 sys=opn2"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][13].metric, metric);
   	  strcpy(HB_Param[i][13].tags  , tags  );
   	  HB_Param[i][13].adc_min=4000; HB_Param[i][13].adc_max=20000;
   	  HB_Param[i][13].eng_min=-50.0; HB_Param[i][13].eng_max=150.0;
      // PT656, МПа, Давление на выходе погружного насоса Н-12
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=n12"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][14].metric, metric);
   	  strcpy(HB_Param[i][14].tags  , tags  );
   	  HB_Param[i][14].adc_min=4000; HB_Param[i][14].adc_max=20000;
   	  HB_Param[i][14].eng_min=0.0; HB_Param[i][14].eng_max=2.5;
      // PT655, МПа, Давление на выходе погружного насоса Н-3
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=n3"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][15].metric, metric);
   	  strcpy(HB_Param[i][15].tags  , tags  );
   	  HB_Param[i][15].adc_min=4000; HB_Param[i][15].adc_max=20000;
   	  HB_Param[i][15].eng_min=0.0; HB_Param[i][15].eng_max=2.5;

    	break;
										
    default:;
    }

  // инициализация сквозного массива происходит 4 раза в каждой потоковой функции

  // PT591, МПа, Давление на входе ПНС-1
  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pt591"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[0].metric, metric);
  strcpy(HB_Param_Cross[0].tags  , tags  );
  HB_Param_Cross[0].adc_min=4000; HB_Param_Cross[0].adc_max=20000;
  HB_Param_Cross[0].eng_min=0.0; HB_Param_Cross[0].eng_max=1.6;
  // PT594, МПа, Давление на входе ПНС-2
  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pt594"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[1].metric, metric);
  strcpy(HB_Param_Cross[1].tags  , tags  );
  HB_Param_Cross[1].adc_min=4000; HB_Param_Cross[1].adc_max=20000;
  HB_Param_Cross[1].eng_min=0.0; HB_Param_Cross[1].eng_max=1.6;
  // PT592, МПа, Давление в коллекторе ПНС-1 (точка 1)
  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pt592"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[2].metric, metric);
  strcpy(HB_Param_Cross[2].tags  , tags  );
  HB_Param_Cross[2].adc_min=4000; HB_Param_Cross[2].adc_max=20000;
  HB_Param_Cross[2].eng_min=0.0; HB_Param_Cross[2].eng_max=1.6;
  // PT593, МПа, Давление в коллекторе ПНС-1 (точка 2)
  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pt593"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[3].metric, metric);
  strcpy(HB_Param_Cross[3].tags  , tags  );
  HB_Param_Cross[3].adc_min=4000; HB_Param_Cross[3].adc_max=20000;
  HB_Param_Cross[3].eng_min=0.0; HB_Param_Cross[3].eng_max=1.6;
  // PT001, МПа, Давление нефти до УРД (из ПНС+РП.САР)
  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pt001"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[4].metric, metric);
  strcpy(HB_Param_Cross[4].tags  , tags  );
  HB_Param_Cross[4].adc_min=4000; HB_Param_Cross[4].adc_max=20000;
  HB_Param_Cross[4].eng_min=0.0; HB_Param_Cross[4].eng_max=1.6;
  // PT002, МПа, Давление нефти после УРД (из ПНС+РП.САР)
  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pt002"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[5].metric, metric);
  strcpy(HB_Param_Cross[5].tags  , tags  );
  HB_Param_Cross[5].adc_min=4000; HB_Param_Cross[5].adc_max=20000;
  HB_Param_Cross[5].eng_min=0.0; HB_Param_Cross[5].eng_max=1.6;
  // PT901, МПа, Давление на входе в РП
  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pt901"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[6].metric, metric);
  strcpy(HB_Param_Cross[6].tags  , tags  );
  HB_Param_Cross[6].adc_min=4000; HB_Param_Cross[6].adc_max=20000;
  HB_Param_Cross[6].eng_min=0.0; HB_Param_Cross[6].eng_max=1.6;
  // TT901, °С, Температура воздуха в резервуарном парке
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[7].metric, metric);
  strcpy(HB_Param_Cross[7].tags  , tags  );
  HB_Param_Cross[7].adc_min=4000; HB_Param_Cross[7].adc_max=20000;
  HB_Param_Cross[7].eng_min=-50.0; HB_Param_Cross[7].eng_max=50.0;
  // PT905, МПа, Разрежение в резервуаре РВС-3000 №5
  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvs5"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[8].metric, metric);
  strcpy(HB_Param_Cross[8].tags  , tags  );
  HB_Param_Cross[8].adc_min=4000; HB_Param_Cross[8].adc_max=20000;
  HB_Param_Cross[8].eng_min=-0.4; HB_Param_Cross[8].eng_max=2.5;
  // PT906, МПа, Разрежение в резервуаре РВС-3000 №6
  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvs6"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[9].metric, metric);
  strcpy(HB_Param_Cross[9].tags  , tags  );
  HB_Param_Cross[9].adc_min=4000; HB_Param_Cross[9].adc_max=20000;
  HB_Param_Cross[9].eng_min=-0.4; HB_Param_Cross[9].eng_max=2.5;
  // PT907, МПа, Разрежение в резервуаре РВС-5000 №7
  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvs7"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[10].metric, metric);
  strcpy(HB_Param_Cross[10].tags  , tags  );
  HB_Param_Cross[10].adc_min=4000; HB_Param_Cross[10].adc_max=20000;
  HB_Param_Cross[10].eng_min=-0.4; HB_Param_Cross[10].eng_max=2.5;
  // PT908, МПа, Разрежение в резервуаре РВС-5000 №8
  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvs8"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[11].metric, metric);
  strcpy(HB_Param_Cross[11].tags  , tags  );
  HB_Param_Cross[11].adc_min=4000; HB_Param_Cross[11].adc_max=20000;
  HB_Param_Cross[11].eng_min=-0.4; HB_Param_Cross[11].eng_max=2.5;
  // QT671, %, Положение регулирующего затвора РД №5 (процент открытия)
  strcpy(metric, "pos");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rd5 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[12].metric, metric);
  strcpy(HB_Param_Cross[12].tags  , tags  );
  HB_Param_Cross[12].adc_min=4000; HB_Param_Cross[12].adc_max=20000;
  HB_Param_Cross[12].eng_min=0.0; HB_Param_Cross[12].eng_max=100.0;
  // QT672, %, Положение регулирующего затвора РД №6 (процент открытия)
  strcpy(metric, "pos");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rd6 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[13].metric, metric);
  strcpy(HB_Param_Cross[13].tags  , tags  );
  HB_Param_Cross[13].adc_min=4000; HB_Param_Cross[13].adc_max=20000;
  HB_Param_Cross[13].eng_min=0.0; HB_Param_Cross[13].eng_max=100.0;
  // FB671, %, Задание положения РД№5
  strcpy(metric, "pos");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rd5 dp=sp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[14].metric, metric);
  strcpy(HB_Param_Cross[14].tags  , tags  );
  HB_Param_Cross[14].adc_min=4000; HB_Param_Cross[14].adc_max=20000;
  HB_Param_Cross[14].eng_min=0.0; HB_Param_Cross[14].eng_max=100.0;
  // FB672, %, Задание положения РД№6
  strcpy(metric, "pos");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rd6 dp=sp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[15].metric, metric);
  strcpy(HB_Param_Cross[15].tags  , tags  );
  HB_Param_Cross[15].adc_min=4000; HB_Param_Cross[15].adc_max=20000;
  HB_Param_Cross[15].eng_min=0.0; HB_Param_Cross[15].eng_max=100.0;
  // SKUT_S1, см/ч, Скорость заполнения (опорожнения) РВСП-20000 №1 (из уровнемера)
  strcpy(metric, "vel");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvsp1 sys=saab"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[16].metric, metric);
  strcpy(HB_Param_Cross[16].tags  , tags  );
  HB_Param_Cross[16].adc_min=0; HB_Param_Cross[16].adc_max=20000;
  HB_Param_Cross[16].eng_min=-10000.0; HB_Param_Cross[16].eng_max=10000.0;
  // SKUT_V1, см/ч, Скорость заполнения (опорожнения) РВСП-20000 №1
  strcpy(metric, "vel");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvsp1 sys=calc"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[17].metric, metric);
  strcpy(HB_Param_Cross[17].tags  , tags  );
  HB_Param_Cross[17].adc_min=0; HB_Param_Cross[17].adc_max=20000;
  HB_Param_Cross[17].eng_min=-10000.0; HB_Param_Cross[17].eng_max=10000.0;
  // SKUT_L1, мм, Уровень РВСП-20000 №1
  strcpy(metric, "level");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvsp1"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[18].metric, metric);
  strcpy(HB_Param_Cross[18].tags  , tags  );
  HB_Param_Cross[18].adc_min=0; HB_Param_Cross[18].adc_max=22000;
  HB_Param_Cross[18].eng_min=0.0; HB_Param_Cross[18].eng_max=22000.0;
  // SKUT_T1, °С, Средняя температура РВСП-20000 №1
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvsp1"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[19].metric, metric);
  strcpy(HB_Param_Cross[19].tags  , tags  );
  HB_Param_Cross[19].adc_min=0; HB_Param_Cross[19].adc_max=2000;
  HB_Param_Cross[19].eng_min=-100.0; HB_Param_Cross[19].eng_max=100.0;
  // SKUT_S2, см/ч, Скорость заполнения (опорожнения) РВСП-20000 №2 (из уровнемера)
  strcpy(metric, "vel");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvsp2 sys=saab"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[20].metric, metric);
  strcpy(HB_Param_Cross[20].tags  , tags  );
  HB_Param_Cross[20].adc_min=0; HB_Param_Cross[20].adc_max=20000;
  HB_Param_Cross[20].eng_min=-10000.0; HB_Param_Cross[20].eng_max=10000.0;
  // SKUT_V2, см/ч, Скорость заполнения (опорожнения) РВСП-20000 №2
  strcpy(metric, "vel");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvsp2 sys=calc"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[21].metric, metric);
  strcpy(HB_Param_Cross[21].tags  , tags  );
  HB_Param_Cross[21].adc_min=0; HB_Param_Cross[21].adc_max=20000;
  HB_Param_Cross[21].eng_min=-10000.0; HB_Param_Cross[21].eng_max=10000.0;
  // SKUT_L2, мм, Уровень РВСП-20000 №2
  strcpy(metric, "level");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvsp2"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[22].metric, metric);
  strcpy(HB_Param_Cross[22].tags  , tags  );
  HB_Param_Cross[22].adc_min=0; HB_Param_Cross[22].adc_max=22000;
  HB_Param_Cross[22].eng_min=0.0; HB_Param_Cross[22].eng_max=22000.0;
  // SKUT_T2, °С, Средняя температура РВСП-20000 №2
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvsp2"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[23].metric, metric);
  strcpy(HB_Param_Cross[23].tags  , tags  );
  HB_Param_Cross[23].adc_min=0; HB_Param_Cross[23].adc_max=2000;
  HB_Param_Cross[23].eng_min=-100.0; HB_Param_Cross[23].eng_max=100.0;
  // SKUT_S3, см/ч, Скорость заполнения (опорожнения) РВСП-20000 №3 (из уровнемера)
  strcpy(metric, "vel");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvsp3 sys=saab"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[24].metric, metric);
  strcpy(HB_Param_Cross[24].tags  , tags  );
  HB_Param_Cross[24].adc_min=0; HB_Param_Cross[24].adc_max=20000;
  HB_Param_Cross[24].eng_min=-10000.0; HB_Param_Cross[24].eng_max=10000.0;
  // SKUT_V3, см/ч, Скорость заполнения (опорожнения) РВСП-20000 №3
  strcpy(metric, "vel");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvsp3 sys=calc"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[25].metric, metric);
  strcpy(HB_Param_Cross[25].tags  , tags  );
  HB_Param_Cross[25].adc_min=0; HB_Param_Cross[25].adc_max=20000;
  HB_Param_Cross[25].eng_min=-10000.0; HB_Param_Cross[25].eng_max=10000.0;
  // SKUT_L3, мм, Уровень РВСП-20000 №3
  strcpy(metric, "level");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvsp3"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[26].metric, metric);
  strcpy(HB_Param_Cross[26].tags  , tags  );
  HB_Param_Cross[26].adc_min=0; HB_Param_Cross[26].adc_max=22000;
  HB_Param_Cross[26].eng_min=0.0; HB_Param_Cross[26].eng_max=22000.0;
  // SKUT_T3, °С, Средняя температура РВСП-20000 №3
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvsp3"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[27].metric, metric);
  strcpy(HB_Param_Cross[27].tags  , tags  );
  HB_Param_Cross[27].adc_min=0; HB_Param_Cross[27].adc_max=2000;
  HB_Param_Cross[27].eng_min=-100.0; HB_Param_Cross[27].eng_max=100.0;
  // SKUT_S4, см/ч, Скорость заполнения (опорожнения) РВСП-20000 №4 (из уровнемера)
  strcpy(metric, "vel");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvsp4 sys=saab"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[28].metric, metric);
  strcpy(HB_Param_Cross[28].tags  , tags  );
  HB_Param_Cross[28].adc_min=0; HB_Param_Cross[28].adc_max=20000;
  HB_Param_Cross[28].eng_min=-10000.0; HB_Param_Cross[28].eng_max=10000.0;
  // SKUT_V4, см/ч, Скорость заполнения (опорожнения) РВСП-20000 №4
  strcpy(metric, "vel");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvsp4 sys=calc"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[29].metric, metric);
  strcpy(HB_Param_Cross[29].tags  , tags  );
  HB_Param_Cross[29].adc_min=0; HB_Param_Cross[29].adc_max=20000;
  HB_Param_Cross[29].eng_min=-10000.0; HB_Param_Cross[29].eng_max=10000.0;
  // SKUT_L4, мм, Уровень РВСП-20000 №4
  strcpy(metric, "level");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvsp4"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[30].metric, metric);
  strcpy(HB_Param_Cross[30].tags  , tags  );
  HB_Param_Cross[30].adc_min=0; HB_Param_Cross[30].adc_max=22000;
  HB_Param_Cross[30].eng_min=0.0; HB_Param_Cross[30].eng_max=22000.0;
  // SKUT_T4, °С, Средняя температура РВСП-20000 №4
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvsp4"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[31].metric, metric);
  strcpy(HB_Param_Cross[31].tags  , tags  );
  HB_Param_Cross[31].adc_min=0; HB_Param_Cross[31].adc_max=2000;
  HB_Param_Cross[31].eng_min=-100.0; HB_Param_Cross[31].eng_max=100.0;
  // XT553, мм/с, ПА-5. Вибрация верхнего подшипника ЭД (горизонтальное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa5 sys=vpdg"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[32].metric, metric);
  strcpy(HB_Param_Cross[32].tags  , tags  );
  HB_Param_Cross[32].adc_min=4000; HB_Param_Cross[32].adc_max=20000;
  HB_Param_Cross[32].eng_min=0.0; HB_Param_Cross[32].eng_max=30.0;
  // XT554, мм/с, ПА-5. Вибрация верхнего подшипника ЭД (поперечное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa5 sys=vpdp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[33].metric, metric);
  strcpy(HB_Param_Cross[33].tags  , tags  );
  HB_Param_Cross[33].adc_min=4000; HB_Param_Cross[33].adc_max=20000;
  HB_Param_Cross[33].eng_min=0.0; HB_Param_Cross[33].eng_max=30.0;
  // XT551, мм/с, ПА-5. Вибрация нижнего подшипника ЭД (горизонтальное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa5 sys=npdg"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[34].metric, metric);
  strcpy(HB_Param_Cross[34].tags  , tags  );
  HB_Param_Cross[34].adc_min=4000; HB_Param_Cross[34].adc_max=20000;
  HB_Param_Cross[34].eng_min=0.0; HB_Param_Cross[34].eng_max=30.0;
  // XT552, мм/с, ПА-5. Вибрация нижнего подшипника ЭД (поперечное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa5 sys=npdp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[35].metric, metric);
  strcpy(HB_Param_Cross[35].tags  , tags  );
  HB_Param_Cross[35].adc_min=4000; HB_Param_Cross[35].adc_max=20000;
  HB_Param_Cross[35].eng_min=0.0; HB_Param_Cross[35].eng_max=30.0;
  // XT555, мм/с, ПА-5. Вибрация опорно-упорного подшипника насоса (горизонтальное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa5 sys=opng"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[36].metric, metric);
  strcpy(HB_Param_Cross[36].tags  , tags  );
  HB_Param_Cross[36].adc_min=4000; HB_Param_Cross[36].adc_max=20000;
  HB_Param_Cross[36].eng_min=0.0; HB_Param_Cross[36].eng_max=30.0;
  // XT556, мм/с, ПА-5. Вибрация опорно-упорного подшипника насоса (поперечное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa5 sys=opnp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[37].metric, metric);
  strcpy(HB_Param_Cross[37].tags  , tags  );
  HB_Param_Cross[37].adc_min=4000; HB_Param_Cross[37].adc_max=20000;
  HB_Param_Cross[37].eng_min=0.0; HB_Param_Cross[37].eng_max=30.0;
  // PT551, МПа, ПА-5. Давление нефти на выходе ПНА
  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa5"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[38].metric, metric);
  strcpy(HB_Param_Cross[38].tags  , tags  );
  HB_Param_Cross[38].adc_min=4000; HB_Param_Cross[38].adc_max=20000;
  HB_Param_Cross[38].eng_min=0.0; HB_Param_Cross[38].eng_max=1.6;
  // FT551, A, ПА-5. Сила тока
  strcpy(metric, "curr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa5"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[39].metric, metric);
  strcpy(HB_Param_Cross[39].tags  , tags  );
  HB_Param_Cross[39].adc_min=4000; HB_Param_Cross[39].adc_max=20000;
  HB_Param_Cross[39].eng_min=0.0; HB_Param_Cross[39].eng_max=600.0;
  // TT554, °С, ПА-5. Температура верхнего подшипника ЭД
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa5 sys=vpd"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[40].metric, metric);
  strcpy(HB_Param_Cross[40].tags  , tags  );
  HB_Param_Cross[40].adc_min=4000; HB_Param_Cross[40].adc_max=20000;
  HB_Param_Cross[40].eng_min=-50.0; HB_Param_Cross[40].eng_max=150.0;
  // TT555, °С, ПА-5. Температура масла в корпусе подшипникового узла насоса
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa5 sys=oil"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[41].metric, metric);
  strcpy(HB_Param_Cross[41].tags  , tags  );
  HB_Param_Cross[41].adc_min=4000; HB_Param_Cross[41].adc_max=20000;
  HB_Param_Cross[41].eng_min=-50.0; HB_Param_Cross[41].eng_max=150.0;
  // TT551, °С, ПА-5. Температура нефти на выходе насоса
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa5 sys=nft"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[42].metric, metric);
  strcpy(HB_Param_Cross[42].tags  , tags  );
  HB_Param_Cross[42].adc_min=4000; HB_Param_Cross[42].adc_max=20000;
  HB_Param_Cross[42].eng_min=-50.0; HB_Param_Cross[42].eng_max=150.0;
  // TT553, °С, ПА-5. Температура нижнего подшипника ЭД
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa5 sys=npd"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[43].metric, metric);
  strcpy(HB_Param_Cross[43].tags  , tags  );
  HB_Param_Cross[43].adc_min=4000; HB_Param_Cross[43].adc_max=20000;
  HB_Param_Cross[43].eng_min=-50.0; HB_Param_Cross[43].eng_max=150.0;
  // TT552-1, °С, ПА-5. Температура опорно-упорного подшипника насоса (точка 1)
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa5 sys=opn1"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[44].metric, metric);
  strcpy(HB_Param_Cross[44].tags  , tags  );
  HB_Param_Cross[44].adc_min=4000; HB_Param_Cross[44].adc_max=20000;
  HB_Param_Cross[44].eng_min=-50.0; HB_Param_Cross[44].eng_max=150.0;
  // TT552-2, °С, ПА-5. Температура опорно-упорного подшипника насоса (точка 2)
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa5 sys=opn2"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[45].metric, metric);
  strcpy(HB_Param_Cross[45].tags  , tags  );
  HB_Param_Cross[45].adc_min=4000; HB_Param_Cross[45].adc_max=20000;
  HB_Param_Cross[45].eng_min=-50.0; HB_Param_Cross[45].eng_max=150.0;
  // LT656, мм, Уровень в емкости для сбора утечек и дренажа №12
  strcpy(metric, "level");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=n12"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[46].metric, metric);
  strcpy(HB_Param_Cross[46].tags  , tags  );
  HB_Param_Cross[46].adc_min=4000; HB_Param_Cross[46].adc_max=20000;
  HB_Param_Cross[46].eng_min=0.0; HB_Param_Cross[46].eng_max=2400.0;
  // LT655, мм, Уровень в емкости для сбора утечек и дренажа №3
  strcpy(metric, "level");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=n3"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[47].metric, metric);
  strcpy(HB_Param_Cross[47].tags  , tags  );
  HB_Param_Cross[47].adc_min=4000; HB_Param_Cross[47].adc_max=20000;
  HB_Param_Cross[47].eng_min=0.0; HB_Param_Cross[47].eng_max=2400.0;
  // XT523, мм/с, ПА-25. Вибрация верхнего подшипника ЭД (горизонтальное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa25 sys=vpdg"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[48].metric, metric);
  strcpy(HB_Param_Cross[48].tags  , tags  );
  HB_Param_Cross[48].adc_min=4000; HB_Param_Cross[48].adc_max=20000;
  HB_Param_Cross[48].eng_min=0.0; HB_Param_Cross[48].eng_max=30.0;
  // XT524, мм/с, ПА-25. Вибрация верхнего подшипника ЭД (поперечное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa25 sys=vpdp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[49].metric, metric);
  strcpy(HB_Param_Cross[49].tags  , tags  );
  HB_Param_Cross[49].adc_min=4000; HB_Param_Cross[49].adc_max=20000;
  HB_Param_Cross[49].eng_min=0.0; HB_Param_Cross[49].eng_max=30.0;
  // XT521, мм/с, ПА-25. Вибрация нижнего подшипника ЭД (горизонтальное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa25 sys=npdg"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[50].metric, metric);
  strcpy(HB_Param_Cross[50].tags  , tags  );
  HB_Param_Cross[50].adc_min=4000; HB_Param_Cross[50].adc_max=20000;
  HB_Param_Cross[50].eng_min=0.0; HB_Param_Cross[50].eng_max=30.0;
  // XT522, мм/с, ПА-25. Вибрация нижнего подшипника ЭД (поперечное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa25 sys=npdp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[51].metric, metric);
  strcpy(HB_Param_Cross[51].tags  , tags  );
  HB_Param_Cross[51].adc_min=4000; HB_Param_Cross[51].adc_max=20000;
  HB_Param_Cross[51].eng_min=0.0; HB_Param_Cross[51].eng_max=30.0;
  // XT525, мм/с, ПА-25. Вибрация опорно-упорного подшипника насоса (горизонтальное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa25 sys=opng"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[52].metric, metric);
  strcpy(HB_Param_Cross[52].tags  , tags  );
  HB_Param_Cross[52].adc_min=4000; HB_Param_Cross[52].adc_max=20000;
  HB_Param_Cross[52].eng_min=0.0; HB_Param_Cross[52].eng_max=30.0;
  // XT526, мм/с, ПА-25. Вибрация опорно-упорного подшипника насоса (поперечное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa25 sys=opnp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[53].metric, metric);
  strcpy(HB_Param_Cross[53].tags  , tags  );
  HB_Param_Cross[53].adc_min=4000; HB_Param_Cross[53].adc_max=20000;
  HB_Param_Cross[53].eng_min=0.0; HB_Param_Cross[53].eng_max=30.0;
  // PT521, МПа, ПА-25. Давление нефти на выходе ПНА
  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa25"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[54].metric, metric);
  strcpy(HB_Param_Cross[54].tags  , tags  );
  HB_Param_Cross[54].adc_min=4000; HB_Param_Cross[54].adc_max=20000;
  HB_Param_Cross[54].eng_min=0.0; HB_Param_Cross[54].eng_max=1.6;
  // FT521, A, ПА-25. Сила тока
  strcpy(metric, "curr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa25"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[55].metric, metric);
  strcpy(HB_Param_Cross[55].tags  , tags  );
  HB_Param_Cross[55].adc_min=4000; HB_Param_Cross[55].adc_max=20000;
  HB_Param_Cross[55].eng_min=0.0; HB_Param_Cross[55].eng_max=600.0;
  // TT524, °С, ПА-25. Температура верхнего подшипника ЭД
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa25 sys=vpd"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[56].metric, metric);
  strcpy(HB_Param_Cross[56].tags  , tags  );
  HB_Param_Cross[56].adc_min=4000; HB_Param_Cross[56].adc_max=20000;
  HB_Param_Cross[56].eng_min=-50.0; HB_Param_Cross[56].eng_max=150.0;
  // TT525, °С, ПА-25. Температура масла в корпусе подшипникового узла насоса
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa25 sys=oil"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[57].metric, metric);
  strcpy(HB_Param_Cross[57].tags  , tags  );
  HB_Param_Cross[57].adc_min=4000; HB_Param_Cross[57].adc_max=20000;
  HB_Param_Cross[57].eng_min=-50.0; HB_Param_Cross[57].eng_max=150.0;
  // TT521, °С, ПА-25. Температура нефти на выходе насоса
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa25 sys=nft"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[58].metric, metric);
  strcpy(HB_Param_Cross[58].tags  , tags  );
  HB_Param_Cross[58].adc_min=4000; HB_Param_Cross[58].adc_max=20000;
  HB_Param_Cross[58].eng_min=-50.0; HB_Param_Cross[58].eng_max=150.0;
  // TT523, °С, ПА-25. Температура нижнего подшипника ЭД
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa25 sys=npd"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[59].metric, metric);
  strcpy(HB_Param_Cross[59].tags  , tags  );
  HB_Param_Cross[59].adc_min=4000; HB_Param_Cross[59].adc_max=20000;
  HB_Param_Cross[59].eng_min=-50.0; HB_Param_Cross[59].eng_max=150.0;
  // TT522-1, °С, ПА-25. Температура опорно-упорного подшипника насоса (точка 1)
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa25 sys=opn1"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[60].metric, metric);
  strcpy(HB_Param_Cross[60].tags  , tags  );
  HB_Param_Cross[60].adc_min=4000; HB_Param_Cross[60].adc_max=20000;
  HB_Param_Cross[60].eng_min=-50.0; HB_Param_Cross[60].eng_max=150.0;
  // TT522-2, °С, ПА-25. Температура опорно-упорного подшипника насоса (точка 2)
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa25 sys=opn2"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[61].metric, metric);
  strcpy(HB_Param_Cross[61].tags  , tags  );
  HB_Param_Cross[61].adc_min=4000; HB_Param_Cross[61].adc_max=20000;
  HB_Param_Cross[61].eng_min=-50.0; HB_Param_Cross[61].eng_max=150.0;
  // PT656, МПа, Давление на выходе погружного насоса Н-12
  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=n12"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[62].metric, metric);
  strcpy(HB_Param_Cross[62].tags  , tags  );
  HB_Param_Cross[62].adc_min=4000; HB_Param_Cross[62].adc_max=20000;
  HB_Param_Cross[62].eng_min=0.0; HB_Param_Cross[62].eng_max=2.5;
  // PT655, МПа, Давление на выходе погружного насоса Н-3
  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=n3"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[63].metric, metric);
  strcpy(HB_Param_Cross[63].tags  , tags  );
  HB_Param_Cross[63].adc_min=4000; HB_Param_Cross[63].adc_max=20000;
  HB_Param_Cross[63].eng_min=0.0; HB_Param_Cross[63].eng_max=2.5;
  // SKUT_S5, см/ч, Скорость заполнения (опорожнения) РВС-3000 №5 (из уровнемера)
  strcpy(metric, "vel");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvs5 sys=saab"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[64].metric, metric);
  strcpy(HB_Param_Cross[64].tags  , tags  );
  HB_Param_Cross[64].adc_min=0; HB_Param_Cross[64].adc_max=20000;
  HB_Param_Cross[64].eng_min=-10000.0; HB_Param_Cross[64].eng_max=10000.0;
  // SKUT_V5, см/ч, Скорость заполнения (опорожнения) РВС-3000 №5
  strcpy(metric, "vel");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvs5 sys=calc"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[65].metric, metric);
  strcpy(HB_Param_Cross[65].tags  , tags  );
  HB_Param_Cross[65].adc_min=0; HB_Param_Cross[65].adc_max=20000;
  HB_Param_Cross[65].eng_min=-10000.0; HB_Param_Cross[65].eng_max=10000.0;
  // SKUT_L5, мм, Уровень РВС-3000 №5
  strcpy(metric, "level");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvs5"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[66].metric, metric);
  strcpy(HB_Param_Cross[66].tags  , tags  );
  HB_Param_Cross[66].adc_min=0; HB_Param_Cross[66].adc_max=22000;
  HB_Param_Cross[66].eng_min=0.0; HB_Param_Cross[66].eng_max=22000.0;
  // SKUT_T5, °С, Средняя температура РВС-3000 №5
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvs5"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[67].metric, metric);
  strcpy(HB_Param_Cross[67].tags  , tags  );
  HB_Param_Cross[67].adc_min=0; HB_Param_Cross[67].adc_max=2000;
  HB_Param_Cross[67].eng_min=-100.0; HB_Param_Cross[67].eng_max=100.0;
  // SKUT_S6, см/ч, Скорость заполнения (опорожнения) РВС-3000 №6 (из уровнемера)
  strcpy(metric, "vel");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvs6 sys=saab"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[68].metric, metric);
  strcpy(HB_Param_Cross[68].tags  , tags  );
  HB_Param_Cross[68].adc_min=0; HB_Param_Cross[68].adc_max=20000;
  HB_Param_Cross[68].eng_min=-10000.0; HB_Param_Cross[68].eng_max=10000.0;
  // SKUT_V6, см/ч, Скорость заполнения (опорожнения) РВС-3000 №6
  strcpy(metric, "vel");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvs6 sys=calc"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[69].metric, metric);
  strcpy(HB_Param_Cross[69].tags  , tags  );
  HB_Param_Cross[69].adc_min=0; HB_Param_Cross[69].adc_max=20000;
  HB_Param_Cross[69].eng_min=-10000.0; HB_Param_Cross[69].eng_max=10000.0;
  // SKUT_L6, мм, Уровень РВС-3000 №6
  strcpy(metric, "level");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvs6"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[70].metric, metric);
  strcpy(HB_Param_Cross[70].tags  , tags  );
  HB_Param_Cross[70].adc_min=0; HB_Param_Cross[70].adc_max=22000;
  HB_Param_Cross[70].eng_min=0.0; HB_Param_Cross[70].eng_max=22000.0;
  // SKUT_T6, °С, Средняя температура РВС-3000 №6
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvs6"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[71].metric, metric);
  strcpy(HB_Param_Cross[71].tags  , tags  );
  HB_Param_Cross[71].adc_min=0; HB_Param_Cross[71].adc_max=2000;
  HB_Param_Cross[71].eng_min=-100.0; HB_Param_Cross[71].eng_max=100.0;
  // SKUT_S7, см/ч, Скорость заполнения (опорожнения) РВС-5000 №7 (из уровнемера)
  strcpy(metric, "vel");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvs7 sys=saab"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[72].metric, metric);
  strcpy(HB_Param_Cross[72].tags  , tags  );
  HB_Param_Cross[72].adc_min=0; HB_Param_Cross[72].adc_max=20000;
  HB_Param_Cross[72].eng_min=-10000.0; HB_Param_Cross[72].eng_max=10000.0;
  // SKUT_V7, см/ч, Скорость заполнения (опорожнения) РВС-5000 №7
  strcpy(metric, "vel");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvs7 sys=calc"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[73].metric, metric);
  strcpy(HB_Param_Cross[73].tags  , tags  );
  HB_Param_Cross[73].adc_min=0; HB_Param_Cross[73].adc_max=20000;
  HB_Param_Cross[73].eng_min=-10000.0; HB_Param_Cross[73].eng_max=10000.0;
  // SKUT_L7, мм, Уровень РВС-5000 №7
  strcpy(metric, "level");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvs7"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[74].metric, metric);
  strcpy(HB_Param_Cross[74].tags  , tags  );
  HB_Param_Cross[74].adc_min=0; HB_Param_Cross[74].adc_max=22000;
  HB_Param_Cross[74].eng_min=0.0; HB_Param_Cross[74].eng_max=22000.0;
  // SKUT_T7, °С, Средняя температура РВС-5000 №7
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvs7"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[75].metric, metric);
  strcpy(HB_Param_Cross[75].tags  , tags  );
  HB_Param_Cross[75].adc_min=0; HB_Param_Cross[75].adc_max=2000;
  HB_Param_Cross[75].eng_min=-100.0; HB_Param_Cross[75].eng_max=100.0;
  // SKUT_S8, см/ч, Скорость заполнения (опорожнения) РВС-5000 №8 (из уровнемера)
  strcpy(metric, "vel");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvs8 sys=saab"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[76].metric, metric);
  strcpy(HB_Param_Cross[76].tags  , tags  );
  HB_Param_Cross[76].adc_min=0; HB_Param_Cross[76].adc_max=20000;
  HB_Param_Cross[76].eng_min=-10000.0; HB_Param_Cross[76].eng_max=10000.0;
  // SKUT_V8, см/ч, Скорость заполнения (опорожнения) РВС-5000 №8
  strcpy(metric, "vel");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvs8 sys=calc"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[77].metric, metric);
  strcpy(HB_Param_Cross[77].tags  , tags  );
  HB_Param_Cross[77].adc_min=0; HB_Param_Cross[77].adc_max=20000;
  HB_Param_Cross[77].eng_min=-10000.0; HB_Param_Cross[77].eng_max=10000.0;
  // SKUT_L8, мм, Уровень РВС-5000 №8
  strcpy(metric, "level");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvs8"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[78].metric, metric);
  strcpy(HB_Param_Cross[78].tags  , tags  );
  HB_Param_Cross[78].adc_min=0; HB_Param_Cross[78].adc_max=22000;
  HB_Param_Cross[78].eng_min=0.0; HB_Param_Cross[78].eng_max=22000.0;
  // SKUT_T8, °С, Средняя температура РВС-5000 №8
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=rvs8"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[79].metric, metric);
  strcpy(HB_Param_Cross[79].tags  , tags  );
  HB_Param_Cross[79].adc_min=0; HB_Param_Cross[79].adc_max=2000;
  HB_Param_Cross[79].eng_min=-100.0; HB_Param_Cross[79].eng_max=100.0;
  // XT533, мм/с, ПА-26. Вибрация верхнего подшипника ЭД (горизонтальное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa26 sys=vpdg"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[80].metric, metric);
  strcpy(HB_Param_Cross[80].tags  , tags  );
  HB_Param_Cross[80].adc_min=4000; HB_Param_Cross[80].adc_max=20000;
  HB_Param_Cross[80].eng_min=0.0; HB_Param_Cross[80].eng_max=30.0;
  // XT534, мм/с, ПА-26. Вибрация верхнего подшипника ЭД (поперечное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa26 sys=vpdp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[81].metric, metric);
  strcpy(HB_Param_Cross[81].tags  , tags  );
  HB_Param_Cross[81].adc_min=4000; HB_Param_Cross[81].adc_max=20000;
  HB_Param_Cross[81].eng_min=0.0; HB_Param_Cross[81].eng_max=30.0;
  // XT531, мм/с, ПА-26. Вибрация нижнего подшипника ЭД (горизонтальное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa26 sys=npdg"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[82].metric, metric);
  strcpy(HB_Param_Cross[82].tags  , tags  );
  HB_Param_Cross[82].adc_min=4000; HB_Param_Cross[82].adc_max=20000;
  HB_Param_Cross[82].eng_min=0.0; HB_Param_Cross[82].eng_max=30.0;
  // XT532, мм/с, ПА-26. Вибрация нижнего подшипника ЭД (поперечное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa26 sys=npdp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[83].metric, metric);
  strcpy(HB_Param_Cross[83].tags  , tags  );
  HB_Param_Cross[83].adc_min=4000; HB_Param_Cross[83].adc_max=20000;
  HB_Param_Cross[83].eng_min=0.0; HB_Param_Cross[83].eng_max=30.0;
  // XT535, мм/с, ПА-26. Вибрация опорно-упорного подшипника насоса (горизонтальное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa26 sys=opng"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[84].metric, metric);
  strcpy(HB_Param_Cross[84].tags  , tags  );
  HB_Param_Cross[84].adc_min=4000; HB_Param_Cross[84].adc_max=20000;
  HB_Param_Cross[84].eng_min=0.0; HB_Param_Cross[84].eng_max=30.0;
  // XT536, мм/с, ПА-26. Вибрация опорно-упорного подшипника насоса (поперечное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa26 sys=opnp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[85].metric, metric);
  strcpy(HB_Param_Cross[85].tags  , tags  );
  HB_Param_Cross[85].adc_min=4000; HB_Param_Cross[85].adc_max=20000;
  HB_Param_Cross[85].eng_min=0.0; HB_Param_Cross[85].eng_max=30.0;
  // PT531, МПа, ПА-26. Давление нефти на выходе ПНА
  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa26"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[86].metric, metric);
  strcpy(HB_Param_Cross[86].tags  , tags  );
  HB_Param_Cross[86].adc_min=4000; HB_Param_Cross[86].adc_max=20000;
  HB_Param_Cross[86].eng_min=0.0; HB_Param_Cross[86].eng_max=1.6;
  // FT531, A, ПА-26. Сила тока
  strcpy(metric, "curr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa26"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[87].metric, metric);
  strcpy(HB_Param_Cross[87].tags  , tags  );
  HB_Param_Cross[87].adc_min=4000; HB_Param_Cross[87].adc_max=20000;
  HB_Param_Cross[87].eng_min=0.0; HB_Param_Cross[87].eng_max=600.0;
  // TT534, °С, ПА-26. Температура верхнего подшипника ЭД
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa26 sys=vpd"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[88].metric, metric);
  strcpy(HB_Param_Cross[88].tags  , tags  );
  HB_Param_Cross[88].adc_min=4000; HB_Param_Cross[88].adc_max=20000;
  HB_Param_Cross[88].eng_min=-50.0; HB_Param_Cross[88].eng_max=150.0;
  // TT535, °С, ПА-26. Температура масла в корпусе подшипникового узла насоса
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa26 sys=oil"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[89].metric, metric);
  strcpy(HB_Param_Cross[89].tags  , tags  );
  HB_Param_Cross[89].adc_min=4000; HB_Param_Cross[89].adc_max=20000;
  HB_Param_Cross[89].eng_min=-50.0; HB_Param_Cross[89].eng_max=150.0;
  // TT531, °С, ПА-26. Температура нефти на выходе насоса
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa26 sys=nft"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[90].metric, metric);
  strcpy(HB_Param_Cross[90].tags  , tags  );
  HB_Param_Cross[90].adc_min=4000; HB_Param_Cross[90].adc_max=20000;
  HB_Param_Cross[90].eng_min=-50.0; HB_Param_Cross[90].eng_max=150.0;
  // TT533, °С, ПА-26. Температура нижнего подшипника ЭД
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa26 sys=npd"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[91].metric, metric);
  strcpy(HB_Param_Cross[91].tags  , tags  );
  HB_Param_Cross[91].adc_min=4000; HB_Param_Cross[91].adc_max=20000;
  HB_Param_Cross[91].eng_min=-50.0; HB_Param_Cross[91].eng_max=150.0;
  // TT532-1, °С, ПА-26. Температура опорно-упорного подшипника насоса (точка 1)
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa26 sys=opn1"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[92].metric, metric);
  strcpy(HB_Param_Cross[92].tags  , tags  );
  HB_Param_Cross[92].adc_min=4000; HB_Param_Cross[92].adc_max=20000;
  HB_Param_Cross[92].eng_min=-50.0; HB_Param_Cross[92].eng_max=150.0;
  // TT532-2, °С, ПА-26. Температура опорно-упорного подшипника насоса (точка 2)
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa26 sys=opn2"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[93].metric, metric);
  strcpy(HB_Param_Cross[93].tags  , tags  );
  HB_Param_Cross[93].adc_min=4000; HB_Param_Cross[93].adc_max=20000;
  HB_Param_Cross[93].eng_min=-50.0; HB_Param_Cross[93].eng_max=150.0;
  // XT543, мм/с, ПА-27. Вибрация верхнего подшипника ЭД (горизонтальное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa27 sys=vpdg"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[94].metric, metric);
  strcpy(HB_Param_Cross[94].tags  , tags  );
  HB_Param_Cross[94].adc_min=4000; HB_Param_Cross[94].adc_max=20000;
  HB_Param_Cross[94].eng_min=0.0; HB_Param_Cross[94].eng_max=30.0;
  // XT544, мм/с, ПА-27. Вибрация верхнего подшипника ЭД (поперечное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa27 sys=vpdp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[95].metric, metric);
  strcpy(HB_Param_Cross[95].tags  , tags  );
  HB_Param_Cross[95].adc_min=4000; HB_Param_Cross[95].adc_max=20000;
  HB_Param_Cross[95].eng_min=0.0; HB_Param_Cross[95].eng_max=30.0;
  // XT541, мм/с, ПА-27. Вибрация нижнего подшипника ЭД (горизонтальное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa27 sys=npdg"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[96].metric, metric);
  strcpy(HB_Param_Cross[96].tags  , tags  );
  HB_Param_Cross[96].adc_min=4000; HB_Param_Cross[96].adc_max=20000;
  HB_Param_Cross[96].eng_min=0.0; HB_Param_Cross[96].eng_max=30.0;
  // XT542, мм/с, ПА-27. Вибрация нижнего подшипника ЭД (поперечное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa27 sys=npdp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[97].metric, metric);
  strcpy(HB_Param_Cross[97].tags  , tags  );
  HB_Param_Cross[97].adc_min=4000; HB_Param_Cross[97].adc_max=20000;
  HB_Param_Cross[97].eng_min=0.0; HB_Param_Cross[97].eng_max=30.0;
  // XT545, мм/с, ПА-27. Вибрация опорно-упорного подшипника насоса (горизонтальное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa27 sys=opng"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[98].metric, metric);
  strcpy(HB_Param_Cross[98].tags  , tags  );
  HB_Param_Cross[98].adc_min=4000; HB_Param_Cross[98].adc_max=20000;
  HB_Param_Cross[98].eng_min=0.0; HB_Param_Cross[98].eng_max=30.0;
  // XT546, мм/с, ПА-27. Вибрация опорно-упорного подшипника насоса (поперечное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa27 sys=opnp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[99].metric, metric);
  strcpy(HB_Param_Cross[99].tags  , tags  );
  HB_Param_Cross[99].adc_min=4000; HB_Param_Cross[99].adc_max=20000;
  HB_Param_Cross[99].eng_min=0.0; HB_Param_Cross[99].eng_max=30.0;
  // PT541, МПа, ПА-27. Давление нефти на выходе ПНА
  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa27"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[100].metric, metric);
  strcpy(HB_Param_Cross[100].tags  , tags  );
  HB_Param_Cross[100].adc_min=4000; HB_Param_Cross[100].adc_max=20000;
  HB_Param_Cross[100].eng_min=0.0; HB_Param_Cross[100].eng_max=1.6;
  // FT541, A, ПА-27. Сила тока
  strcpy(metric, "curr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa27"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[101].metric, metric);
  strcpy(HB_Param_Cross[101].tags  , tags  );
  HB_Param_Cross[101].adc_min=4000; HB_Param_Cross[101].adc_max=20000;
  HB_Param_Cross[101].eng_min=0.0; HB_Param_Cross[101].eng_max=600.0;
  // TT544, °С, ПА-27. Температура верхнего подшипника ЭД
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa27 sys=vpd"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[102].metric, metric);
  strcpy(HB_Param_Cross[102].tags  , tags  );
  HB_Param_Cross[102].adc_min=4000; HB_Param_Cross[102].adc_max=20000;
  HB_Param_Cross[102].eng_min=-50.0; HB_Param_Cross[102].eng_max=150.0;
  // TT545, °С, ПА-27. Температура масла в корпусе подшипникового узла насоса
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa27 sys=oil"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[103].metric, metric);
  strcpy(HB_Param_Cross[103].tags  , tags  );
  HB_Param_Cross[103].adc_min=4000; HB_Param_Cross[103].adc_max=20000;
  HB_Param_Cross[103].eng_min=-50.0; HB_Param_Cross[103].eng_max=150.0;
  // TT541, °С, ПА-27. Температура нефти на выходе насоса
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa27 sys=nft"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[104].metric, metric);
  strcpy(HB_Param_Cross[104].tags  , tags  );
  HB_Param_Cross[104].adc_min=4000; HB_Param_Cross[104].adc_max=20000;
  HB_Param_Cross[104].eng_min=-50.0; HB_Param_Cross[104].eng_max=150.0;
  // TT543, °С, ПА-27. Температура нижнего подшипника ЭД
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa27 sys=npd"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[105].metric, metric);
  strcpy(HB_Param_Cross[105].tags  , tags  );
  HB_Param_Cross[105].adc_min=4000; HB_Param_Cross[105].adc_max=20000;
  HB_Param_Cross[105].eng_min=-50.0; HB_Param_Cross[105].eng_max=150.0;
  // TT542-1, °С, ПА-27. Температура опорно-упорного подшипника насоса (точка 1)
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa27 sys=opn1"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[106].metric, metric);
  strcpy(HB_Param_Cross[106].tags  , tags  );
  HB_Param_Cross[106].adc_min=4000; HB_Param_Cross[106].adc_max=20000;
  HB_Param_Cross[106].eng_min=-50.0; HB_Param_Cross[106].eng_max=150.0;
  // TT542-2, °С, ПА-27. Температура опорно-упорного подшипника насоса (точка 2)
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa27 sys=opn2"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[107].metric, metric);
  strcpy(HB_Param_Cross[107].tags  , tags  );
  HB_Param_Cross[107].adc_min=4000; HB_Param_Cross[107].adc_max=20000;
  HB_Param_Cross[107].eng_min=-50.0; HB_Param_Cross[107].eng_max=150.0;
  // XT563, мм/с, ПА-6. Вибрация верхнего подшипника ЭД (горизонтальное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa6 sys=vpdg"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[108].metric, metric);
  strcpy(HB_Param_Cross[108].tags  , tags  );
  HB_Param_Cross[108].adc_min=4000; HB_Param_Cross[108].adc_max=20000;
  HB_Param_Cross[108].eng_min=0.0; HB_Param_Cross[108].eng_max=30.0;
  // XT564, мм/с, ПА-6. Вибрация верхнего подшипника ЭД (поперечное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa6 sys=vpdp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[109].metric, metric);
  strcpy(HB_Param_Cross[109].tags  , tags  );
  HB_Param_Cross[109].adc_min=4000; HB_Param_Cross[109].adc_max=20000;
  HB_Param_Cross[109].eng_min=0.0; HB_Param_Cross[109].eng_max=30.0;
  // XT561, мм/с, ПА-6. Вибрация нижнего подшипника ЭД (горизонтальное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa6 sys=npdg"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[110].metric, metric);
  strcpy(HB_Param_Cross[110].tags  , tags  );
  HB_Param_Cross[110].adc_min=4000; HB_Param_Cross[110].adc_max=20000;
  HB_Param_Cross[110].eng_min=0.0; HB_Param_Cross[110].eng_max=30.0;
  // XT562, мм/с, ПА-6. Вибрация нижнего подшипника ЭД (поперечное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa6 sys=npdp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[111].metric, metric);
  strcpy(HB_Param_Cross[111].tags  , tags  );
  HB_Param_Cross[111].adc_min=4000; HB_Param_Cross[111].adc_max=20000;
  HB_Param_Cross[111].eng_min=0.0; HB_Param_Cross[111].eng_max=30.0;
  // XT565, мм/с, ПА-6. Вибрация опорно-упорного подшипника насоса (горизонтальное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa6 sys=opng"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[112].metric, metric);
  strcpy(HB_Param_Cross[112].tags  , tags  );
  HB_Param_Cross[112].adc_min=4000; HB_Param_Cross[112].adc_max=20000;
  HB_Param_Cross[112].eng_min=0.0; HB_Param_Cross[112].eng_max=30.0;
  // XT566, мм/с, ПА-6. Вибрация опорно-упорного подшипника насоса (поперечное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa6 sys=opnp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[113].metric, metric);
  strcpy(HB_Param_Cross[113].tags  , tags  );
  HB_Param_Cross[113].adc_min=4000; HB_Param_Cross[113].adc_max=20000;
  HB_Param_Cross[113].eng_min=0.0; HB_Param_Cross[113].eng_max=30.0;
  // PT561, МПа, ПА-6. Давление нефти на выходе ПНА
  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa6"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[114].metric, metric);
  strcpy(HB_Param_Cross[114].tags  , tags  );
  HB_Param_Cross[114].adc_min=4000; HB_Param_Cross[114].adc_max=20000;
  HB_Param_Cross[114].eng_min=0.0; HB_Param_Cross[114].eng_max=1.6;
  // FT561, A, ПА-6. Сила тока
  strcpy(metric, "curr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa6"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[115].metric, metric);
  strcpy(HB_Param_Cross[115].tags  , tags  );
  HB_Param_Cross[115].adc_min=4000; HB_Param_Cross[115].adc_max=20000;
  HB_Param_Cross[115].eng_min=0.0; HB_Param_Cross[115].eng_max=600.0;
  // TT564, °С, ПА-6. Температура верхнего подшипника ЭД
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa6 sys=vpd"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[116].metric, metric);
  strcpy(HB_Param_Cross[116].tags  , tags  );
  HB_Param_Cross[116].adc_min=4000; HB_Param_Cross[116].adc_max=20000;
  HB_Param_Cross[116].eng_min=-50.0; HB_Param_Cross[116].eng_max=150.0;
  // TT565, °С, ПА-6. Температура масла в корпусе подшипникового узла насоса
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa6 sys=oil"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[117].metric, metric);
  strcpy(HB_Param_Cross[117].tags  , tags  );
  HB_Param_Cross[117].adc_min=4000; HB_Param_Cross[117].adc_max=20000;
  HB_Param_Cross[117].eng_min=-50.0; HB_Param_Cross[117].eng_max=150.0;
  // TT561, °С, ПА-6. Температура нефти на выходе насоса
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa6 sys=nft"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[118].metric, metric);
  strcpy(HB_Param_Cross[118].tags  , tags  );
  HB_Param_Cross[118].adc_min=4000; HB_Param_Cross[118].adc_max=20000;
  HB_Param_Cross[118].eng_min=-50.0; HB_Param_Cross[118].eng_max=150.0;
  // TT563, °С, ПА-6. Температура нижнего подшипника ЭД
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa6 sys=npd"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[119].metric, metric);
  strcpy(HB_Param_Cross[119].tags  , tags  );
  HB_Param_Cross[119].adc_min=4000; HB_Param_Cross[119].adc_max=20000;
  HB_Param_Cross[119].eng_min=-50.0; HB_Param_Cross[119].eng_max=150.0;
  // TT562-1, °С, ПА-6. Температура опорно-упорного подшипника насоса (точка 1)
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa6 sys=opn1"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[120].metric, metric);
  strcpy(HB_Param_Cross[120].tags  , tags  );
  HB_Param_Cross[120].adc_min=4000; HB_Param_Cross[120].adc_max=20000;
  HB_Param_Cross[120].eng_min=-50.0; HB_Param_Cross[120].eng_max=150.0;
  // TT562-2, °С, ПА-6. Температура опорно-упорного подшипника насоса (точка 2)
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa6 sys=opn2"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[121].metric, metric);
  strcpy(HB_Param_Cross[121].tags  , tags  );
  HB_Param_Cross[121].adc_min=4000; HB_Param_Cross[121].adc_max=20000;
  HB_Param_Cross[121].eng_min=-50.0; HB_Param_Cross[121].eng_max=150.0;
  // XT573, мм/с, ПА-7. Вибрация верхнего подшипника ЭД (горизонтальное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa7 sys=vpdg"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[122].metric, metric);
  strcpy(HB_Param_Cross[122].tags  , tags  );
  HB_Param_Cross[122].adc_min=4000; HB_Param_Cross[122].adc_max=20000;
  HB_Param_Cross[122].eng_min=0.0; HB_Param_Cross[122].eng_max=30.0;
  // XT574, мм/с, ПА-7. Вибрация верхнего подшипника ЭД (поперечное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa7 sys=vpdp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[123].metric, metric);
  strcpy(HB_Param_Cross[123].tags  , tags  );
  HB_Param_Cross[123].adc_min=4000; HB_Param_Cross[123].adc_max=20000;
  HB_Param_Cross[123].eng_min=0.0; HB_Param_Cross[123].eng_max=30.0;
  // XT571, мм/с, ПА-7. Вибрация нижнего подшипника ЭД (горизонтальное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa7 sys=npdg"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[124].metric, metric);
  strcpy(HB_Param_Cross[124].tags  , tags  );
  HB_Param_Cross[124].adc_min=4000; HB_Param_Cross[124].adc_max=20000;
  HB_Param_Cross[124].eng_min=0.0; HB_Param_Cross[124].eng_max=30.0;
  // XT572, мм/с, ПА-7. Вибрация нижнего подшипника ЭД (поперечное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa7 sys=npdp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[125].metric, metric);
  strcpy(HB_Param_Cross[125].tags  , tags  );
  HB_Param_Cross[125].adc_min=4000; HB_Param_Cross[125].adc_max=20000;
  HB_Param_Cross[125].eng_min=0.0; HB_Param_Cross[125].eng_max=30.0;
  // XT575, мм/с, ПА-7. Вибрация опорно-упорного подшипника насоса (горизонтальное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa7 sys=opng"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[126].metric, metric);
  strcpy(HB_Param_Cross[126].tags  , tags  );
  HB_Param_Cross[126].adc_min=4000; HB_Param_Cross[126].adc_max=20000;
  HB_Param_Cross[126].eng_min=0.0; HB_Param_Cross[126].eng_max=30.0;
  // XT576, мм/с, ПА-7. Вибрация опорно-упорного подшипника насоса (поперечное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa7 sys=opnp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[127].metric, metric);
  strcpy(HB_Param_Cross[127].tags  , tags  );
  HB_Param_Cross[127].adc_min=4000; HB_Param_Cross[127].adc_max=20000;
  HB_Param_Cross[127].eng_min=0.0; HB_Param_Cross[127].eng_max=30.0;
  // PT571, МПа, ПА-7. Давление нефти на выходе ПНА
  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa7"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[128].metric, metric);
  strcpy(HB_Param_Cross[128].tags  , tags  );
  HB_Param_Cross[128].adc_min=4000; HB_Param_Cross[128].adc_max=20000;
  HB_Param_Cross[128].eng_min=0.0; HB_Param_Cross[128].eng_max=1.6;
  // FT571, A, ПА-7. Сила тока
  strcpy(metric, "curr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa7"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[129].metric, metric);
  strcpy(HB_Param_Cross[129].tags  , tags  );
  HB_Param_Cross[129].adc_min=4000; HB_Param_Cross[129].adc_max=20000;
  HB_Param_Cross[129].eng_min=0.0; HB_Param_Cross[129].eng_max=600.0;
  // TT574, °С, ПА-7. Температура верхнего подшипника ЭД
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa7 sys=vpd"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[130].metric, metric);
  strcpy(HB_Param_Cross[130].tags  , tags  );
  HB_Param_Cross[130].adc_min=4000; HB_Param_Cross[130].adc_max=20000;
  HB_Param_Cross[130].eng_min=-50.0; HB_Param_Cross[130].eng_max=150.0;
  // TT575, °С, ПА-7. Температура масла в корпусе подшипникового узла насоса
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa7 sys=oil"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[131].metric, metric);
  strcpy(HB_Param_Cross[131].tags  , tags  );
  HB_Param_Cross[131].adc_min=4000; HB_Param_Cross[131].adc_max=20000;
  HB_Param_Cross[131].eng_min=-50.0; HB_Param_Cross[131].eng_max=150.0;
  // TT571, °С, ПА-7. Температура нефти на выходе насоса
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa7 sys=nft"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[132].metric, metric);
  strcpy(HB_Param_Cross[132].tags  , tags  );
  HB_Param_Cross[132].adc_min=4000; HB_Param_Cross[132].adc_max=20000;
  HB_Param_Cross[132].eng_min=-50.0; HB_Param_Cross[132].eng_max=150.0;
  // TT573, °С, ПА-7. Температура нижнего подшипника ЭД
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa7 sys=npd"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[133].metric, metric);
  strcpy(HB_Param_Cross[133].tags  , tags  );
  HB_Param_Cross[133].adc_min=4000; HB_Param_Cross[133].adc_max=20000;
  HB_Param_Cross[133].eng_min=-50.0; HB_Param_Cross[133].eng_max=150.0;
  // TT572-1, °С, ПА-7. Температура опорно-упорного подшипника насоса (точка 1)
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa7 sys=opn1"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[134].metric, metric);
  strcpy(HB_Param_Cross[134].tags  , tags  );
  HB_Param_Cross[134].adc_min=4000; HB_Param_Cross[134].adc_max=20000;
  HB_Param_Cross[134].eng_min=-50.0; HB_Param_Cross[134].eng_max=150.0;
  // TT572-2, °С, ПА-7. Температура опорно-упорного подшипника насоса (точка 2)
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa7 sys=opn2"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[135].metric, metric);
  strcpy(HB_Param_Cross[135].tags  , tags  );
  HB_Param_Cross[135].adc_min=4000; HB_Param_Cross[135].adc_max=20000;
  HB_Param_Cross[135].eng_min=-50.0; HB_Param_Cross[135].eng_max=150.0;
  // XT583, мм/с, ПА-8. Вибрация верхнего подшипника ЭД (горизонтальное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa8 sys=vpdg"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[136].metric, metric);
  strcpy(HB_Param_Cross[136].tags  , tags  );
  HB_Param_Cross[136].adc_min=4000; HB_Param_Cross[136].adc_max=20000;
  HB_Param_Cross[136].eng_min=0.0; HB_Param_Cross[136].eng_max=30.0;
  // XT584, мм/с, ПА-8. Вибрация верхнего подшипника ЭД (поперечное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa8 sys=vpdp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[137].metric, metric);
  strcpy(HB_Param_Cross[137].tags  , tags  );
  HB_Param_Cross[137].adc_min=4000; HB_Param_Cross[137].adc_max=20000;
  HB_Param_Cross[137].eng_min=0.0; HB_Param_Cross[137].eng_max=30.0;
  // XT581, мм/с, ПА-8. Вибрация нижнего подшипника ЭД (горизонтальное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa8 sys=npdg"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[138].metric, metric);
  strcpy(HB_Param_Cross[138].tags  , tags  );
  HB_Param_Cross[138].adc_min=4000; HB_Param_Cross[138].adc_max=20000;
  HB_Param_Cross[138].eng_min=0.0; HB_Param_Cross[138].eng_max=30.0;
  // XT582, мм/с, ПА-8. Вибрация нижнего подшипника ЭД (поперечное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa8 sys=npdp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[139].metric, metric);
  strcpy(HB_Param_Cross[139].tags  , tags  );
  HB_Param_Cross[139].adc_min=4000; HB_Param_Cross[139].adc_max=20000;
  HB_Param_Cross[139].eng_min=0.0; HB_Param_Cross[139].eng_max=30.0;
  // XT585, мм/с, ПА-8. Вибрация опорно-упорного подшипника насоса (горизонтальное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa8 sys=opng"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[140].metric, metric);
  strcpy(HB_Param_Cross[140].tags  , tags  );
  HB_Param_Cross[140].adc_min=4000; HB_Param_Cross[140].adc_max=20000;
  HB_Param_Cross[140].eng_min=0.0; HB_Param_Cross[140].eng_max=30.0;
  // XT586, мм/с, ПА-8. Вибрация опорно-упорного подшипника насоса (поперечное направление)
  strcpy(metric, "vibr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa8 sys=opnp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[141].metric, metric);
  strcpy(HB_Param_Cross[141].tags  , tags  );
  HB_Param_Cross[141].adc_min=4000; HB_Param_Cross[141].adc_max=20000;
  HB_Param_Cross[141].eng_min=0.0; HB_Param_Cross[141].eng_max=30.0;
  // PT581, МПа, ПА-8. Давление нефти на выходе ПНА
  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa8"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[142].metric, metric);
  strcpy(HB_Param_Cross[142].tags  , tags  );
  HB_Param_Cross[142].adc_min=4000; HB_Param_Cross[142].adc_max=20000;
  HB_Param_Cross[142].eng_min=0.0; HB_Param_Cross[142].eng_max=1.6;
  // FT581, A, ПА-8. Сила тока
  strcpy(metric, "curr");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa8"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[143].metric, metric);
  strcpy(HB_Param_Cross[143].tags  , tags  );
  HB_Param_Cross[143].adc_min=4000; HB_Param_Cross[143].adc_max=20000;
  HB_Param_Cross[143].eng_min=0.0; HB_Param_Cross[143].eng_max=600.0;
  // TT584, °С, ПА-8. Температура верхнего подшипника ЭД
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa8 sys=vpd"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[144].metric, metric);
  strcpy(HB_Param_Cross[144].tags  , tags  );
  HB_Param_Cross[144].adc_min=4000; HB_Param_Cross[144].adc_max=20000;
  HB_Param_Cross[144].eng_min=-50.0; HB_Param_Cross[144].eng_max=150.0;
  // TT585, °С, ПА-8. Температура масла в корпусе подшипникового узла насоса
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa8 sys=oil"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[145].metric, metric);
  strcpy(HB_Param_Cross[145].tags  , tags  );
  HB_Param_Cross[145].adc_min=4000; HB_Param_Cross[145].adc_max=20000;
  HB_Param_Cross[145].eng_min=-50.0; HB_Param_Cross[145].eng_max=150.0;
  // TT581, °С, ПА-8. Температура нефти на выходе насоса
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa8 sys=nft"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[146].metric, metric);
  strcpy(HB_Param_Cross[146].tags  , tags  );
  HB_Param_Cross[146].adc_min=4000; HB_Param_Cross[146].adc_max=20000;
  HB_Param_Cross[146].eng_min=-50.0; HB_Param_Cross[146].eng_max=150.0;
  // TT583, °С, ПА-8. Температура нижнего подшипника ЭД
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa8 sys=npd"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[147].metric, metric);
  strcpy(HB_Param_Cross[147].tags  , tags  );
  HB_Param_Cross[147].adc_min=4000; HB_Param_Cross[147].adc_max=20000;
  HB_Param_Cross[147].eng_min=-50.0; HB_Param_Cross[147].eng_max=150.0;
  // TT582-1, °С, ПА-8. Температура опорно-упорного подшипника насоса (точка 1)
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa8 sys=opn1"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[148].metric, metric);
  strcpy(HB_Param_Cross[148].tags  , tags  );
  HB_Param_Cross[148].adc_min=4000; HB_Param_Cross[148].adc_max=20000;
  HB_Param_Cross[148].eng_min=-50.0; HB_Param_Cross[148].eng_max=150.0;
  // TT582-2, °С, ПА-8. Температура опорно-упорного подшипника насоса (точка 2)
  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
  strcpy(tags  , " node=pa8 sys=opn2"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
  strcpy(HB_Param_Cross[149].metric, metric);
  strcpy(HB_Param_Cross[149].tags  , tags  );
  HB_Param_Cross[149].adc_min=4000; HB_Param_Cross[149].adc_max=20000;
  HB_Param_Cross[149].eng_min=-50.0; HB_Param_Cross[149].eng_max=150.0;

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

    case GATEWAY_T02:

	    switch((group>>0)&0xf) {
        case 0: cpprm(i,999, 0); cpprm(i,999, 1); cpprm(i,999, 2); cpprm(i,999, 3); break;
        case 1: cpprm(i, 16, 0); cpprm(i, 17, 1); cpprm(i, 18, 2); cpprm(i, 19, 3); break;
        case 2: cpprm(i, 20, 0); cpprm(i, 21, 1); cpprm(i, 22, 2); cpprm(i, 23, 3); break;
        case 3: cpprm(i, 24, 0); cpprm(i, 25, 1); cpprm(i, 26, 2); cpprm(i, 27, 3); break;
        case 4: cpprm(i, 28, 0); cpprm(i, 29, 1); cpprm(i, 30, 2); cpprm(i, 31, 3); break;
        case 5: cpprm(i, 64, 0); cpprm(i, 65, 1); cpprm(i, 66, 2); cpprm(i, 67, 3); break;
        case 6: cpprm(i, 68, 0); cpprm(i, 69, 1); cpprm(i, 70, 2); cpprm(i, 71, 3); break;
        case 7: cpprm(i, 72, 0); cpprm(i, 73, 1); cpprm(i, 74, 2); cpprm(i, 75, 3); break;
        case 8: cpprm(i, 76, 0); cpprm(i, 77, 1); cpprm(i, 78, 2); cpprm(i, 79, 3); break;
        }
	    switch((group>>4)&0xf) {
        case 0: cpprm(i,999, 4); cpprm(i,999, 5); cpprm(i,999, 6); cpprm(i,999, 7); break;
        case 1: cpprm(i, 16, 4); cpprm(i, 17, 5); cpprm(i, 18, 6); cpprm(i, 19, 7); break;
        case 2: cpprm(i, 20, 4); cpprm(i, 21, 5); cpprm(i, 22, 6); cpprm(i, 23, 7); break;
        case 3: cpprm(i, 24, 4); cpprm(i, 25, 5); cpprm(i, 26, 6); cpprm(i, 27, 7); break;
        case 4: cpprm(i, 28, 4); cpprm(i, 29, 5); cpprm(i, 30, 6); cpprm(i, 31, 7); break;
        case 5: cpprm(i, 64, 4); cpprm(i, 65, 5); cpprm(i, 66, 6); cpprm(i, 67, 7); break;
        case 6: cpprm(i, 68, 4); cpprm(i, 69, 5); cpprm(i, 70, 6); cpprm(i, 71, 7); break;
        case 7: cpprm(i, 72, 4); cpprm(i, 73, 5); cpprm(i, 74, 6); cpprm(i, 75, 7); break;
        case 8: cpprm(i, 76, 4); cpprm(i, 77, 5); cpprm(i, 78, 6); cpprm(i, 79, 7); break;
        }
	    switch((group>>8)&0xf) {
        case 0: cpprm(i,999, 8); cpprm(i,999, 9); cpprm(i,999,10); cpprm(i,999,11); break;
        case 1: cpprm(i, 16, 8); cpprm(i, 17, 9); cpprm(i, 18,10); cpprm(i, 19,11); break;
        case 2: cpprm(i, 20, 8); cpprm(i, 21, 9); cpprm(i, 22,10); cpprm(i, 23,11); break;
        case 3: cpprm(i, 24, 8); cpprm(i, 25, 9); cpprm(i, 26,10); cpprm(i, 27,11); break;
        case 4: cpprm(i, 28, 8); cpprm(i, 29, 9); cpprm(i, 30,10); cpprm(i, 31,11); break;
        case 5: cpprm(i, 64, 8); cpprm(i, 65, 9); cpprm(i, 66,10); cpprm(i, 67,11); break;
        case 6: cpprm(i, 68, 8); cpprm(i, 69, 9); cpprm(i, 70,10); cpprm(i, 71,11); break;
        case 7: cpprm(i, 72, 8); cpprm(i, 73, 9); cpprm(i, 74,10); cpprm(i, 75,11); break;
        case 8: cpprm(i, 76, 8); cpprm(i, 77, 9); cpprm(i, 78,10); cpprm(i, 79,11); break;
        }
	    switch((group>>12)&0xf) {
        case 0: cpprm(i,999,12); cpprm(i,999,13); cpprm(i,999,14); cpprm(i,999,15); break;
        case 1: cpprm(i, 16,12); cpprm(i, 17,13); cpprm(i, 18,14); cpprm(i, 19,15); break;
        case 2: cpprm(i, 20,12); cpprm(i, 21,13); cpprm(i, 22,14); cpprm(i, 23,15); break;
        case 3: cpprm(i, 24,12); cpprm(i, 25,13); cpprm(i, 26,14); cpprm(i, 27,15); break;
        case 4: cpprm(i, 28,12); cpprm(i, 29,13); cpprm(i, 30,14); cpprm(i, 31,15); break;
        case 5: cpprm(i, 64,12); cpprm(i, 65,13); cpprm(i, 66,14); cpprm(i, 67,15); break;
        case 6: cpprm(i, 68,12); cpprm(i, 69,13); cpprm(i, 70,14); cpprm(i, 71,15); break;
        case 7: cpprm(i, 72,12); cpprm(i, 73,13); cpprm(i, 74,14); cpprm(i, 75,15); break;
        case 8: cpprm(i, 76,12); cpprm(i, 77,13); cpprm(i, 78,14); cpprm(i, 79,15); break;
        }
    break; // GATEWAY_T02

    case GATEWAY_T03:

	    switch(group&0xff) {
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
        }
    break; // GATEWAY_T03

    case GATEWAY_T04:

	    switch(group&0xff) {
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
        }
    break; // GATEWAY_T04

		default:;
    } 

  return;
	}
///-----------------------------------------------------------------------------------------------------------------

