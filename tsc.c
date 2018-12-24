/***********   XXXXXXXXX   *************
        MODBUS XXXXXXX SOFTWARE         
                    VERSION X.X         
        AO NPO SPETSELECTROMEHANIKA     
               BRYANSK 2015                 
***************************************/

///******************* ������ TSC ********************************

///=== TSC_H IMPLEMENTATION

#include <string.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <pthread.h>

#include "tsc.h"
#include "interfaces.h"

///=== TSC_H private variables

///=== TSC_H private functions

//******************************************************************************
int init_tsc_h(int port)
  {
  int i=port-GATEWAY_T01;
  char metric[128];
  char tags  [128];

  if(port<GATEWAY_T01 || port>GATEWAY_T04) return 1;

  switch(port) {

      // ����, ��. ���.
//   	strcpy(metric, "metric1");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
//   	strcpy(tags  , " tagk=tagv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
//   	strcpy(HB_Param[i][0].metric, metric);
//   	strcpy(HB_Param[i][0].tags  , tags  );
//   	HB_Param[i][0].adc_min=0; HB_Param[i][0].adc_max=35;
//   	HB_Param[i][0].eng_min=0; HB_Param[i][0].eng_max=66535.5;

//15	LT653, ��	������� � ������� ��-40 (�����)	3605.0	0.0	65535	0	level	node=ep40

//      // \2, \3
//   	  strcpy(metric, "\8");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
//   	  strcpy(tags  , " \9"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
//   	  strcpy(HB_Param[i][\1].metric, metric);
//   	  strcpy(HB_Param[i][\1].tags  , tags  );
//   	  HB_Param[i][\1].adc_min=\7; HB_Param[i][\1].adc_max=\6;
//   	  HB_Param[i][\1].eng_min=\5; HB_Param[i][\1].eng_max=\4;

    case GATEWAY_T01:
      // TT851, ����. �, ����������� ������� � ���
   	  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ktp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][0].metric, metric);
   	  strcpy(HB_Param[i][0].tags  , tags  );
   	  HB_Param[i][0].adc_min=0; HB_Param[i][0].adc_max=65535;
   	  HB_Param[i][0].eng_min=-25.0; HB_Param[i][0].eng_max=50.0;
      // TT852, ����. �, ����������� ������� � ������� ���
   	  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=schsu"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][1].metric, metric);
   	  strcpy(HB_Param[i][1].tags  , tags  );
   	  HB_Param[i][1].adc_min=0; HB_Param[i][1].adc_max=65535;
   	  HB_Param[i][1].eng_min=-25.0; HB_Param[i][1].eng_max=50.0;
      // TT900, ����. �, ����������� ������� � ����� ���11.1
   	  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=uso11_1"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][2].metric, metric);
   	  strcpy(HB_Param[i][2].tags  , tags  );
   	  HB_Param[i][2].adc_min=0; HB_Param[i][2].adc_max=65535;
   	  HB_Param[i][2].eng_min=-50.0; HB_Param[i][2].eng_max=200.0;
      // TT901, ����. �, ����������� ������� � ����� ���11.2
   	  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=uso11_2"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][3].metric, metric);
   	  strcpy(HB_Param[i][3].tags  , tags  );
   	  HB_Param[i][3].adc_min=0; HB_Param[i][3].adc_max=65535;
   	  HB_Param[i][3].eng_min=-50.0; HB_Param[i][3].eng_max=180.0;
      // TT902, ����. �, ����������� ������� � ����� ���11.3
   	  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=uso11_3"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][4].metric, metric);
   	  strcpy(HB_Param[i][4].tags  , tags  );
   	  HB_Param[i][4].adc_min=0; HB_Param[i][4].adc_max=65535;
   	  HB_Param[i][4].eng_min=-50.0; HB_Param[i][4].eng_max=180.0;
      // TT903, ����. �, ����������� ������� � ����� ���11.4
   	  strcpy(metric, "temp");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=uso11_4"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][5].metric, metric);
   	  strcpy(HB_Param[i][5].tags  , tags  );
   	  HB_Param[i][5].adc_min=0; HB_Param[i][5].adc_max=65535;
   	  HB_Param[i][5].eng_min=-50.0; HB_Param[i][5].eng_max=200.0;
      // FT679, ���.�./���, ������ ����� �.2 �� 10,11 (��-9,10)
   	  strcpy(metric, "flow");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rdab dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][6].metric, metric);
   	  strcpy(HB_Param[i][6].tags  , tags  );
   	  HB_Param[i][6].adc_min=0; HB_Param[i][6].adc_max=65535;
   	  HB_Param[i][6].eng_min=0.0; HB_Param[i][6].eng_max=2200.0;
      // SPRDAB, ���.�./���, ������� ������� ��� �� 10,11 (��-9,10)
   	  strcpy(metric, "flow");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rdab dp=sp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][7].metric, metric);
   	  strcpy(HB_Param[i][7].tags  , tags  );
   	  HB_Param[i][7].adc_min=0; HB_Param[i][7].adc_max=65535;
   	  HB_Param[i][7].eng_min=0.0; HB_Param[i][7].eng_max=65535.0;
      // QT_P9, %, ��������� �� 10 (��-9)
   	  strcpy(metric, "pos");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd10 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][8].metric, metric);
   	  strcpy(HB_Param[i][8].tags  , tags  );
   	  HB_Param[i][8].adc_min=0; HB_Param[i][8].adc_max=65535;
   	  HB_Param[i][8].eng_min=-12.5; HB_Param[i][8].eng_max=112.5;
      // SPRDA, %, ������� ��������� �� 10 (��-9)
   	  strcpy(metric, "pos");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd10 dp=sp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][9].metric, metric);
   	  strcpy(HB_Param[i][9].tags  , tags  );
   	  HB_Param[i][9].adc_min=0; HB_Param[i][9].adc_max=65535;
   	  HB_Param[i][9].eng_min=0.0; HB_Param[i][9].eng_max=100.0;
      // QT_P10, %, ��������� �� 11 (��-10)
   	  strcpy(metric, "pos");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd11 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][10].metric, metric);
   	  strcpy(HB_Param[i][10].tags  , tags  );
   	  HB_Param[i][10].adc_min=0; HB_Param[i][10].adc_max=65535;
   	  HB_Param[i][10].eng_min=-12.5; HB_Param[i][10].eng_max=112.5;
      // SPRDB, %, ������� ��������� �� 11 (��-10)
   	  strcpy(metric, "pos");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd11 dp=sp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][11].metric, metric);
   	  strcpy(HB_Param[i][11].tags  , tags  );
   	  HB_Param[i][11].adc_min=0; HB_Param[i][11].adc_max=65535;
   	  HB_Param[i][11].eng_min=0.0; HB_Param[i][11].eng_max=100.0;
      // PT675, ���/��2, �������� �� ����� �� 10,11 (��-9,10)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rdab"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][12].metric, metric);
   	  strcpy(HB_Param[i][12].tags  , tags  );
   	  HB_Param[i][12].adc_min=0; HB_Param[i][12].adc_max=65535;
   	  HB_Param[i][12].eng_min=-2.0; HB_Param[i][12].eng_max=57.2;
    	break;

    case GATEWAY_T02:
      // PT501, ���/��2, �������� �� ������ �����_1 �.0 DN_1220 (�����)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=sod0 dp=out"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][0].metric, metric);
   	  strcpy(HB_Param[i][0].tags  , tags  );
   	  HB_Param[i][0].adc_min=0; HB_Param[i][0].adc_max=65535;
   	  HB_Param[i][0].eng_min=-1.6; HB_Param[i][0].eng_max=10.0;
      // PT502, ���/��2, �������� �� ������ �����_2 �.9 DN_1020 (�����)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=sod9 dp=out"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][1].metric, metric);
   	  strcpy(HB_Param[i][1].tags  , tags  );
   	  HB_Param[i][1].adc_min=0; HB_Param[i][1].adc_max=65535;
   	  HB_Param[i][1].eng_min=-1.6; HB_Param[i][1].eng_max=10.0;
      // PT503, ���/��2, �������� �� ������ �����_3 �.4 DN_820 (�����)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=sod4 dp=out"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][2].metric, metric);
   	  strcpy(HB_Param[i][2].tags  , tags  );
   	  HB_Param[i][2].adc_min=0; HB_Param[i][2].adc_max=65535;
   	  HB_Param[i][2].eng_min=-1.6; HB_Param[i][2].eng_max=10.0;
      // PT504, ���/��2, �������� �� ������ �����_4 �.1 DN_630 (�����)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=sod1 dp=out"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][3].metric, metric);
   	  strcpy(HB_Param[i][3].tags  , tags  );
   	  HB_Param[i][3].adc_min=0; HB_Param[i][3].adc_max=65535;
   	  HB_Param[i][3].eng_min=-1.6; HB_Param[i][3].eng_max=10.0;
      // PT505, ���/��2, �������� �� ������ �����_5 �.2 DN_630 (��)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=sod2 dp=out"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][4].metric, metric);
   	  strcpy(HB_Param[i][4].tags  , tags  );
   	  HB_Param[i][4].adc_min=0; HB_Param[i][4].adc_max=65535;
   	  HB_Param[i][4].eng_min=-1.6; HB_Param[i][4].eng_max=10.0;
      // PT506, ���/��2, �������� �� ������ �����_6 �.7 DN_426 (�����/��)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=sod7 dp=out"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][5].metric, metric);
   	  strcpy(HB_Param[i][5].tags  , tags  );
   	  HB_Param[i][5].adc_min=0; HB_Param[i][5].adc_max=65535;
   	  HB_Param[i][5].eng_min=-1.6; HB_Param[i][5].eng_max=10.0;
      // PT507, ���/��2, �������� � �����_1 �.0 DN_1220 (�����)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=sod0 dp=in"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][6].metric, metric);
   	  strcpy(HB_Param[i][6].tags  , tags  );
   	  HB_Param[i][6].adc_min=0; HB_Param[i][6].adc_max=65535;
   	  HB_Param[i][6].eng_min=-1.0; HB_Param[i][6].eng_max=16.0;
      // PT508, ���/��2, �������� � �����_2 �.9 DN_1020 (�����)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=sod9 dp=in"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][7].metric, metric);
   	  strcpy(HB_Param[i][7].tags  , tags  );
   	  HB_Param[i][7].adc_min=0; HB_Param[i][7].adc_max=65535;
   	  HB_Param[i][7].eng_min=-1.0; HB_Param[i][7].eng_max=16.0;
      // PT509, ���/��2, �������� � �����_3 �.4 DN_820 (�����)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=sod4 dp=in"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][8].metric, metric);
   	  strcpy(HB_Param[i][8].tags  , tags  );
   	  HB_Param[i][8].adc_min=0; HB_Param[i][8].adc_max=65535;
   	  HB_Param[i][8].eng_min=-1.0; HB_Param[i][8].eng_max=16.0;
      // PT510, ���/��2, �������� � �����_4 �.1 DN_630 (�����)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=sod1 dp=in"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][9].metric, metric);
   	  strcpy(HB_Param[i][9].tags  , tags  );
   	  HB_Param[i][9].adc_min=0; HB_Param[i][9].adc_max=65535;
   	  HB_Param[i][9].eng_min=-1.0; HB_Param[i][9].eng_max=16.0;
      // PT511, ���/��2, �������� � �����_5 �.2 DN_630 (��)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=sod2 dp=in"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][10].metric, metric);
   	  strcpy(HB_Param[i][10].tags  , tags  );
   	  HB_Param[i][10].adc_min=0; HB_Param[i][10].adc_max=65535;
   	  HB_Param[i][10].eng_min=-1.0; HB_Param[i][10].eng_max=16.0;
      // PT512, ���/��2, �������� � �����_6 �.7 DN_426 (�����/��)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=sod7 dp=in"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][11].metric, metric);
   	  strcpy(HB_Param[i][11].tags  , tags  );
   	  HB_Param[i][11].adc_min=0; HB_Param[i][11].adc_max=65535;
   	  HB_Param[i][11].eng_min=-1.0; HB_Param[i][11].eng_max=16.0;
      // QT901, % ����, �������������� �� �������� ����� � �������
   	  strcpy(metric, "gaz");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=sp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][12].metric, metric);
   	  strcpy(HB_Param[i][12].tags  , tags  );
   	  HB_Param[i][12].adc_min=0; HB_Param[i][12].adc_max=65535;
   	  HB_Param[i][12].eng_min=-5.0; HB_Param[i][12].eng_max=105.0;
      // LT651, ��, ������� � ������� ��-25 (�����)
   	  strcpy(metric, "level");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ep25"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][13].metric, metric);
   	  strcpy(HB_Param[i][13].tags  , tags  );
   	  HB_Param[i][13].adc_min=0; HB_Param[i][13].adc_max=65535;
   	  HB_Param[i][13].eng_min=0.0; HB_Param[i][13].eng_max=3355.0;
      // LT652, ��, ������� � ������� ��-16 (��)
   	  strcpy(metric, "level");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ep16"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][14].metric, metric);
   	  strcpy(HB_Param[i][14].tags  , tags  );
   	  HB_Param[i][14].adc_min=0; HB_Param[i][14].adc_max=65535;
   	  HB_Param[i][14].eng_min=0.0; HB_Param[i][14].eng_max=3355.0;
      // LT653, ��, ������� � ������� ��-40 (�����)
   	  strcpy(metric, "level");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=ep40"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][15].metric, metric);
   	  strcpy(HB_Param[i][15].tags  , tags  );
   	  HB_Param[i][15].adc_min=0; HB_Param[i][15].adc_max=65535;
   	  HB_Param[i][15].eng_min=0.0; HB_Param[i][15].eng_max=3605.0;
    	break;

    case GATEWAY_T03:
      // FT671, ���.�./���, ������ ����� �.0 �� 7,6 (��-1,2)
   	  strcpy(metric, "flow");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd67 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][0].metric, metric);
   	  strcpy(HB_Param[i][0].tags  , tags  );
   	  HB_Param[i][0].adc_min=0; HB_Param[i][0].adc_max=65535;
   	  HB_Param[i][0].eng_min=0.0; HB_Param[i][0].eng_max=13500.0;
      // SPRD67, ���.�./���, ������� ������� ��� �� 7,6 (��-1,2)
   	  strcpy(metric, "flow");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd67 dp=sp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][1].metric, metric);
   	  strcpy(HB_Param[i][1].tags  , tags  );
   	  HB_Param[i][1].adc_min=0; HB_Param[i][1].adc_max=65535;
   	  HB_Param[i][1].eng_min=0.0; HB_Param[i][1].eng_max=65535.0;
      // QT_P1, %, ��������� �� 7 (��-1)
   	  strcpy(metric, "pos");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd7 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][2].metric, metric);
   	  strcpy(HB_Param[i][2].tags  , tags  );
   	  HB_Param[i][2].adc_min=0; HB_Param[i][2].adc_max=65535;
   	  HB_Param[i][2].eng_min=-12.5; HB_Param[i][2].eng_max=112.5;
      // SPRD7, %, ������� ��������� �� 7 (��-1)
   	  strcpy(metric, "pos");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd7 dp=sp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][3].metric, metric);
   	  strcpy(HB_Param[i][3].tags  , tags  );
   	  HB_Param[i][3].adc_min=0; HB_Param[i][3].adc_max=65535;
   	  HB_Param[i][3].eng_min=0.0; HB_Param[i][3].eng_max=100.0;
      // QT_P2, %, ��������� �� 6 (��-2)
   	  strcpy(metric, "pos");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd6 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][4].metric, metric);
   	  strcpy(HB_Param[i][4].tags  , tags  );
   	  HB_Param[i][4].adc_min=0; HB_Param[i][4].adc_max=65535;
   	  HB_Param[i][4].eng_min=-12.5; HB_Param[i][4].eng_max=112.5;
      // SPRD6, %, ������� ��������� �� 6 (��-2)
   	  strcpy(metric, "pos");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd6 dp=sp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][5].metric, metric);
   	  strcpy(HB_Param[i][5].tags  , tags  );
   	  HB_Param[i][5].adc_min=0; HB_Param[i][5].adc_max=65535;
   	  HB_Param[i][5].eng_min=0.0; HB_Param[i][5].eng_max=100.0;
      // PT671, ���/��2, �������� �� ����� �� 7,6 (��-1,2)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd67"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][6].metric, metric);
   	  strcpy(HB_Param[i][6].tags  , tags  );
   	  HB_Param[i][6].adc_min=0; HB_Param[i][6].adc_max=65535;
   	  HB_Param[i][6].eng_min=-2.0; HB_Param[i][6].eng_max=57.2;
      // FT673, ���.�./���, ������ ����� �.8 �� 5,4 (��-3,4)
   	  strcpy(metric, "flow");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd45 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][7].metric, metric);
   	  strcpy(HB_Param[i][7].tags  , tags  );
   	  HB_Param[i][7].adc_min=0; HB_Param[i][7].adc_max=65535;
   	  HB_Param[i][7].eng_min=0.0; HB_Param[i][7].eng_max=8300.0;
      // SPRD45, ���.�./���, ������� ������� ��� �� 5,4 (��-3,4)
   	  strcpy(metric, "flow");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd45 dp=sp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][8].metric, metric);
   	  strcpy(HB_Param[i][8].tags  , tags  );
   	  HB_Param[i][8].adc_min=0; HB_Param[i][8].adc_max=65535;
   	  HB_Param[i][8].eng_min=0.0; HB_Param[i][8].eng_max=65535.0;
      // QT_P3, %, ��������� �� 5 (��-3)
   	  strcpy(metric, "pos");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd5 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][9].metric, metric);
   	  strcpy(HB_Param[i][9].tags  , tags  );
   	  HB_Param[i][9].adc_min=0; HB_Param[i][9].adc_max=65535;
   	  HB_Param[i][9].eng_min=-12.5; HB_Param[i][9].eng_max=112.5;
      // SPRD5, %, ������� ��������� �� 5 (��-3)
   	  strcpy(metric, "pos");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd5 dp=sp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][10].metric, metric);
   	  strcpy(HB_Param[i][10].tags  , tags  );
   	  HB_Param[i][10].adc_min=0; HB_Param[i][10].adc_max=65535;
   	  HB_Param[i][10].eng_min=0.0; HB_Param[i][10].eng_max=100.0;
      // QT_P4, %, ��������� �� 4 (��-4)
   	  strcpy(metric, "pos");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd4 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][11].metric, metric);
   	  strcpy(HB_Param[i][11].tags  , tags  );
   	  HB_Param[i][11].adc_min=0; HB_Param[i][11].adc_max=65535;
   	  HB_Param[i][11].eng_min=-12.5; HB_Param[i][11].eng_max=112.5;
      // SPRD4, %, ������� ��������� �� 4 (��-4)
   	  strcpy(metric, "pos");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd4 dp=sp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][12].metric, metric);
   	  strcpy(HB_Param[i][12].tags  , tags  );
   	  HB_Param[i][12].adc_min=0; HB_Param[i][12].adc_max=65535;
   	  HB_Param[i][12].eng_min=0.0; HB_Param[i][12].eng_max=100.0;
      // PT672, ���/��2, �������� �� ����� �� 5,4 (��-3,4)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd45"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][13].metric, metric);
   	  strcpy(HB_Param[i][13].tags  , tags  );
   	  HB_Param[i][13].adc_min=0; HB_Param[i][13].adc_max=65535;
   	  HB_Param[i][13].eng_min=-2.0; HB_Param[i][13].eng_max=57.2;
      // FT672, ���.�./���, ������ ����� �.3 �� 7,6 ��� �� 5,4
   	  strcpy(metric, "flow");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=line3 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][14].metric, metric);
   	  strcpy(HB_Param[i][14].tags  , tags  );
   	  HB_Param[i][14].adc_min=0; HB_Param[i][14].adc_max=65535;
   	  HB_Param[i][14].eng_min=0.0; HB_Param[i][14].eng_max=5000.0;
      // , ���.�./���, ������� ������� ����� �.3 �� 7,6 ��� �� 5,4
   	  strcpy(metric, "flow");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=line3 dp=sp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][15].metric, metric);
   	  strcpy(HB_Param[i][15].tags  , tags  );
   	  HB_Param[i][15].adc_min=0; HB_Param[i][15].adc_max=65535;
   	  HB_Param[i][15].eng_min=0.0; HB_Param[i][15].eng_max=65535.0;
    	break;

    case GATEWAY_T04:
      // FT675, ���.�./���, ������ ����� �.4 �� 3,2 (��-5,6)
   	  strcpy(metric, "flow");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd23 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][0].metric, metric);
   	  strcpy(HB_Param[i][0].tags  , tags  );
   	  HB_Param[i][0].adc_min=0; HB_Param[i][0].adc_max=65535;
   	  HB_Param[i][0].eng_min=0.0; HB_Param[i][0].eng_max=3000.0;
      // SPRD23, ���.�./���, ������� ������� ��� �� 3,2 (��-5,6)
   	  strcpy(metric, "flow");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd23 dp=sp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][1].metric, metric);
   	  strcpy(HB_Param[i][1].tags  , tags  );
   	  HB_Param[i][1].adc_min=0; HB_Param[i][1].adc_max=65535;
   	  HB_Param[i][1].eng_min=0.0; HB_Param[i][1].eng_max=65535.0;
      // QT_P5, %, ��������� �� 3 (��-5)
   	  strcpy(metric, "pos");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd3 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][2].metric, metric);
   	  strcpy(HB_Param[i][2].tags  , tags  );
   	  HB_Param[i][2].adc_min=0; HB_Param[i][2].adc_max=65535;
   	  HB_Param[i][2].eng_min=-12.5; HB_Param[i][2].eng_max=112.5;
      // SPRD3, %, ������� ��������� �� 3 (��-5)
   	  strcpy(metric, "pos");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd3 dp=sp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][3].metric, metric);
   	  strcpy(HB_Param[i][3].tags  , tags  );
   	  HB_Param[i][3].adc_min=0; HB_Param[i][3].adc_max=65535;
   	  HB_Param[i][3].eng_min=0.0; HB_Param[i][3].eng_max=100.0;
      // QT_P6, %, ��������� �� 2 (��-6)
   	  strcpy(metric, "pos");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd2 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][4].metric, metric);
   	  strcpy(HB_Param[i][4].tags  , tags  );
   	  HB_Param[i][4].adc_min=0; HB_Param[i][4].adc_max=65535;
   	  HB_Param[i][4].eng_min=-12.5; HB_Param[i][4].eng_max=112.5;
      // SPRD2, %, ������� ��������� �� 2 (��-6)
   	  strcpy(metric, "pos");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd2 dp=sp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][5].metric, metric);
   	  strcpy(HB_Param[i][5].tags  , tags  );
   	  HB_Param[i][5].adc_min=0; HB_Param[i][5].adc_max=65535;
   	  HB_Param[i][5].eng_min=0.0; HB_Param[i][5].eng_max=100.0;
      // PT673, ���/��2, �������� �� ����� �� 3,2 (��-5,6)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd23"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][6].metric, metric);
   	  strcpy(HB_Param[i][6].tags  , tags  );
   	  HB_Param[i][6].adc_min=0; HB_Param[i][6].adc_max=65535;
   	  HB_Param[i][6].eng_min=-2.0; HB_Param[i][6].eng_max=57.2;
      // FT677, ���.�./���, ������ ����� �.1 �� 8,9 (��-7,8)
   	  strcpy(metric, "flow");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd89 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][7].metric, metric);
   	  strcpy(HB_Param[i][7].tags  , tags  );
   	  HB_Param[i][7].adc_min=0; HB_Param[i][7].adc_max=65535;
   	  HB_Param[i][7].eng_min=0.0; HB_Param[i][7].eng_max=2700.0;
      // SPRD89, ���.�./���, ������� ������� ��� �� 8,9 (��-7,8)
   	  strcpy(metric, "flow");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd89 dp=sp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][8].metric, metric);
   	  strcpy(HB_Param[i][8].tags  , tags  );
   	  HB_Param[i][8].adc_min=0; HB_Param[i][8].adc_max=65535;
   	  HB_Param[i][8].eng_min=0.0; HB_Param[i][8].eng_max=65535.0;
      // QT_P7, %, ��������� �� 8 (��-7)
   	  strcpy(metric, "pos");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd8 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][9].metric, metric);
   	  strcpy(HB_Param[i][9].tags  , tags  );
   	  HB_Param[i][9].adc_min=0; HB_Param[i][9].adc_max=65535;
   	  HB_Param[i][9].eng_min=-12.5; HB_Param[i][9].eng_max=112.5;
      // SPRD8, %, ������� ��������� �� 8 (��-7)
   	  strcpy(metric, "pos");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd8 dp=sp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][10].metric, metric);
   	  strcpy(HB_Param[i][10].tags  , tags  );
   	  HB_Param[i][10].adc_min=0; HB_Param[i][10].adc_max=65535;
   	  HB_Param[i][10].eng_min=0.0; HB_Param[i][10].eng_max=100.0;
      // QT_P8, %, ��������� �� 9 (��-8)
   	  strcpy(metric, "pos");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd9 dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][11].metric, metric);
   	  strcpy(HB_Param[i][11].tags  , tags  );
   	  HB_Param[i][11].adc_min=0; HB_Param[i][11].adc_max=65535;
   	  HB_Param[i][11].eng_min=-12.5; HB_Param[i][11].eng_max=112.5;
      // SPRD9, %, ������� ��������� �� 9 (��-8)
   	  strcpy(metric, "pos");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd9 dp=sp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][12].metric, metric);
   	  strcpy(HB_Param[i][12].tags  , tags  );
   	  HB_Param[i][12].adc_min=0; HB_Param[i][12].adc_max=65535;
   	  HB_Param[i][12].eng_min=0.0; HB_Param[i][12].eng_max=100.0;
      // PT674, ���/��2, �������� �� ����� �� 8,9 (��-7,8)
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , " node=rd89"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][13].metric, metric);
   	  strcpy(HB_Param[i][13].tags  , tags  );
   	  HB_Param[i][13].adc_min=0; HB_Param[i][13].adc_max=65535;
   	  HB_Param[i][13].eng_min=-2.0; HB_Param[i][13].eng_max=57.2;
    	break;
										
    default:;
    }

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

  // �������� �� ������������ �������� � ��������� ������ �������
  if((hdr->header<0 || hdr->header>=HAGENT_BUFEIP_MAX) ||
     (hdr->min   <0 || hdr->min   > 59) ||
     (hdr->hour  <0 || hdr->hour  > 23) ||
     (hdr->day   <1 || hdr->day   > 31) ||
     (hdr->month <1 || hdr->month > 12) ||
     (hdr->year  <0 || hdr->year  > 63) ||
     (hdr->prmnum<0 || hdr->prmnum> 31)  ) return 1;
     
  hdr->prmnum++;

  // ������������ ����� ������� ��������� � ������� Linux (��� ������)
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

///-----------------------------------------------------------------------------------------------------------------

///-----------------------------------------------------------------------------------------------------------------

///-----------------------------------------------------------------------------------------------------------------

///-----------------------------------------------------------------------------------------------------------------
