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

#include <string.h>
#include <time.h>

#include "kltm.h"
#include "../interfaces.h"
#include "../moxagate.h"

#include "../serial/serial.h"
#include "../IEC870_3/IEC870_3.h"

///=== KLTM_H private variables
//коды запросов от ПЛК
#define PACK_CODE_MASK            0x7F
#define PACK_CODE_READ_STATUS     0x01 
#define PACK_CODE_READ_STATUS2    0x05
#define PACK_CODE_WRITE_CONFIG    0x02
#define PACK_CODE_PUT_EVENTS      0x03
#define PACK_CODE_SPE_REQ         0x04

kltm_sCom PLC, CM;
enum STATES_KM_MASTER{KM_INIT = 0,  KM_WORK};
enum STATES_KM_KLTM{KLTM_START = 0,READY_INIT, WAIT_KVIT, WAIT_QUERY_STATE, WAIT_INIT_DATA,  WORK_KLTM};
///=== KLTM_H private functions

void wrt_kltm_ack(kltm_sCom *PLC, kltm_sCom * CM)
{
  if(PLC->T_PAC_NUM != CM->R_PAC_NUM){
    CM->R_PAC_NUM = PLC->T_PAC_NUM;      
    CM->T_PAC_CODE = PLC->T_PAC_CODE; 
  }
  CM->T_PAC_NUM++;
  if(CM->T_PAC_NUM == 0) CM->T_PAC_NUM = 2;
}

void kltm_in_debug(kltm_sCom *PLC)
{
  printf("PLC->R_PAC_NUM: 0x%X\n", PLC->R_PAC_NUM);
  printf("PLC->T_PAC_NUM: 0x%X\n", PLC->T_PAC_NUM);
  printf("PLC->T_PAC_LEN: 0x%X\n", PLC->T_PAC_LEN);
  printf("PLC->T_PAC_CODE: 0x%X\n", PLC->T_PAC_CODE);
}

void kltm_debug_out(kltm_sCom *CM)
{
  printf("CM->R_PAC_NUM: 0x%X\n", CM->R_PAC_NUM);
  printf("CM->T_PAC_NUM: 0x%X\n", CM->T_PAC_NUM);
  printf("CM->T_PAC_LEN: 0x%X\n", CM->T_PAC_LEN);
  printf("CM->T_PAC_CODE: 0x%X\n", CM->T_PAC_CODE);
}

//перевод непоянтных циферок в конфигурации КП в нужный формат
unsigned int get_kp_seep(kp_cfg_t *kp_cfg){
switch(kp_cfg->BaudFb){
  case 2:
    return 1200;
  case 3:
    return 2400;
  case 4:
    return 4800;
  case 5:
    return 9600;
  case 6:
    return 19200;
  case 7:
    return 38400;
  case 8:
    return 57600;
  case 9:
    return 115200;
  default:
    return 115200;
  }
}

void set_kp_opts(kp_cfg_t *kp_cfg, int port, unsigned char *opts) {
  unsigned char *op;
  port_options_t port_options;

  op = opts;
  kp_cfg->addr = *op; op++; 
  kp_cfg->pack_size = *op; op++; //kp_cfg->LenPacIEC = *op; op++;
  kp_cfg->LenAnsFq = *op; op++;
  kp_cfg->RTS_CTS_Ctrl = *op; op++;
  kp_cfg->BaudFq = *op; op++;
  kp_cfg->LevelFq = *op; op++;
  kp_cfg->BaudFb = *op; op++;
  kp_cfg->MinPauseFb =  *op; op++;
  kp_cfg->TimeIEC =  *op | (8 >>*(op + 1));

  kp_cfg->state &= ~KP_CFG_ACK_MASK;
  kp_cfg->state |= KP_CFG_SHORT_ACK; //будем использовать короткое потверждение
  
  //задаем настройки опций порта
  port_options.baudrate = get_kp_seep(kp_cfg);
  
  if(kp_cfg->RTS_CTS_Ctrl != 0) port_options.flow_control = CTRCTS;
  else port_options.flow_control = NONE;

  port_options.party = EVEN;
  port_options.mode = RS232_MODE; 
  
  printf("kp_cfg->addr: %d\n", kp_cfg->addr);
  printf("kp_cfg->TimeIEC: %d\n", kp_cfg->TimeIEC);
  printf("port_options.baudrate: %d\n", port_options.baudrate);
  printf("kp_cfg->RTS_CTS_Ctrl: %d\n", kp_cfg->RTS_CTS_Ctrl);
  printf("port_options.flow_control: %d\n", port_options.flow_control);

  set_port_options(port, &port_options);  
}

