/***********   K M - 4 0 0   *************
      КОММУНИКАЦИОННЫЙ МОДУЛЬ                                
                              ВЕРСИЯ 1.0
      ООО "БЗПА"
               БРЯНСК 2013                 
*****************************************/

///**** МОДУЛЬ ДЛЯ РАБОТЫ С КОНТРОЛЛЕРОМ ТЕЛЕМЕХАНИКИ ПО ЧАСТНОМУ ПРОТОКОЛУ
///**** ("ПРОТОКОЛ ПЕРЕДАЧИ ПАКЕТОВ", РАБОТАЮЩИЙ ПОВЕРХ MODBUS RTU/MODBUS TCP)

/* Предполагается использование этого модуля в параллельном потоке программы,
   требуется настроенный интерфейс типа IFACE_RTUMASTER или IFACE_TCPMASTER
   для циклического опроса и записи значений переменных PLC и CM в КЛТМ */

#include <pthread.h>

#include "kltm.h"
#include "../interfaces.h"
#include "../moxagate.h"

///=== KLTM_H private variables

GW_Iface *kltm_iface; // указатель на структуру параметров интерфейса

///=== KLTM_H private functions


///----------------------------------------------------------------------------
// условно конструктор
int  init_kltm_h()
  {
  int i;
  	
  // проверка идентификатора интерфейса
  if( ((kltm_port > GATEWAY_P8 ) && (kltm_port < GATEWAY_T01)) ||
       (kltm_port > GATEWAY_T32)
    ) return 1;

  if(kltm_port<=GATEWAY_P8) {
    i=kltm_port;
    kltm_iface=&IfaceRTU[i];
  } else {
    i=kltm_port - GATEWAY_T01;
    kltm_iface=&IfaceTCP[i];
    }

  // проверка режима работы интерфейса
  if(!(
       kltm_iface->modbus_mode==IFACE_RTUMASTER ||
       kltm_iface->modbus_mode==IFACE_TCPMASTER
    )) return 2;

  kltm_PLC=kltm_iface->PQueryIndex[0];
  // проверка таблицы опроса интерфейса
  if( (kltm_iface->PQueryIndex[MAX_QUERY_ENTRIES] != 1)  ||
      (PQuery[kltm_PLC].mbf!=MBF_READ_HOLDING_REGISTERS) ||
      (PQuery[kltm_PLC].access!=QT_ACCESS_READONLY)      ||
      (PQuery[kltm_PLC].length!=KLTM_SCOM_STRUCT_SIZE)   ) return 3;

  // запись в таблице виртуальных устройств существует
  for(kltm_CM=0; kltm_CM<MAX_VIRTUAL_SLAVES; kltm_CM++)
    if(VSlave[kltm_CM].iface==kltm_port) break;
  if(kltm_CM==MAX_VIRTUAL_SLAVES) return 4;

  // проверка таблицы виртуальных устройств интерфейса
    if( (VSlave[kltm_CM].device != PQuery[kltm_PLC].device) ||
        (VSlave[kltm_CM].length != PQuery[kltm_PLC].length) ||
        (VSlave[kltm_CM].modbus_table!=HOLDING_REGISTER_TABLE) ) return 5;

  return 0;
  }

///----------------------------------------------------------------------------
// получить от КЛТМ запрос (актуальное значение переменной PLC)
int kltm_getPLC(kltm_sCom *plc)
  {
  static int i;
  
  // проверка бита наличия связи с КЛТМ
  if(PQuery[kltm_PLC].status_bit == 0) return 1;
  
  pthread_mutex_lock(&kltm_iface->serial_mutex);
  
  plc->R_PAC_NUM  = MoxaDevice.wData4x[PQuery[kltm_PLC].offset + 0] & 0x00ff; // номер пакета принятого
  plc->T_PAC_NUM  = MoxaDevice.wData4x[PQuery[kltm_PLC].offset + 0] >>     8; // номер пакета отправленного
  plc->T_PAC_LEN  = MoxaDevice.wData4x[PQuery[kltm_PLC].offset + 1] & 0x00ff; // актуальная длина блока данных в поле T_DATA
  plc->T_PAC_CODE = MoxaDevice.wData4x[PQuery[kltm_PLC].offset + 1] >>     8; // код пакета (операции)

  //!!! здесь реализовывать дополнительные проверки на корректность данных в принятой переменной PLC

  if(plc->T_PAC_LEN > KLTM_DATA_FIELD_SIZE) {
    pthread_mutex_unlock(&kltm_iface->serial_mutex);
  	return 2;
    }

  // чтение массива с данными
  for(i=0; i<plc->T_PAC_LEN; i++)
    plc->T_DATA[i] = (i%2)==0 ?
      MoxaDevice.wData4x[PQuery[kltm_PLC].offset + 2 + i/2] & 0x00ff :
      MoxaDevice.wData4x[PQuery[kltm_PLC].offset + 2 + i/2] >>     8 ;
  
  pthread_mutex_unlock(&kltm_iface->serial_mutex);
  
  return 0;
  }
  	
