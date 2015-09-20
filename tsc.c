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

      // тест, ед. изм.
//   	strcpy(metric, "metric1");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
//   	strcpy(tags  , "tagk=tagv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
//   	strcpy(HB_Param[i][0].metric, metric);
//   	strcpy(HB_Param[i][0].tags  , tags  );
//   	HB_Param[i][0].adc_min=0; HB_Param[i][0].adc_max=35;
//   	HB_Param[i][0].eng_min=0; HB_Param[i][0].eng_max=66535.5;

//15	LT653, мм	Уровень в емкости ЕП-40 (нефть)	3605.0	0.0	65535	0	level	node=ep40

//      // \2, \3
//   	  strcpy(metric, "\8");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
//   	  strcpy(tags  , "\9"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
//   	  strcpy(HB_Param[i][\1].metric, metric);
//   	  strcpy(HB_Param[i][\1].tags  , tags  );
//   	  HB_Param[i][\1].adc_min=\7; HB_Param[i][\1].adc_max=\6;
//   	  HB_Param[i][\1].eng_min=\5; HB_Param[i][\1].eng_max=\4;

    case GATEWAY_T01:
      // QT671, %, Регулятор давления №1 - положение регулирующего затвора
   	  strcpy(metric, "pos");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , "sys=im1"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][0].metric, metric);
   	  strcpy(HB_Param[i][0].tags  , tags  );
   	  HB_Param[i][0].adc_min=2400; HB_Param[i][0].adc_max=21600;
   	  HB_Param[i][0].eng_min=-10.0; HB_Param[i][0].eng_max=110.0;

      // QT672, %, Регулятор давления №2 - положение регулирующего затвора
   	  strcpy(metric, "pos");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , "sys=im2"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][1].metric, metric);
   	  strcpy(HB_Param[i][1].tags  , tags  );
   	  HB_Param[i][1].adc_min=2400; HB_Param[i][1].adc_max=21600;
   	  HB_Param[i][1].eng_min=-10.0; HB_Param[i][1].eng_max=110.0;

      // FR671, Гц, Регулятор давления №1 - задание частоты на ПЧ ЭП
   	  strcpy(metric, "frq");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , "sys=im1"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][2].metric, metric);
   	  strcpy(HB_Param[i][2].tags  , tags  );
   	  HB_Param[i][2].adc_min=0; HB_Param[i][2].adc_max=65535;
   	  HB_Param[i][2].eng_min=-50.0; HB_Param[i][2].eng_max=50.0;

      // FR672, Гц, Регулятор давления №2 - задание частоты на ПЧ ЭП
   	  strcpy(metric, "frq");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , "sys=im2"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][3].metric, metric);
   	  strcpy(HB_Param[i][3].tags  , tags  );
   	  HB_Param[i][3].adc_min=0; HB_Param[i][3].adc_max=65535;
   	  HB_Param[i][3].eng_min=-50.0; HB_Param[i][3].eng_max=50.0;

      // PT003_1, МПа, Давление нефти до УРД
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , "sys=bfr dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][4].metric, metric);
   	  strcpy(HB_Param[i][4].tags  , tags  );
   	  HB_Param[i][4].adc_min=2400; HB_Param[i][4].adc_max=21600;
   	  HB_Param[i][4].eng_min=-1.0; HB_Param[i][4].eng_max=11.0;

      // PT005_1, МПа, Давление нефти после УРД
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , "sys=aft dp=cv"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][5].metric, metric);
   	  strcpy(HB_Param[i][5].tags  , tags  );
   	  HB_Param[i][5].adc_min=2400; HB_Param[i][5].adc_max=21600;
   	  HB_Param[i][5].eng_min=-1.0; HB_Param[i][5].eng_max=11.0;

      // PT003_2, МПа, Текущее значение уставки давления до УРД
   	  strcpy(metric, "pres");    if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
   	  strcpy(tags  , "sys=bfr dp=sp"); if(strlen(tags)  >(HAGENT_TAGS_LEN  -1)) return 3;
   	  strcpy(HB_Param[i][6].metric, metric);
   	  strcpy(HB_Param[i][6].tags  , tags  );
   	  HB_Param[i][6].adc_min=0; HB_Param[i][6].adc_max=65535;
   	  HB_Param[i][6].eng_min=0.0; HB_Param[i][6].eng_max=10.0;

    	break;

    case GATEWAY_T02:
    	return 1;
    	break;

    case GATEWAY_T03:
    	return 1;
    	break;

    case GATEWAY_T04:
    	return 1;
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

///-----------------------------------------------------------------------------------------------------------------

///-----------------------------------------------------------------------------------------------------------------

///-----------------------------------------------------------------------------------------------------------------

///-----------------------------------------------------------------------------------------------------------------