unsigned int kltm_state_mashine_start(kltm_sCom *PLC, kltm_sCom * CM, kp_cfg_t *kp_cfg){
  unsigned int i;
  printf("KLTM_START\n");
  CM->R_PAC_NUM = 0;  
  CM->T_PAC_NUM = 0;  
  CM->T_PAC_LEN = 0;  
  CM->T_PAC_CODE = 0; 	  
  for(i=0; i<CM->T_PAC_LEN; i++) CM->T_DATA[i] = 0;
  kp_cfg->flags = KP_CFG_FLAGS_ERROR | KP_CFG_FLAGS_WAIT_INIT | KP_CFG_FLAGS_MASTER_NO_ACTIVITY;
  kltm_in_debug(PLC);
  return READY_INIT;
}

unsigned int kltm_state_mashine_ready_init(kltm_sCom *PLC, kltm_sCom *CM, kp_cfg_t *kp_cfg){
  printf("READY_INIT\n"); 
  CM->T_PAC_NUM = 1;
  kltm_in_debug(PLC);
  return WAIT_KVIT;
}

unsigned int kltm_state_mashine_wait_kvit(kltm_sCom *PLC, kltm_sCom *CM, kp_cfg_t *kp_cfg){
  printf("WAIT_KVIT\n");
  kltm_in_debug(PLC);
  if(PLC->R_PAC_NUM == 1){
    printf("PLC.R_PAC_NUM == 1\n"); 
    if((PLC->T_PAC_CODE & PACK_CODE_MASK) == PACK_CODE_READ_STATUS){
      printf("(PLC.T_PAC_CODE & PACK_CODE_MASK) == PACK_CODE_READ_STATUS\n");
      wrt_kltm_ack (PLC, CM);
      CM->T_PAC_LEN = 8;
      CM->T_DATA[0] = 0; 
      CM->T_DATA[1] = 0;
      CM->T_DATA[2] = 0;
      CM->T_DATA[3] = 0;
      CM->T_DATA[4] = kp_cfg->flags;
      CM->T_DATA[5] = 0xD;
      CM->T_DATA[6] = 0x28;
      CM->T_DATA[7] = 0xC0;
      kp_cfg->flags &= ~(KP_CFG_FLAGS_ERROR | KP_CFG_FLAGS_WAIT_INIT);
      return WAIT_INIT_DATA;
    } //else {
    //state_km_kltm = KLTM_START;
    //CM.T_PAC_NUM = 1;
    //}
  }
  return WAIT_KVIT;
}

unsigned int kltm_state_mashine_query_state(kltm_sCom *PLC, kltm_sCom *CM, kp_cfg_t *kp_cfg){
  printf("WAIT_QUERY_STATE\n");
  kltm_in_debug(PLC);
  if((PLC->T_PAC_CODE & PACK_CODE_MASK) == PACK_CODE_READ_STATUS){
    printf("(PLC.T_PAC_CODE & PACK_CODE_MASK) == PACK_CODE_READ_STATUS\n");
    wrt_kltm_ack (PLC, CM);
    CM->T_PAC_LEN = 3;
    CM->T_DATA[0] = 0; 
    CM->T_DATA[1] = 0;
    CM->T_DATA[2] = KP_CFG_FLAGS_WAIT_INIT;
    return WAIT_INIT_DATA;
  }
  return WAIT_QUERY_STATE;
}


 unsigned int kltm_state_mashine_wait_init_data(kltm_sCom *PLC, kltm_sCom *CM, int port, kp_cfg_t *kp_cfg){
  unsigned int i;
  printf("WAIT_INIT_DATA\n");
  kltm_in_debug(PLC);
  if((PLC->T_PAC_CODE != CM->T_PAC_CODE) && (PLC->T_PAC_CODE == PACK_CODE_WRITE_CONFIG)) { 
    printf("(PLC.T_PAC_CODE != CM.T_PAC_CODE) && (PLC.T_PAC_CODE == PACK_CODE_WRITE_CONFIG)\n");
    for(i = 0; i < 10; i++) printf("PLC.T_DATA[%d] = 0x%X\n", i,  PLC->T_DATA[i]);
    set_kp_opts(kp_cfg, port, &PLC->T_DATA[0]);
    wrt_kltm_ack (PLC, CM);
    CM->T_PAC_LEN = 0;
    return WORK_KLTM;
  }
  return WAIT_INIT_DATA;
}