///----------------------------------------------------------------------------
// записать в  КЛТМ ответ  (новое значение переменной CM)
int kltm_setCM (kltm_sCom *cm)
  {
  static int i;

  if(cm->T_PAC_LEN > KLTM_DATA_FIELD_SIZE) return 1;

  Client[kltm_client].out_len = cm->T_PAC_LEN+4 + TCPADU_FUNCTION;
  make_tcp_adu(Client[kltm_client].out_adu,
	             Client[kltm_client].out_len);
  Client[kltm_client].out_len += TCPADU_ADDRESS;

//Client[kltm_client].out_adu[TCPADU_ADDRESS ] = VSlave[kltm_CM].device      ;   // 6
  Client[kltm_client].out_adu[TCPADU_FUNCTION] = MBF_WRITE_MULTIPLE_REGISTERS;   // 7

  Client[kltm_client].out_adu[TCPADU_START_HI] = VSlave[kltm_CM].start >>   8;   // 8
  Client[kltm_client].out_adu[TCPADU_START_LO] = VSlave[kltm_CM].start & 0xff;   // 9
//  printf("kltm_setCM: %d:[%X][%X]\n", VSlave[kltm_CM].start,
//                                      Client[kltm_client].out_adu[TCPADU_START_HI],
//  	                                  Client[kltm_client].out_adu[TCPADU_START_LO]);

  Client[kltm_client].out_adu[TCPADU_LEN_HI  ] = (cm->T_PAC_LEN+4 + 1)/2 >>   8; // 10
  Client[kltm_client].out_adu[TCPADU_LEN_LO  ] = (cm->T_PAC_LEN+4 + 1)/2 & 0xff; // 11
  Client[kltm_client].out_adu[TCPADU_BYTECOUNT]=((cm->T_PAC_LEN+4 + 1)/2)*2;     // 12

  // запись массива с данными
  Client[kltm_client].out_adu[TCPADU_BYTECOUNT+2]= cm->R_PAC_NUM;
  Client[kltm_client].out_adu[TCPADU_BYTECOUNT+1]= cm->T_PAC_NUM;
  Client[kltm_client].out_adu[TCPADU_BYTECOUNT+4]= cm->T_PAC_LEN;
  Client[kltm_client].out_adu[TCPADU_BYTECOUNT+3]= cm->T_PAC_CODE;

  for(i=0; i<cm->T_PAC_LEN; i++)
    Client[kltm_client].out_adu[TCPADU_BYTECOUNT+5+i] = i%2==0 ? cm->T_DATA[i+1] : cm->T_DATA[i-1];

  if(i%2!=0) {
  	Client[kltm_client].out_adu[TCPADU_BYTECOUNT+5+i] = cm->T_DATA[i-1];
  	Client[kltm_client].out_adu[TCPADU_BYTECOUNT+5+i-1] = 0;
    Client[kltm_client].out_len++;
    }
  
  /// ставим запрос в очередь MASTER-интерфейса
  if(enqueue_query_ex(&kltm_iface->queue,
	                    GW_CLIENT_KM400,
	                    (FRWD_TYPE_REGISTER<<8)|(kltm_CM&0xff),
	                    Client[kltm_client].out_adu,
	                    Client[kltm_client].out_len)!=0) return 2;

  return 0;
  }

///----------------------------------------------------------------------------
