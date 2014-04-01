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
#include "../cmx868a/cmx868a.h"
///=== KLTM_H private variables
//коды запросов от ПЛК
#define PACK_CODE_MASK            0x7F
#define PACK_CODE_READ_STATUS     0x01 
#define PACK_CODE_READ_STATUS2    0x05
#define PACK_CODE_WRITE_CONFIG    0x02
#define PACK_CODE_PUT_EVENTS      0x03
#define PACK_CODE_SPEC_REQ        0x04

/*Коды сервесных функций*/
#define SF_NORNAL    0x00    //вернутся в нормальный режим
#define SF_ZERO_F    0x01    //выдача частоты соотвествующей 0
#define SF_ONE_F     0x02    //выдача частоты соотвествующей 1
#define SF_PINTS     0x03    //выдача точек, то есть 10101010101

/*Пишем в контроллер постоянно переменную CM  или нет*/
#define MULTIPLI_CM_WRT 0

kltm_sCom PLC, CM;
enum STATES_KM_MASTER{KM_INIT = 0,  KM_WORK};
enum STATES_KM_KLTM{KLTM_START = 0,READY_INIT, WAIT_KVIT, WAIT_INIT_DATA,  WORK_KLTM};

unsigned int state_km_kltm = KLTM_START; //переменная состояния автомата работаы с контроллером линейной телемеханики
//тип параметров для передачи в потоковую функцию обработки данных канала связи с мастером сети линейной телемеханики
typedef struct{
  kp_cfg_t *kp_cfg;
  unsigned int index;
}chanel_thread_param_t;

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

void kltm_debug_PLC(kltm_sCom *PLC)
{
  #ifdef DEBUG_KLTM_MAIN
  printf("PLC->R_PAC_NUM: 0x%X\n", PLC->R_PAC_NUM);
  printf("PLC->T_PAC_NUM: 0x%X\n", PLC->T_PAC_NUM);
  printf("PLC->T_PAC_LEN: 0x%X\n", PLC->T_PAC_LEN);
  printf("PLC->T_PAC_CODE: 0x%X\n", PLC->T_PAC_CODE);
  #endif
}