unsigned int kltm_state_mashine_wait_work(kltm_sCom *PLC, kltm_sCom *CM, kp_cfg_t *kp_cfg, iec3_buf_data_send_t *iec3_buf){
   unsigned int i;
   printf("WORK_KLTM\n");
  if(PLC->R_PAC_NUM == 0) {
    return READY_INIT;
  }
  if(PLC->T_PAC_NUM != CM->R_PAC_NUM){
    printf("PLC->T_PAC_NUM != CM->R_PAC_NUM\n");
    kltm_in_debug(PLC);
    if((PLC->T_PAC_CODE & PACK_CODE_MASK) == PACK_CODE_READ_STATUS){
      if((kp_cfg->flags & KP_CFG_FLAGS_MASTER_QUERY) == KP_CFG_FLAGS_MASTER_QUERY){
	//есть команды к ПЛК
	printf("(kp_cfg->flags & KP_CFG_FLAGS_MASTER_QUERY) == KP_CFG_FLAGS_MASTER_QUERY\n");
	wrt_kltm_ack (PLC, CM);
	CM->T_DATA[0] = 0; //тут должна быть метка времени но ее нет
	CM->T_DATA[1] = 0;
	CM->T_DATA[2] = 0;
	CM->T_DATA[3] = 0;
	CM->T_DATA[4] = kp_cfg->flags;
	CM->T_PAC_LEN = 5 + kltm_buf_get_data(&CM->T_DATA[5], kp_cfg->kltm_max_pack_size, kp_cfg->kltm_out_buf);
	printf("CM->T_PAC_LEN = %d\n", CM->T_PAC_LEN);
	if(kp_cfg->kltm_out_buf->num == 0){
	  kp_cfg->flags &= ~KP_CFG_FLAGS_MASTER_QUERY;
	}
      }else{
	printf("(PLC.T_PAC_CODE & PACK_CODE_MASK) == PACK_CODE_READ_STATUS\n");
	wrt_kltm_ack (PLC, CM);
	CM->T_PAC_LEN = 11;
	CM->T_DATA[0] = 0; //тут должна быть метка времени но ее нет
	CM->T_DATA[1] = 0;
	CM->T_DATA[2] = 0;
	CM->T_DATA[3] = 0;
	CM->T_DATA[4] = kp_cfg->flags | KP_CFG_FLAGS_NO_DATA;
	CM->T_DATA[5] = iec3_buf->num;
	CM->T_DATA[6] = 0x28;
	CM->T_DATA[7] =       kp_cfg->MasterPackCnt & 0x000000FF;
	CM->T_DATA[8] =  8>>(kp_cfg->MasterPackCnt & 0x0000FF00);
	CM->T_DATA[9] =  16>>(kp_cfg->MasterPackCnt & 0x00FF0000);
	CM->T_DATA[10] = 32>>(kp_cfg->MasterPackCnt & 0xFF000000);
      }
    }else  if((PLC->T_PAC_CODE & PACK_CODE_MASK) == PACK_CODE_PUT_EVENTS){
      //#ifdef DEBUG
      printf("(PLC->T_PAC_CODE & PACK_CODE_MASK) == PACK_CODE_PUT_EVENTS\n");
      for(i = 0; i < PLC->T_PAC_LEN; i++){
	printf("PLC->T_DATA[%d] = %X\n", i, PLC->T_DATA[i]); 
      }
      //#endif
      CM->T_PAC_LEN = 1;
      CM->T_DATA[0] = iec3_convert_plc2kltm(&PLC->T_DATA[0], PLC->T_PAC_LEN,  kp_cfg, iec3_buf);
    }
     wrt_kltm_ack(PLC, CM);
  }
  return WORK_KLTM;
}

//автомат работы с мастером ЛТМ
unsigned int cm_master_state_mashime(unsigned int state_km_kltm, unsigned int state_km_master, int port,  kp_cfg_t *kp_cfg, iec3_buf_data_send_t *iec3_buf, iec3_buf_out_t *iec3_out_buf){
   unsigned int result; 
   unsigned int i; 
   char BUF_IN[BUF_IN_LEN];

   if(state_km_kltm != WORK_KLTM) return KM_INIT; 
   
   switch(state_km_master){
   case KM_INIT: 
     printf("INIT_KM_MASTER\n");
     iec3_init(kp_cfg, iec3_buf, iec3_out_buf);
     return KM_WORK; 
    break;
  case KM_WORK: 
    //работа 
    result = port_read(port, BUF_IN, BUF_IN_LEN);
    if (result !=0 ) {
      //printf("result = port_read(port, BUF_IN, BUF_IN_LEN) !=0 \n");
      read_iec3_data(BUF_IN, result, kp_cfg, iec3_buf); // Дешифруем принятый пакет
      wrt_out_buf(iec3_buf, iec3_out_buf);//перекладываем данные на передачу 
      if(iec3_out_buf->len != 0) {
	port_write(port, iec3_out_buf->data, iec3_out_buf->len);
      }

      if((kp_cfg->state & KP_CFG_GENERAL_POLL) == KP_CFG_GENERAL_POLL){
	//пришла команда общего опроса КП
	kp_cfg->state &= ~KP_CFG_GENERAL_POLL; //сбрасывается флаг общего опроса
      } 
      
      if((kp_cfg->state & SIGNAL_POLL) == SIGNAL_POLL){
	//пришла команда опроса какого-то сигнала
	kp_cfg->state &= ~SIGNAL_POLL;          //сбрасывается флаг общего опроса
      }
      
      if((kp_cfg->state & KP_CFG_INIT) == KP_CFG_INIT){
	//КП проинициализировано мастером
      }
    }	
    break;
   } 
   return KM_WORK;
}

