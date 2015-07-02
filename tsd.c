/***********   XXXXXXXXX   *************
        MODBUS XXXXXXX SOFTWARE         
                    VERSION X.X         
        AO NPO SPETSELECTROMEHANIKA     
               BRYANSK 2015                 
***************************************/

///******************* МОДУЛЬ TSD ********************************

///=== TSD_H IMPLEMENTATION

#include <string.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <pthread.h>

#include "tsd.h"
#include "interfaces.h"

///=== TSD_H private variables

///=== TSD_H private functions

//******************************************************************************
int init_tsd_h(int port)
  {
  int i=port-GATEWAY_T01;
  char metric[128];

  if(port<GATEWAY_T01 || port>GATEWAY_T04) return 1;

  switch(port) {

    case GATEWAY_T01:
      // тест, ед. изм.
//    	strcpy(metric, "metric1"); if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
//    	strcpy(HB_Param[i][0].metric, metric);
//    	HB_Param[i][0].adc_min=0; HB_Param[i][0].adc_max=35;
//    	HB_Param[i][0].eng_min=0; HB_Param[i][0].eng_max=66535.5;

    	// Давление на выходе КПСОД_1 л.0 DN_1220 (нефть), кгс/см2
    	strcpy(metric, "sp.pt501"); if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
    	strcpy(HB_Param[i][0].metric, metric);
    	HB_Param[i][0].adc_min=3420; HB_Param[i][0].adc_max=20580;
    	HB_Param[i][0].eng_min=-2.0; HB_Param[i][0].eng_max=57.2;
    	// Давление на выходе КПСОД_2 л.9 DN_1020 (нефть), кгс/см2
    	strcpy(metric, "sp.pt502"); if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
    	strcpy(HB_Param[i][1].metric, metric);
    	HB_Param[i][1].adc_min=3420; HB_Param[i][1].adc_max=20580;
    	HB_Param[i][1].eng_min=-2.0; HB_Param[i][1].eng_max=57.2;
    	// Давление на выходе КПСОД_3 л.4 DN_820 (нефть), кгс/см2
    	strcpy(metric, "sp.pt503"); if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
    	strcpy(HB_Param[i][2].metric, metric);
    	HB_Param[i][2].adc_min=3420; HB_Param[i][2].adc_max=20580;
    	HB_Param[i][2].eng_min=-2.0; HB_Param[i][2].eng_max=57.2;
    	// Давление на выходе КПСОД_4 л.1 DN_630 (нефть), кгс/см2
    	strcpy(metric, "sp.pt504"); if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
    	strcpy(HB_Param[i][3].metric, metric);
    	HB_Param[i][3].adc_min=3420; HB_Param[i][3].adc_max=20580;
    	HB_Param[i][3].eng_min=-2.0; HB_Param[i][3].eng_max=57.2;
    	// Давление на выходе КПСОД_5 л.2 DN_630 (ДТ), кгс/см2
    	strcpy(metric, "sp.pt505"); if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
    	strcpy(HB_Param[i][4].metric, metric);
    	HB_Param[i][4].adc_min=3420; HB_Param[i][4].adc_max=20580;
    	HB_Param[i][4].eng_min=-2.0; HB_Param[i][4].eng_max=57.2;
    	// Давление на выходе КПСОД_6 л.7 DN_426 (мазут/ДТ), кгс/см2
    	strcpy(metric, "sp.pt506"); if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
    	strcpy(HB_Param[i][5].metric, metric);
    	HB_Param[i][5].adc_min=3420; HB_Param[i][5].adc_max=20580;
    	HB_Param[i][5].eng_min=-2.0; HB_Param[i][5].eng_max=57.2;
    	// Давление в КПСОД_1 л.0 DN_1220 (нефть), кгс/см2
    	strcpy(metric, "sp.pt507"); if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
    	strcpy(HB_Param[i][6].metric, metric);
    	HB_Param[i][6].adc_min=3420; HB_Param[i][6].adc_max=20580;
    	HB_Param[i][6].eng_min=-2.0; HB_Param[i][6].eng_max=57.2;
    	// Давление в КПСОД_2 л.9 DN_1020 (нефть), кгс/см2
    	strcpy(metric, "sp.pt508"); if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
    	strcpy(HB_Param[i][7].metric, metric);
    	HB_Param[i][7].adc_min=3420; HB_Param[i][7].adc_max=20580;
    	HB_Param[i][7].eng_min=-2.0; HB_Param[i][7].eng_max=57.2;
    	// Давление в КПСОД_3 л.4 DN_820 (нефть), кгс/см2
    	strcpy(metric, "sp.pt509"); if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
    	strcpy(HB_Param[i][8].metric, metric);
    	HB_Param[i][8].adc_min=3420; HB_Param[i][8].adc_max=20580;
    	HB_Param[i][8].eng_min=-2.0; HB_Param[i][8].eng_max=57.2;
    	// Давление в КПСОД_4 л.1 DN_630 (нефть), кгс/см2
    	strcpy(metric, "sp.pt510"); if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
    	strcpy(HB_Param[i][9].metric, metric);
    	HB_Param[i][9].adc_min=3420; HB_Param[i][9].adc_max=20580;
    	HB_Param[i][9].eng_min=-2.0; HB_Param[i][9].eng_max=57.2;
    	// Давление в КПСОД_5 л.2 DN_630 (ДТ), кгс/см2
    	strcpy(metric, "sp.pt511"); if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
    	strcpy(HB_Param[i][10].metric, metric);
    	HB_Param[i][10].adc_min=3420; HB_Param[i][10].adc_max=20580;
    	HB_Param[i][10].eng_min=-2.0; HB_Param[i][10].eng_max=57.2;
    	// Давление в КПСОД_6 л.7 DN_426 (мазут/ДТ), кгс/см2
    	strcpy(metric, "sp.pt512"); if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
    	strcpy(HB_Param[i][11].metric, metric);
    	HB_Param[i][11].adc_min=3420; HB_Param[i][11].adc_max=20580;
    	HB_Param[i][11].eng_min=-2.0; HB_Param[i][11].eng_max=57.2;
    	// Загазованность на площадке входа в тоннель, % НКПР
    	strcpy(metric, "sp.qt901"); if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
    	strcpy(HB_Param[i][12].metric, metric);
    	HB_Param[i][12].adc_min=3200; HB_Param[i][12].adc_max=20800;
    	HB_Param[i][12].eng_min=-5.0; HB_Param[i][12].eng_max=105.0;
    	// Уровень в емкости ЕП-25 (нефть), мм
    	strcpy(metric, "sp.lt651"); if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
    	strcpy(HB_Param[i][13].metric, metric);
    	HB_Param[i][13].adc_min=4000; HB_Param[i][13].adc_max=20000;
    	HB_Param[i][13].eng_min=0.0; HB_Param[i][13].eng_max=3355.0;
    	// Уровень в емкости ЕП-16 (ДТ), мм
    	strcpy(metric, "sp.lt652"); if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
    	strcpy(HB_Param[i][14].metric, metric);
    	HB_Param[i][14].adc_min=4000; HB_Param[i][14].adc_max=20000;
    	HB_Param[i][14].eng_min=0.0; HB_Param[i][14].eng_max=3355.0;
    	// Уровень в емкости ЕП-40 (нефть), мм
    	strcpy(metric, "sp.lt653"); if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
    	strcpy(HB_Param[i][15].metric, metric);
    	HB_Param[i][15].adc_min=4000; HB_Param[i][15].adc_max=20000;
    	HB_Param[i][15].eng_min=0.0; HB_Param[i][15].eng_max=3605.0;
    	return 3;
    	break;

    case GATEWAY_T02:
    	// Температура воздуха в КТП, град. С
    	strcpy(metric, "sp.tt851"); if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
    	strcpy(HB_Param[i][0].metric, metric);
    	HB_Param[i][0].adc_min=4000; HB_Param[i][0].adc_max=20000;
    	HB_Param[i][0].eng_min=-50.0; HB_Param[i][0].eng_max=100.0;
    	// Температура воздуха в щитовой КИП, град. С
    	strcpy(metric, "sp.tt852"); if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
    	strcpy(HB_Param[i][1].metric, metric);
    	HB_Param[i][1].adc_min=4000; HB_Param[i][1].adc_max=20000;
    	HB_Param[i][1].eng_min=-50.0; HB_Param[i][1].eng_max=100.0;
    	// Температура воздуха в шкафу УСО11.4, град. С
    	strcpy(metric, "sp.tt903"); if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
    	strcpy(HB_Param[i][2].metric, metric);
    	HB_Param[i][2].adc_min=4000; HB_Param[i][2].adc_max=20000;
    	HB_Param[i][2].eng_min=-50.0; HB_Param[i][2].eng_max=100.0;
    	// Температура воздуха в шкафу УСО11.1, град. С
    	strcpy(metric, "sp.tt900"); if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
    	strcpy(HB_Param[i][3].metric, metric);
    	HB_Param[i][3].adc_min=4000; HB_Param[i][3].adc_max=20000;
    	HB_Param[i][3].eng_min=-50.0; HB_Param[i][3].eng_max=100.0;
    	// Температура воздуха в шкафу УСО11.2, град. С
    	strcpy(metric, "sp.tt901"); if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
    	strcpy(HB_Param[i][4].metric, metric);
    	HB_Param[i][4].adc_min=4000; HB_Param[i][4].adc_max=20000;
    	HB_Param[i][4].eng_min=-50.0; HB_Param[i][4].eng_max=100.0;
    	// Температура воздуха в шкафу УСО11.3, град. С
    	strcpy(metric, "sp.tt902"); if(strlen(metric)>(HAGENT_METRIC_LEN-1)) return 2;
    	strcpy(HB_Param[i][5].metric, metric);
    	HB_Param[i][5].adc_min=4000; HB_Param[i][5].adc_max=20000;
    	HB_Param[i][5].eng_min=-50.0; HB_Param[i][5].eng_max=100.0;
    	break;

    case GATEWAY_T03:
      return 3;
    	break;

    case GATEWAY_T04:
      return 3;
    	break;
										
    default:;
    }

  return 0;
  }

