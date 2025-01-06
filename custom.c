
#include <string.h>
#include <stdio.h>

#include "interfaces.h"
#include "moxagate.h"

static int shes1rp_sended_prev, shes2rp_sended_prev;
static int shes1rp_link_counter, shes2rp_link_counter;

// обмен между 1РП и 2РП ПП Шесхарис, система автоматики на MKLogic-500
int refresh_tcpslave_data_shes() {

  int i, n, src, dst;

  // защита от ошибочного применения
  if (!(
      (IfaceTCP[0].modbus_mode==IFACE_TCPSLAVE) && (IfaceTCP[0].PQueryIndex[MAX_QUERY_ENTRIES]==15) && (strcmp(IfaceTCP[0].description,"Имитатор КС А3 1РП")==0) &&
      (IfaceTCP[1].modbus_mode==IFACE_TCPSLAVE) && (IfaceTCP[1].PQueryIndex[MAX_QUERY_ENTRIES]==18) && (strcmp(IfaceTCP[1].description,"Имитатор КС А3 2РП")==0) &&
      (IfaceTCP[2].modbus_mode==IFACE_TCPSLAVE) && (IfaceTCP[2].PQueryIndex[MAX_QUERY_ENTRIES]==12) && (strcmp(IfaceTCP[2].description,"Имитатор КС А13 2РП")==0)
      )) {
    printf("refresh_tcpslave_data_shes not verified\n");
    return 1;
    }

  // флаг наличия связи по счётчикам обработанных запросов
  if(IfaceTCP[0].stat.sended != shes1rp_sended_prev) shes1rp_link_counter= Security.scan_counter;
  shes1rp_sended_prev= IfaceTCP[0].stat.sended;
  if(IfaceTCP[1].stat.sended != shes2rp_sended_prev) shes2rp_link_counter= Security.scan_counter;
  shes2rp_sended_prev= IfaceTCP[1].stat.sended;

  pthread_mutex_lock(&IfaceTCP[0].serial_mutex);
  pthread_mutex_lock(&IfaceTCP[1].serial_mutex);

  for(i=0; i<4; i++) {

    switch(i) {
      case 0: src= 27 - 1;
              dst=  9 - 1; // СА 2РП ПП «Шесхарис» (основной канал). Чтение 1
              break;
//    case 1: src= 33 - 1;
//            dst= 10 - 1; // СА 2РП ПП «Шесхарис» (основной канал). Чтение 2
//            break;
      case 2: src= 14 - 1; // СА 2РП ПП «Шесхарис» (основной канал). Запись 1
              dst= 17 - 1;
              break;
      case 3: src= 15 - 1; // СА 2РП ПП «Шесхарис» (основной канал). Запись 2
              dst= 26 - 1;
              break;
      default:
              src= 0;
              dst= 0;
              break;
      }

    if(!((src==0)&&(dst==0)))
      if(PQuery[dst].length >= PQuery[src].length)
        for(n=0; n<PQuery[src].length; n++)
          MoxaDevice.wData4x[PQuery[dst].offset+n] =
          MoxaDevice.wData4x[PQuery[src].offset+n] ;

    }

    src= 33 - 1;
    dst=  9 - 1; // СА 2РП ПП «Шесхарис» (основной канал). Чтение 1
    for(n=0; n<23; n++)
      MoxaDevice.wData4x[PQuery[dst].offset+n+100] =
      MoxaDevice.wData4x[PQuery[src].offset+n    ] ;

    src= 33 - 1;
    dst= 10 - 1; // СА 2РП ПП «Шесхарис» (основной канал). Чтение 2
    for(n=0; n<24; n++)
      MoxaDevice.wData4x[PQuery[dst].offset+n    ] =
      MoxaDevice.wData4x[PQuery[src].offset+n+ 25] ;

  // DiagState:        uDiagState;                 //   1-  4 - Диагностика - состояние
  // ms1970UTC:        ULINT;                      //   5-  8 - Текущая дата и время UTC в миллисекундах c 01.01.1970 00:00:00.000
  // PrjVersion:       sPrjVersion;                //   9- 16 - Версия проекта
  // SysState:         uSysState;                  //  17- 26 - Диагностика NTP - состояние    
  // mPSState:         umPSState;                  //  27- 34 - Модуль PS - состояние
  // mCPUState:        umCPUState;                 //  35- 48 - Модуль CPU - состояние
  // mEth4State:       umEth4State;                //  49- 50 - Модуль Eth4 - состояние 
  // mAI8State:        umAI8State;                 //  51     - Модуль AI8 - состояние
  // mDI32State:       umDI32State;                //  52     - Модуль DI32 - состояние 
  // mRS2State:        ARRAY [1..6] OF umRS2State; //  53- 64 - Модуль RS2 - состояние 
  // mAI8Value:        ARRAY [1..8] OF UINT;       //  65- 72 - Модуль AI8 - значения
  // mDI32Value:       DWORD;                      //  73- 74 - Модуль DI32 - значения
                                                   //  75- 76 - Резерв (выравнивание в памяти)
  // RSReqLinkOk:      LWORD;                      //  77- 80 - Запросы Modbus - запрос выполнился (бит 0 - запрос 1)
  // RSReqStatus:      ARRAY [1..40] OF USINT;     //  81-100 - Запросы Modbus - статус   
  // RSReqLinkOkCount: ARRAY [1..40] OF USINT;     // 101-120 - Запросы Modbus - счетчик выполненных запросов   

  dst=  1 - 1; // 1РП. Блок диагностики для передачи из КС А3
  MoxaDevice.wData4x[PQuery[dst].offset+ 1-1]= Security.scan_counter;
  MoxaDevice.wData4x[PQuery[dst].offset+27-1]= 0x4521; // состояние модуля
  MoxaDevice.wData4x[PQuery[dst].offset+35-1]= 0x4521; // состояние модуля
  MoxaDevice.wData4x[PQuery[dst].offset+51-1]= 0x4521; // состояние модуля
  MoxaDevice.wData4x[PQuery[dst].offset+52-1]= 0x4521; // состояние модуля
  MoxaDevice.wData4x[PQuery[dst].offset+53-1]= 0x4521; // состояние модуля
  MoxaDevice.wData4x[PQuery[dst].offset+55-1]= 0x4521; // состояние модуля
  MoxaDevice.wData4x[PQuery[dst].offset+57-1]= 0x4521; // состояние модуля
  MoxaDevice.wData4x[PQuery[dst].offset+59-1]= 0x4521; // состояние модуля
  MoxaDevice.wData4x[PQuery[dst].offset+61-1]= 0x4521; // состояние модуля
  MoxaDevice.wData4x[PQuery[dst].offset+63-1]= 0x4521; // состояние модуля
  MoxaDevice.wData4x[PQuery[dst].offset+54-1]= (((Security.scan_counter - shes1rp_link_counter)< 40) << 3) |
                                               (((Security.scan_counter - shes1rp_link_counter)>=40) << 5) |
                                               (                                                  1  << 7)   ;
  MoxaDevice.wData4x[PQuery[dst].offset+77-1]= (((Security.scan_counter - shes1rp_link_counter)< 40) << (15- 1)) |
                                               (((Security.scan_counter - shes1rp_link_counter)< 40) << (16- 1))   ;
  MoxaDevice.wData4x[PQuery[dst].offset+78-1]= (((Security.scan_counter - shes1rp_link_counter)< 40) << (17-17)) |
                                               (((Security.scan_counter - shes1rp_link_counter)< 40) << (18-17))   ;
//for(n=1; n<PQuery[dst].length; n++)
//  MoxaDevice.wData4x[PQuery[dst].offset+n]= n+1;

  // DiagState:   uDiagState;                 //   1-  4 - Диагностика - состояние
  // ms1970UTC:   ULINT;                      //   5-  8 - Текущая дата и время UTC в миллисекундах c 01.01.1970 00:00:00.000
  // PrjVersion:  sPrjVersion;                //   9- 16 - Версия проекта
  // SysState:    uSysState;                  //  17- 26 - Диагностика NTP - состояние    
  // mPSState:    umPSState;                  //  27- 34 - Модуль PS - состояние
  // mCPUState:   umCPUState;                 //  35- 48 - Модуль CPU - состояние
  // mEth4State:  umEth4State;                //  49- 50 - Модуль Eth4 - состояние 
  // mAI8State:   umAI8State;                 //  51     - Модуль AI8 - состояние
  // mDI32State:  umDI32State;                //  52     - Модуль DI32 - состояние 
  // mRS2State:   ARRAY [1..4] OF umRS2State; //  53- 60 - Модуль RS2 - состояние 
  // mAI8Value:   ARRAY [1..8] OF UINT;       //  61- 68 - Модуль AI8 - значения
  // mDI32Value:  DWORD;                      //  69- 70 - Модуль DI32 - значения
                                              //  71- 72 - Резерв (выравнивание в памяти)
  // RSReqLinkOk: LWORD;                      //  73- 76 - Запросы Modbus - запрос выполнился (бит 0 - запрос 1)
  // RSReqStatus: ARRAY [1..64] OF USINT;     //  77-108 - Запросы Modbus - статус   

  dst= 16 - 1; // 2РП. KSA3toKC_Diag
  MoxaDevice.wData4x[PQuery[dst].offset+0]= Security.scan_counter;
  MoxaDevice.wData4x[PQuery[dst].offset+27-1]= 0x4521; // состояние модуля
  MoxaDevice.wData4x[PQuery[dst].offset+35-1]= 0x4521; // состояние модуля
  MoxaDevice.wData4x[PQuery[dst].offset+51-1]= 0x4521; // состояние модуля
  MoxaDevice.wData4x[PQuery[dst].offset+52-1]= 0x4521; // состояние модуля
  MoxaDevice.wData4x[PQuery[dst].offset+53-1]= 0x4521; // состояние модуля
  MoxaDevice.wData4x[PQuery[dst].offset+55-1]= 0x4521; // состояние модуля
  MoxaDevice.wData4x[PQuery[dst].offset+57-1]= 0x4521; // состояние модуля
  MoxaDevice.wData4x[PQuery[dst].offset+59-1]= 0x4521; // состояние модуля
  MoxaDevice.wData4x[PQuery[dst].offset+54-1]= (((Security.scan_counter - shes2rp_link_counter)< 40) << 0) |
                                               (((Security.scan_counter - shes2rp_link_counter)>=40) << 2)   ;
  MoxaDevice.wData4x[PQuery[dst].offset+73-1]= (((Security.scan_counter - shes2rp_link_counter)< 40) << 0) |
                                               (((Security.scan_counter - shes2rp_link_counter)< 40) << 1) |
                                               (((Security.scan_counter - shes2rp_link_counter)< 40) << 2) |
                                               (((Security.scan_counter - shes2rp_link_counter)< 40) << 3)   ;
//for(n=1; n<PQuery[dst].length; n++)
//  MoxaDevice.wData4x[PQuery[dst].offset+n]= n+1;

  pthread_mutex_unlock(&IfaceTCP[1].serial_mutex);
  pthread_mutex_unlock(&IfaceTCP[0].serial_mutex);

  return 0;
  }