//автомат работы с ПЛК
unsigned int cm_kltm_state_mashine(unsigned int state_km_kltm, kltm_sCom *PLC, kltm_sCom *CM, int port, kp_cfg_t *kp_cfg, iec3_buf_data_send_t *iec3_buf) {
  unsigned int i;
  switch(state_km_kltm){
  case KLTM_START:
    return kltm_state_mashine_start(PLC, CM, kp_cfg);
  case READY_INIT:
    return kltm_state_mashine_ready_init(PLC, CM, kp_cfg);
  case WAIT_KVIT:
    return kltm_state_mashine_wait_kvit(PLC, CM, kp_cfg);
  case WAIT_QUERY_STATE:
    return kltm_state_mashine_query_state(PLC, CM, kp_cfg);
  case WAIT_INIT_DATA:
    return kltm_state_mashine_wait_init_data(PLC, CM, port, kp_cfg);
  case WORK_KLTM:
    return kltm_state_mashine_wait_work(PLC, CM, kp_cfg, iec3_buf);
  }
}
///----------------------------------------------------------------------------
/// потоковая функция главного потока КМ-400
void *kltm_main(void *arg)
{
  int i;
  int  result;
  unsigned int state_km_master = KM_INIT;
  unsigned int state_km_kltm = KLTM_START;
  int port;
  kp_cfg_t kp_cfg;
  port_options_t port_options;
  iec3_buf_out_t iec3_out_buf; //буфер с данными которые будут отосланы
  iec3_buf_data_send_t iec3_buf; //здесь скалдируются данные на передачу
  kltm_out_buf_t  kltm_out_buf;  //здесь храниться информация будет о командах ПЛК от мастера ЛТМ
  time_t starttime, curtime;

  memset(&PLC, 0, sizeof(PLC));
  memset(&CM , 0, sizeof(CM ));
  
  Client[kltm_client].iface = kltm_port;
  VSlave[kltm_CM].critical=3; //!!! параметр должен вводиться при запуске, через командную строку

  printf("kltm_main: THREAD STARTED\n");
  
  //PQuery[kltm_PLC].delay  = 4000; // на время тестирования замедляем опрос
  //PQuery[kltm_PLC].length = 5;    // на время тестирования максимальный размер пакета - 5 регистров
  time(&starttime);
  
  /****Инициазизация канала обмена с мастером*******/
//  port = open_port(COM3);
//  if(port < 0 ){
//    printf("Ошибка открытия порта: ttyM%d\n", COM3);
//    return 0;
//  }
  
  /*******************************************/
  /*Привязывем выходной буфер с командами мастера ЛТМ для ПЛК к конфигурации КП
   пусть пока будет так, позднее можно будет включить в структуру не ссылку а переменную*/
  kp_cfg.kltm_out_buf = &kltm_out_buf;
  kp_cfg.kltm_max_pack_size = 50;

  while (1) {
    
    time(&curtime);
    
    if(curtime-starttime > 1) { // условие выполняется каждые 1н секунд
      starttime=curtime;
      //printf("kltm_main: tick\n");
      
      if(kltm_getPLC(&PLC)==0) {
        if(PLC.R_PAC_NUM != CM.R_PAC_NUM) {
          CM.R_PAC_NUM  = PLC.R_PAC_NUM; // номер пакета принятого
          CM.T_PAC_NUM  = PLC.T_PAC_NUM; // номер пакета отправленного
          CM.T_PAC_LEN  = PLC.T_PAC_LEN; // длина блока данных в поле T_DATA
          CM.T_PAC_CODE = PLC.T_PAC_CODE; // код пакета (операции)
          for(i=0; i<CM.T_PAC_LEN; i++)
            CM.T_DATA[i] = PLC.T_DATA[i];
          kltm_setCM(&CM);
          }
        } else {
          printf("kltm_main: connection to KLTM broken\n");
          }
      }
    } // while(1)
  
  // THREAD STOPPED
  // sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|GATEWAY_MOXAGATE, 43, 0, 0, 0, 0);
  pthread_exit (0);	
}
///----------------------------------------------------------------------------