void kltm_debug_CM(kltm_sCom *CM)
{
  #ifdef DEBUG_KLTM_MAIN
  printf("CM->R_PAC_NUM: 0x%X\n", CM->R_PAC_NUM);
  printf("CM->T_PAC_NUM: 0x%X\n", CM->T_PAC_NUM);
  printf("CM->T_PAC_LEN: 0x%X\n", CM->T_PAC_LEN);
  printf("CM->T_PAC_CODE: 0x%X\n", CM->T_PAC_CODE);
  #endif
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

int reset_tonal_modem(kp_cfg_t *kp_cfg, int index){
  //сброс модема ТЧ 
  int result;
  if((kp_cfg->mltm[index].flags & CHANEL_INIT) == 0){
    //канал не инициализирован можно по новой открыть порт
    result = cmx868a_init();
    if( result != 0){
      printf("kltm_main:reset_tonal_modem -> cmx868a_init() error 1 %d", result);
      return 1;
    }
    kp_cfg->mltm[index].flags |= CHANEL_INIT;
    return 0;
  }
  
  //порт уже открыт, его нужно закрыть!!!
  cmx868a_close();
  kp_cfg->mltm[index].flags &= ~CHANEL_INIT;
  result = cmx868a_init();
  if( result != 0){
    printf("kltm_main:reset_tonal_modem -> cmx868a_init() error 2 %d", result);
    return 1;
    }
  kp_cfg->mltm[index].flags |= CHANEL_INIT;
  return 0;
}

void set_kp_opts(kp_cfg_t *kp_cfg, unsigned char *opts) {
  unsigned char *op;
  port_options_t port_options;
  int i;

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
  for( i = 0; i < kp_cfg->mltm_max; i++){ 
    if(kp_cfg->mltm[i].type == SERIAL_TYPE){
      port_options.mode = kp_cfg->mltm[i].mode; 
      set_port_options(kp_cfg->mltm[i].port, &port_options);
    }
  }
  
  #ifdef DEBUG_KLTM_MAIN
  printf("kp_cfg->addr: %d\n", kp_cfg->addr);
  printf("kp_cfg->pack_size: %d\n", kp_cfg->pack_size);
  printf("kp_cfg->LenAnsFq: %d\n", kp_cfg->LenAnsFq);
  printf("kp_cfg->RTS_CTS_Ctrl: %Xh\n", kp_cfg->RTS_CTS_Ctrl);
  printf("kp_cfg->BaudFq: %d - Скорость обмена с мастер-контроллером по ТЧ\n", kp_cfg->BaudFq);
  printf("kp_cfg->LevelFq: %d - Уровень передачи по ТЧ\n", kp_cfg->LevelFq);
  printf("kp_cfg->BaudFb: %d - Скорость обмена с мастер-контроллером по ВОЛС\n", kp_cfg->BaudFb);
  printf("kp_cfg->MinPauseFb: %d\n", kp_cfg->MinPauseFb);
  printf("kp_cfg->TimeIEC: %d\n", kp_cfg->TimeIEC);
  printf("kp_cfg->RTS_CTS_Ctrl: %d\n", kp_cfg->RTS_CTS_Ctrl);
  
  printf("port_options.baudrate: %d\n", port_options.baudrate);
  printf("port_options.flow_control: %d - Минимальная пауза между пакетами по ВОЛС\n", port_options.flow_control);
  #endif  
}

unsigned int kltm_state_mashine_start(kltm_sCom *PLC, kltm_sCom * CM, kp_cfg_t *kp_cfg){
  unsigned int i;
  #ifdef DEBUG_KLTM_MAIN
  printf("KLTM_START\n");
  #endif
  CM->R_PAC_NUM = 0;  
  CM->T_PAC_NUM = 0;  
  CM->T_PAC_LEN = 0;  
  CM->T_PAC_CODE = 0; 	  
  for(i=0; i<CM->T_PAC_LEN; i++) CM->T_DATA[i] = 0;
  kp_cfg->flags = KP_CFG_FLAGS_ERROR | KP_CFG_FLAGS_WAIT_INIT | KP_CFG_FLAGS_MASTER_NO_ACTIVITY;
  kltm_debug_PLC(PLC);
  
  if(MULTIPLI_CM_WRT == 0){
    kltm_debug_CM(CM);
    kltm_setCM(CM);
  }
  
 return READY_INIT;
}

unsigned int kltm_state_mashine_ready_init(kltm_sCom *PLC, kltm_sCom *CM, kp_cfg_t *kp_cfg){
  #ifdef DEBUG_KLTM_MAIN
  printf("READY_INIT\n"); 
  #endif
  CM->T_PAC_NUM = 1;
  kltm_debug_PLC(PLC);
  
  if(MULTIPLI_CM_WRT == 0){
    kltm_debug_CM(CM);
    kltm_setCM(CM);
  }

  return WAIT_KVIT;
}

unsigned int kltm_state_mashine_wait_kvit(kltm_sCom *PLC, kltm_sCom *CM, kp_cfg_t *kp_cfg){
  #ifdef DEBUG_KLTM_MAIN
  printf("WAIT_KVIT\n");
  #endif
  kltm_debug_PLC(PLC);
  if(PLC->R_PAC_NUM == 1){
    #ifdef DEBUG_KLTM_MAIN
    printf("PLC.R_PAC_NUM == 1\n"); 
    #endif
    if((PLC->T_PAC_CODE & PACK_CODE_MASK) == PACK_CODE_READ_STATUS){
      #ifdef DEBUG_KLTM_MAIN
      printf("(PLC.T_PAC_CODE & PACK_CODE_MASK) == PACK_CODE_READ_STATUS\n");
      #endif
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
      kltm_debug_PLC(PLC);
      
      if(MULTIPLI_CM_WRT == 0){
	kltm_debug_CM(CM);
	kltm_setCM(CM);
      }

      return WAIT_INIT_DATA;
    } 
  }
  return WAIT_KVIT;
}

 unsigned int kltm_state_mashine_wait_init_data(kltm_sCom *PLC, kltm_sCom *CM, kp_cfg_t *kp_cfg){
  #ifdef DEBUG_KLTM_MAIN
  printf("WAIT_INIT_DATA\n");
  #endif
  kltm_debug_PLC(PLC);
  if((PLC->T_PAC_CODE != CM->T_PAC_CODE) && (PLC->T_PAC_CODE == PACK_CODE_WRITE_CONFIG)) { 
    #ifdef DEBUG_KLTM_MAIN
    printf("(PLC.T_PAC_CODE != CM.T_PAC_CODE) && (PLC.T_PAC_CODE == PACK_CODE_WRITE_CONFIG)\n");
    for(i = 0; i < 10; i++) {
      printf("PLC.T_DATA[%d] = 0x%X\n", i,  PLC->T_DATA[i]);
    }
    #endif
    set_kp_opts(kp_cfg, &PLC->T_DATA[0]);
    
    wrt_kltm_ack (PLC, CM);
    CM->T_PAC_LEN = 0;
    kltm_debug_PLC(PLC);
    
    if(MULTIPLI_CM_WRT == 0){
      kltm_debug_CM(CM);
      kltm_setCM(CM);
    }

    return WORK_KLTM;
  }
  return WAIT_INIT_DATA;
}


unsigned int kltm_state_mashine_work(kltm_sCom *PLC, kltm_sCom *CM, kp_cfg_t *kp_cfg){
   unsigned int i;

   #ifdef DEBUG_KLTM_MAIN
   printf("WORK_KLTM\n");
   #endif
   if(PLC->R_PAC_NUM == 0) {
     #ifdef DEBUG_KLTM_MAIN
     printf("PLC->R_PAC_NUM == 0\n");
     #endif
     return KLTM_START;
   }

   if(PLC->T_PAC_NUM == CM->R_PAC_NUM){
     return WORK_KLTM;
   }
   #ifdef DEBUG_KLTM_MAIN
   printf("PLC->T_PAC_NUM != CM->R_PAC_NUM\n");
   #endif
  
   if((PLC->T_PAC_CODE & PACK_CODE_MASK) == PACK_CODE_READ_STATUS){
    //чтение статуса КМ
    if((kp_cfg->flags & KP_CFG_FLAGS_MASTER_QUERY) == KP_CFG_FLAGS_MASTER_QUERY){
      //есть команды к ПЛК
      #ifdef DEBUG_KLTM_MAIN
      printf("(kp_cfg->flags & KP_CFG_FLAGS_MASTER_QUERY) == KP_CFG_FLAGS_MASTER_QUERY\n");
      #endif
      wrt_kltm_ack (PLC, CM);
      CM->T_DATA[0] = kp_cfg->time.tch[0]; // метка времени
      CM->T_DATA[1] = kp_cfg->time.tch[1];
      CM->T_DATA[2] = kp_cfg->time.tch[2];
      CM->T_DATA[3] = kp_cfg->time.tch[3];
      CM->T_DATA[4] = kp_cfg->flags;
      kp_cfg->flags &= ~KP_CFG_FLAGS_MASTER_TIME;
      pthread_mutex_lock(&kp_cfg->mutex_a);
      CM->T_PAC_LEN = 5 + kltm_buf_get_data(&CM->T_DATA[5], kp_cfg->kltm_max_pack_size, kp_cfg->kltm_out_buf);
      pthread_mutex_unlock(&kp_cfg->mutex_a);
      if(kp_cfg->kltm_out_buf->num == 0){
	kp_cfg->flags &= ~KP_CFG_FLAGS_MASTER_QUERY;
      }
    }else{
      //команд к ПЛК нет
      #ifdef DEBUG_KLTM_MAIN
      printf("(PLC.T_PAC_CODE & PACK_CODE_MASK) == PACK_CODE_READ_STATUS\n");
      #endif
      wrt_kltm_ack (PLC, CM);
      CM->T_PAC_LEN = 11;
      CM->T_DATA[0] = kp_cfg->time.tch[0]; //метка времени
      CM->T_DATA[1] = kp_cfg->time.tch[1];
      CM->T_DATA[2] = kp_cfg->time.tch[2];
      CM->T_DATA[3] = kp_cfg->time.tch[3];
      CM->T_DATA[4] = kp_cfg->flags | KP_CFG_FLAGS_NO_DATA;
      kp_cfg->flags &= ~KP_CFG_FLAGS_MASTER_TIME;
      if(kp_cfg->mltm[0].large_out_buf.num > kp_cfg->mltm[1].large_out_buf.num){
	CM->T_DATA[5] = kp_cfg->mltm[0].large_out_buf.num;
      }else{
	CM->T_DATA[5] = kp_cfg->mltm[1].large_out_buf.num;
      }
      CM->T_DATA[6] = 0x28;
      CM->T_DATA[7] =  kp_cfg->MasterPackCnt & 0x000000FF;
      CM->T_DATA[8] =  8>>(kp_cfg->MasterPackCnt & 0x0000FF00);
      CM->T_DATA[9] =  16>>(kp_cfg->MasterPackCnt & 0x00FF0000);
      CM->T_DATA[10] = 32>>(kp_cfg->MasterPackCnt & 0xFF000000);
    }
  }
  
  if((PLC->T_PAC_CODE & PACK_CODE_MASK) == PACK_CODE_PUT_EVENTS){
    //разбор очереди событий
    wrt_kltm_ack(PLC, CM);
    #ifdef DEBUG_KLTM_MAIN
    printf("(PLC->T_PAC_CODE & PACK_CODE_MASK) == PACK_CODE_PUT_EVENTS\n");
    for(i = 0; i < PLC->T_PAC_LEN; i++){
      printf("PLC->T_DATA[%d] = %X\n", i, PLC->T_DATA[i]); 
    }
    #endif
    CM->T_PAC_LEN = 1;
    for(i = 0; i < kp_cfg->mltm_max; i++){
      if(kp_cfg->mltm[i].state == KM_WORK && ((kp_cfg->mltm[i].flags & KP_CFG_FLAGS_MASTER_NO_ACTIVITY) == 0)){
	pthread_mutex_lock(&kp_cfg->mltm[i].mutex[1]);
	//pthread_mutex_lock(&kp_cfg->mutex);
	CM->T_DATA[0] = iec3_convert_plc2kltm(&PLC->T_DATA[0], PLC->T_PAC_LEN,  kp_cfg, &kp_cfg->mltm[i].large_out_buf);
	//pthread_mutex_unlock(&kp_cfg->mutex);
	pthread_mutex_unlock(&kp_cfg->mltm[i].mutex[1]);
      }
    }
    #ifdef DEBUG_KLTM_MAIN
    printf("CM->T_DATA[0] = %X\n", CM->T_DATA[0]);
    #endif
  }
  
  if((PLC->T_PAC_CODE & PACK_CODE_MASK) == PACK_CODE_SPEC_REQ){
    //выполнение сервисной функции
    wrt_kltm_ack(PLC, CM);
    CM->T_PAC_LEN = 1;
    CM->T_DATA[0] = 0;
    printf("Код тестовой функции %X\n", PLC->T_DATA[0]);
  
    kp_cfg->flags |= KP_CFG_FLAGS_TEST_FUNC;
    switch(PLC->T_DATA[0]){
    case SF_NORNAL:
      kp_cfg->flags &= ~KP_CFG_FLAGS_TEST_FUNC;
      break;
    case SF_ZERO_F:
    case SF_ONE_F:
    case SF_PINTS:
      break;
    }
    cmx868a_close();
    /*На будущее. Код ищет канал настроенный на работу с ТЧ и производит его реконфигурацию*/
    for( i = 0; i <  kp_cfg->mltm_max; i++){
      if(kp_cfg->mltm[i].type == TONAL_TYPE){
	reset_tonal_modem(kp_cfg, i);
	cmx868a_test(PLC->T_DATA[0]);
	break;
      }
    }
  }
  
  kltm_debug_PLC(PLC);
  if(MULTIPLI_CM_WRT == 0){
    kltm_debug_CM(CM);
    kltm_setCM(CM);
  }

  return WORK_KLTM;
}

//обмен данными с мастером через канал последовательного порта RS-232/485
void cm_master_state_mashime_serial_com(kp_cfg_t *kp_cfg, int index, time_t curtime){
  unsigned int result; 
  
  //читаем данный из последовательного порта
  result = port_read(kp_cfg->mltm[index].port,kp_cfg-> mltm_buf_in, BUF_IN_LEN);
  if (result !=0 ) {
    pthread_mutex_lock(&kp_cfg->mltm[index].mutex[0]);
    //pthread_mutex_lock(&kp_cfg->mutex);
    read_iec3_data(kp_cfg-> mltm_buf_in, result, kp_cfg, &kp_cfg->mltm[index].large_out_buf);    // Дешифруем принятый пакет
    //pthread_mutex_unlock(&kp_cfg->mutex);
    pthread_mutex_unlock(&kp_cfg->mltm[index].mutex[0]);
    wrt_out_buf(&kp_cfg->mltm[index].large_out_buf, &kp_cfg->mltm[index].small_out_buf);         //перекладываем данные на передачу 
    
    if(kp_cfg->mltm[index].small_out_buf.len != 0) {
      port_write(kp_cfg->mltm[index].port, kp_cfg->mltm[index].small_out_buf.data, kp_cfg->mltm[index].small_out_buf.len);
      kp_cfg->mltm[index].flags &= ~KP_CFG_FLAGS_MASTER_NO_ACTIVITY;
    }
    kp_cfg->mltm[index].state = KM_WORK;
    kp_cfg->mltm[index].start_time = curtime;
  }else{
    //проблемы со связью
    if(curtime - kp_cfg->mltm[index].start_time > MLTM_TIME_OUT){
      //время истекло фиксируем обрыв связи
      kp_cfg->mltm[index].state = KM_INIT;
      kp_cfg->mltm[index].start_time = curtime;
      kp_cfg->mltm[index].flags |= KP_CFG_FLAGS_MASTER_NO_ACTIVITY;
    }
  }
}

//обмен данными с мастером через канал тональной частоты
void cm_master_state_mashime_tonal_com(kp_cfg_t *kp_cfg, int index, time_t curtime){
 unsigned int result; 
 unsigned char length = BUF_IN_LEN;

//читаем данные по ТЧ
 if((kp_cfg->mltm[index].flags & CHANEL_INIT) == CHANEL_INIT){
   //канал настроен
   result = cmx868a_recv_packet(kp_cfg-> mltm_buf_in, &length);
   if (result == 0 && length > 0) {
     pthread_mutex_lock(&kp_cfg->mltm[index].mutex[0]);
     //pthread_mutex_lock(&kp_cfg->mutex);
     read_iec3_data(kp_cfg-> mltm_buf_in, length, kp_cfg, &kp_cfg->mltm[index].large_out_buf);    // Дешифруем принятый пакет
     //pthread_mutex_unlock(&kp_cfg->mutex);
     pthread_mutex_unlock(&kp_cfg->mltm[index].mutex[0]);
     wrt_out_buf(&kp_cfg->mltm[index].large_out_buf, &kp_cfg->mltm[index].small_out_buf);         //перекладываем данные на передачу 
     
     if(kp_cfg->mltm[index].small_out_buf.len != 0) {
       cmx868a_send_packet(kp_cfg->mltm[index].small_out_buf.data, kp_cfg->mltm[index].small_out_buf.len);
       kp_cfg->mltm[index].flags &= ~KP_CFG_FLAGS_MASTER_NO_ACTIVITY;
     }
     kp_cfg->mltm[index].state = KM_WORK;
     kp_cfg->mltm[index].start_time = curtime;
     return;
   }
 }
 
 //канал связи не настроен или ошибка связи - считаем тики
 if(curtime - kp_cfg->mltm[index].start_time > MLTM_TIME_OUT){
   kp_cfg->mltm[index].state = KM_INIT;
   kp_cfg->mltm[index].start_time = curtime;
 }
}

void cm_master_state_mashime_work(kp_cfg_t *kp_cfg, int index){
  time_t curtime;
 
  time(&curtime);
  /*если связь есть хотябы по одному каналу флаг будет скинут
    причем для формирования флага для отдельного канала требуется отсуствие связи с мастеров в течении 10 секунд,
    после чего будет взведен соотвествующий флаг в регистре статуса в структуре конфигурации канала связи с 
    мастером сети линейной телемеханики.
    Флаг скидывается, если есть чем этому мастеру ответить, сброс флага - отсутвие активности мастера сети
    линейной телемеханики, для КП происходит в другой функции */
  if((kp_cfg->flags & KP_CFG_FLAGS_TEST_FUNC) == KP_CFG_FLAGS_TEST_FUNC){
    //включена тестовая функция
    kp_cfg->mltm[index].flags |= KP_CFG_FLAGS_MASTER_NO_ACTIVITY;
    kp_cfg->mltm[index].flags |= KP_CFG_FLAGS_NO_DATA;
    return;
  }
  
  switch(kp_cfg->mltm[index].type){
  case SERIAL_TYPE:
    cm_master_state_mashime_serial_com(kp_cfg, index, curtime);
    break;
  case TONAL_TYPE:
    cm_master_state_mashime_tonal_com(kp_cfg, index, curtime);
    break;
  }
}

//автомат работы с мастером ЛТМ
void* cm_master_state_mashime(void *params){
  chanel_thread_param_t* p = (chanel_thread_param_t*) params;
  int result;
  while(1){
    if(state_km_kltm != WORK_KLTM) {
      //нет связи к КЛТМ
      p->kp_cfg->flags |= KP_CFG_FLAGS_MASTER_NO_ACTIVITY;
      p->kp_cfg->mltm[p->index].state =  KM_INIT;
      p->kp_cfg->mltm[p->index].flags |= KP_CFG_FLAGS_MASTER_NO_ACTIVITY;
      continue;
    }
  
    switch(p->kp_cfg->mltm[p->index].state){
    case KM_INIT: 
      #ifdef DEBUG_KLTM_MAIN
      printf("INIT_KM_MASTER\n");
      #endif
      iec3_init(p->kp_cfg, &p->kp_cfg->mltm[p->index].large_out_buf, &p->kp_cfg->mltm[p->index].small_out_buf);
      if(p->kp_cfg->mltm[p->index].type ==  TONAL_TYPE){
	if(reset_tonal_modem(p->kp_cfg, p->index) == 0){
	  result = cmx868a_reset(p->kp_cfg->LenAnsFq, p->kp_cfg->BaudFq, p->kp_cfg->LevelFq); // лидер, скорость, уровень
	  if(result != 0) { 
	    printf("kltm_main: cm_master_state_mashime -> cmx868a_reset failed: %d\n", result);
	    p->kp_cfg->mltm[p->index].state = KM_INIT;
	    continue;
	  }
	}
      }
      p->kp_cfg->mltm[p->index].state = KM_WORK;
      break;
    case KM_WORK: 
      cm_master_state_mashime_work(p->kp_cfg, p->index);
      break;
    }
  }//while(1)
}
   
//автомат работы с ПЛК
unsigned int cm_kltm_state_mashine(unsigned int state_km_kltm, kltm_sCom *PLC, kltm_sCom *CM, kp_cfg_t *kp_cfg) {
  //unsigned int i;
  switch(state_km_kltm){
  case KLTM_START:
    return kltm_state_mashine_start(PLC, CM, kp_cfg);
  case READY_INIT:
    return kltm_state_mashine_ready_init(PLC, CM, kp_cfg);
  case WAIT_KVIT:
    return kltm_state_mashine_wait_kvit(PLC, CM, kp_cfg);
  case WAIT_INIT_DATA:
    return kltm_state_mashine_wait_init_data(PLC, CM, kp_cfg);
  case WORK_KLTM:
    return kltm_state_mashine_work(PLC, CM, kp_cfg);
  default: 
      return KLTM_START;
  }
}
///----------------------------------------------------------------------------
/// потоковая функция главного потока КМ-400
void *kltm_main(void *arg)
{
  int i;
  int result;
  kp_cfg_t kp_cfg;
  kltm_out_buf_t  kltm_out_buf;  //здесь храниться информация будет о командах ПЛК от мастера ЛТМ
  time_t starttime, curtime;
  struct timeval tv_old;
  struct timeval tv_cur;
  struct timezone time_zone;
  chanel_thread_param_t chanel_thread_param[MLTM_MAX];
  /*масив идентификаторов для потоков обработки 
    данных по каналамп связи с мастером линейной телемеханики*/
  pthread_t chanel_threads[MLTM_MAX];  
  memset(&PLC, 0, sizeof(PLC));
  memset(&CM , 0, sizeof(CM ));
  
  Client[kltm_client].iface = kltm_port;
  VSlave[kltm_CM].critical=3; //!!! параметр должен вводиться при запуске, через командную строку

  printf("kltm_main: THREAD STARTED\n");
  
  //PQuery[kltm_PLC].delay  = 4000; // на время тестирования замедляем опрос
  //PQuery[kltm_PLC].length = 5;    // на время тестирования максимальный размер пакета - 5 регистров
  time(&starttime);
  time_zone.tz_minuteswest = 0;
  time_zone.tz_dsttime = 0;
  /*Настройка параметров обмена */
  /****Инициазизация каналов обмена с мастером сети телемеханики**/
  kp_cfg.mltm_max = MLTM_MAX;
  kp_cfg.mltm[0].type = SERIAL_TYPE;
  kp_cfg.mltm[0].port_number = COM3;
  kp_cfg.mltm[0].mode = RS232_MODE;
  kp_cfg.mltm[0].state = KM_INIT;

  kp_cfg.mltm[1].type = TONAL_TYPE;
  kp_cfg.mltm[1].state = KM_INIT;

  pthread_mutex_init(&kp_cfg.mutex_a, NULL);
  pthread_mutex_init(&kp_cfg.mutex_b, NULL);
  for( i = 0; i <  kp_cfg.mltm_max; i++){
    chanel_thread_param[i].kp_cfg = &kp_cfg; 
    chanel_thread_param[i].index = i;
    pthread_mutex_init(&kp_cfg.mltm[i].mutex[0], NULL); //инициализирем семафоры
    pthread_mutex_init(&kp_cfg.mltm[i].mutex[1], NULL);
    switch(kp_cfg.mltm[i].type){
    case TONAL_TYPE:
      result = cmx868a_init();
      if( result != 0){
	printf("kltm_main:set_kp_opts -> cmx868a_init() error %d", result);
      }else{
	kp_cfg.mltm[i].flags |= CHANEL_INIT;
      }
      break;
    case SERIAL_TYPE:
    default:
      kp_cfg.mltm[i].port = open_port(kp_cfg.mltm[i].port_number);
      if(kp_cfg.mltm[i].port < 0 ){
	printf("Ошибка открытия порта: ttyM%d\n", kp_cfg.mltm[i].port_number);
	return 0;
      }
      break;
    }
    
  }

  /*******************************************/
  /*Привязывем выходной буфер с командами мастера ЛТМ для ПЛК к конфигурации КП
   пусть пока будет так, позднее можно будет включить в структуру не ссылку а переменную*/
  kp_cfg.kltm_out_buf = &kltm_out_buf;
  kp_cfg.kltm_max_pack_size = 50;
  state_km_kltm = KLTM_START;

  //потоки потоки.... наш девиз -- каждому каналу связи с мастером по потоку!!!
  pthread_create(&chanel_threads[0], NULL, &cm_master_state_mashime, &chanel_thread_param[0]);
  pthread_create(&chanel_threads[0], NULL, &cm_master_state_mashime, &chanel_thread_param[1]);

  
  while (1) {
    //время в контроллере
    gettimeofday(&tv_cur, &time_zone);
    result = (tv_cur.tv_usec - tv_old.tv_usec)/10000;
    if(result < 0){
      kp_cfg.time.ti -= result;
    }
    else{
      kp_cfg.time.ti += result;
    } 
    tv_old.tv_usec = tv_cur.tv_usec;

    time(&curtime);
      if(kltm_getPLC(&PLC)==0) { 
	state_km_kltm = cm_kltm_state_mashine(state_km_kltm, &PLC, &CM, &kp_cfg);
	starttime=curtime;
	
	if(MULTIPLI_CM_WRT != 0){
	  kltm_debug_CM(&CM);
	  kltm_setCM(&CM);
	}
	
	kp_cfg.flags |= KP_CFG_FLAGS_MASTER_NO_ACTIVITY;
	//вычислем есть ли активность мастеров на линии
	for( i = 0; i <  kp_cfg.mltm_max; i++){
	  kp_cfg.flags &= ~(KP_CFG_FLAGS_MASTER_NO_ACTIVITY & ~kp_cfg.mltm[i].flags);
	}
      } else {
	#ifdef DEBUG_KLTM_MAIN
	printf("kltm_main: connection to KLTM broken\n");
	#endif
	if(curtime-starttime > KLTM_TIME_OUT) {
	  state_km_kltm = KLTM_START;
	  starttime=curtime;
	}
      }
  } // while(1)
  
  // THREAD STOPPED
  // sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|GATEWAY_MOXAGATE, 43, 0, 0, 0, 0);
  pthread_exit (0);	
}
///----------------------------------------------------------------------------