///-----------------------------------------------------------------------------------------------------------------
int check_header(u8 *rsp_adu, HB_Header *hbhdr)
  {
  hbhdr->header =  (rsp_adu[TCPADU_START_LO    ]<<8)|rsp_adu[TCPADU_START_LO + 1];
  hbhdr->cycles =  (rsp_adu[TCPADU_START_LO + 2]<<8)|rsp_adu[TCPADU_START_LO + 3];
  
  hbhdr->min    = ((rsp_adu[TCPADU_START_LO + 4]<<8)|rsp_adu[TCPADU_START_LO + 5]) & 0x003f;
  hbhdr->hour   =(((rsp_adu[TCPADU_START_LO + 4]<<8)|rsp_adu[TCPADU_START_LO + 5]) & 0x07c0) >>  6;
  hbhdr->day    =(((rsp_adu[TCPADU_START_LO + 4]<<8)|rsp_adu[TCPADU_START_LO + 5]) & 0xf800) >> 11;

  hbhdr->month  = ((rsp_adu[TCPADU_START_LO + 6]<<8)|rsp_adu[TCPADU_START_LO + 7]) & 0x000f;
  hbhdr->year   =(((rsp_adu[TCPADU_START_LO + 6]<<8)|rsp_adu[TCPADU_START_LO + 7]) & 0x03f0) >>  4;
  hbhdr->prmnum =(((rsp_adu[TCPADU_START_LO + 6]<<8)|rsp_adu[TCPADU_START_LO + 7]) & 0x7c00) >> 10;
  hbhdr->msprec =(((rsp_adu[TCPADU_START_LO + 6]<<8)|rsp_adu[TCPADU_START_LO + 7]) & 0x8000)>0 ? 1:0;
  
  hbhdr->fault1 = ((rsp_adu[TCPADU_START_LO + 8]<<8)|rsp_adu[TCPADU_START_LO + 9]);
  hbhdr->fault2 = ((rsp_adu[TCPADU_START_LO +10]<<8)|rsp_adu[TCPADU_START_LO +11]);
  hbhdr->group1 = ((rsp_adu[TCPADU_START_LO +12]<<8)|rsp_adu[TCPADU_START_LO +13]);
  hbhdr->group2 = ((rsp_adu[TCPADU_START_LO +14]<<8)|rsp_adu[TCPADU_START_LO +15]);

  // проверка на корректность значений в заголовке буфера истории
  if((hbhdr->header<0 || hbhdr->header>255) ||
     (hbhdr->min   <0 || hbhdr->min   > 59) ||
     (hbhdr->hour  <0 || hbhdr->hour  > 23) ||
     (hbhdr->day   <1 || hbhdr->day   > 31) ||
     (hbhdr->month <1 || hbhdr->month > 12) ||
     (hbhdr->year  <0 || hbhdr->year  > 64) ||
     (hbhdr->prmnum<0 || hbhdr->prmnum> 31)  ) return 1;
     
  hbhdr->prmnum++;

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
  
  dst->fault1 = src->fault1;
  dst->fault2 = src->fault2;
  dst->group1 = src->group1;
  dst->group2 = src->group2;
  
  return;
	}
///-----------------------------------------------------------------------------------------------------------------

///-----------------------------------------------------------------------------------------------------------------

///-----------------------------------------------------------------------------------------------------------------

///-----------------------------------------------------------------------------------------------------------------

///-----------------------------------------------------------------------------------------------------------------
