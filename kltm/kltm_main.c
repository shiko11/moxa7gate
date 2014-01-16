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

kltm_sCom PLC, CM;

///=== KLTM_H private functions

///----------------------------------------------------------------------------
/// потоковая функция главного потока КМ-400
void *kltm_main(void *arg)
  {
  int i, j;
  int  result;
  int port;
  char BUF_IN[BUF_IN_LEN];
  kp_cfg_t kp_cfg;
  port_options_t port_options;
  time_t starttime, curtime;

  memset(&PLC, 0, sizeof(PLC));
  memset(&CM , 0, sizeof(CM ));

  Client[kltm_client].iface = kltm_port;
  VSlave[kltm_CM].critical=3; //!!! параметр должен вводиться при запуске, через командную строку

  printf("kltm_main: THREAD STARTED\n");

  PQuery[kltm_PLC].delay  = 4000; // на время тестирования замедляем опрос
  PQuery[kltm_PLC].length = 5;    // на время тестирования максимальный размер пакета - 5 регистров
  time(&starttime);

  init_modem(); // инициализация модема

  /****Инициазизация канала обмена с мастером*******
  kp_cfg.addr = 5; //задаем адресс КП
  kp_cfg.state &= ~KP_CFG_ACK_MASK;
  kp_cfg.state |= KP_CFG_SHORT_ACK; //будем использовать короткое потверждение

  port = open_port(COM3);
  if(port < 0 ){
    printf("Ошибка открытия порта: ttyM%d\n", COM3);
    return 0;
  }
  //задаем настройки опций порта
  port_options.baudrate = 115200;
  port_options.party = EVEN;
  port_options.mode = RS232_MODE; 
  port_options.flow_control = NONE;

  if(set_port_options(port, &port_options) < 0){
    printf("Ошибка при настройке параметров порта вот такая:  %d\n", errno);
    return 0;
    }
  
  iec3_init(&kp_cfg);
  *******************************************/
  j=0;
  set_rts(0); // изменить значение сигнала RTS порта модема
  while (1) {
  	
    time(&curtime);
    
    if(curtime-starttime > 10) { // условие выполняется каждые 10 секунд
      starttime=curtime;
      printf("kltm_main: tick #%d\n", j+1);
      set_rts(0); // изменить значение сигнала RTS порта модема
      
      j++;
      if(j==2)  { strcpy(BUF_IN, "ATZ\r"                  ); send_AT(BUF_IN); }
      if(j==4)  { strcpy(BUF_IN, "ATFY0B6C2R2S10=255@L1\r"); send_AT(BUF_IN); }
      if(j==6)  { strcpy(BUF_IN, "ATA\r"                  ); send_AT(BUF_IN); }
      if(j==8)  { strcpy(BUF_IN, "abcdefghijklmnopqrstuvwxuz"); send_AT(BUF_IN); }
      if(j==10) { strcpy(BUF_IN, "+++"                    ); send_AT(BUF_IN); }
      if(j==12) { strcpy(BUF_IN, "ATO\r"                  ); send_AT(BUF_IN); }
      if(j==14) { strcpy(BUF_IN, "abcdefghijklmnopqrstuvwxuz"); send_AT(BUF_IN); }
      if(j==16) { strcpy(BUF_IN, "+++"                    ); send_AT(BUF_IN); }
      if(j==18) { strcpy(BUF_IN, "ATO\r"                  ); send_AT(BUF_IN); }
      if(j%2==1) get_modem_data(BUF_IN);

      if(kltm_getPLC(&PLC)==0) { // обновляем значение переменной PLC
    	
    	if(PLC.T_PAC_CODE != CM.T_PAC_CODE) { // пришел новый запрос
    		
	  CM.R_PAC_NUM= PLC.R_PAC_NUM;  // номер пакета принятого
	  CM.T_PAC_NUM= PLC.T_PAC_NUM;  // номер пакета отправленного
	  CM.T_PAC_LEN= PLC.T_PAC_LEN;  // актуальная длина блока данных в поле T_DATA
	  CM.T_PAC_CODE=PLC.T_PAC_CODE; // код пакета (операции)
	  
	  for(i=0; i<CM.T_PAC_LEN; i++) CM.T_DATA[i] = PLC.T_DATA[i]; // данные пакета
        
	  // отправка ответа
	  kltm_setCM(&CM);
	}
	
      } else {printf("kltm_main: connection to KLTM broken\n");} // нет связи с КЛТМ
    }
  
 //##################################################################################

/*    
    result = port_read(port, BUF_IN, BUF_IN_LEN);
    if(result !=0){
      printf("Принято: %s \n", BUF_IN);
      printf("Принято в хексе: ");
      for(i = 0; i < result; i++){
	printf("%X ", BUF_IN[i]);
      }
      printf("\n");
      
      //Тут тестовые вызовы размещены были мной
      read_iec_data(BUF_IN, result, &kp_cfg);
      if((kp_cfg.state & KP_CFG_GENERAL_POLL) == KP_CFG_GENERAL_POLL){
	//пришла команда общего опроса КП
	kp_cfg.state &= ~KP_CFG_GENERAL_POLL; //сбрасывается флаг общего опроса
	iec3_send_data(&kp_cfg);              //Запихиваем в буфер текущее данные
	//if(plc_data_len != 0) iec3_convert_plc2kltm(plc_data, plc_data_len,  &kp_cfg);
	//else iec3_send_data(&kp_cfg);
      } 

      if((kp_cfg.state & SIGNAL_POLL) == SIGNAL_POLL){
	//пришла команда опроса какого-то сигнала
	kp_cfg.state &= ~SIGNAL_POLL;          //сбрасывается флаг общего опроса
	iec3_send_data(&kp_cfg);              //Запихиваем в буфер текущее данные
	//if(plc_data_len != 0) iec3_convert_plc2kltm(plc_data, plc_data_len,  &kp_cfg);
	//else iec3_send_data(&kp_cfg);
      }

      if(iec3_out_buf.len != 0) port_write(port, iec3_out_buf.data, iec3_out_buf.len);
      
      if((kp_cfg.state & KP_CFG_INIT) == KP_CFG_INIT){
	//КП проинициализировано мастером
      }
    }*/
  } // while(1)
	
  // THREAD STOPPED
  // sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|GATEWAY_MOXAGATE, 43, 0, 0, 0, 0);
  pthread_exit (0);	
}
///----------------------------------------------------------------------------
