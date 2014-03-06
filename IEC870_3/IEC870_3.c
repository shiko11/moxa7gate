/************************************************
ОАО НПО Спецэлетромеханика Брянск 2014
Модуль с функциями для работы по протоколу 
Элеси МЭК-3
 ************************************************/
#include "IEC870_3.h"

/*Разбор принятого пакета*/
int read_iec3_data(char *in_buf, int in_len, kp_cfg_t *kp_cfg, iec3_buf_data_send_t *iec3_buf){
  int i = 0;
  int pack_len = 0;
  int result = 0;
  while(i<in_len){
    switch(in_buf[i]){
    case VAR_LEN_FRAME_TYPE:
      //разбор кадра переменной длинны
      result = analize_var_frame(&in_buf[i], in_len - i, kp_cfg, &pack_len, iec3_buf);
      if(!pack_len) i += pack_len; //длина не нулевая продолжаем обработку 
      else {
	return  result; //ошибка длины пакета выходим нах дальше понять, что в буфере невозможно
      }
      break;
    case CONST_LEN_FRAME_TYPE:
      //разбор кадра постоянной длинны
      result = analize_const_frame(&in_buf[i], in_len - i, kp_cfg, iec3_buf);
       i += CONSTANT_FRAME_LEN;
       break;
    case  ACK_POSITIVE_FRAME_TYPE:
      //пришло положительное подтверждение
      return 0; //заглушка 
      result =  0;
       break;
    case ACK_NEGATIVE_FRAME_TYPE:
      //пришло отрицательное подтверждение
      return 0;  //заглушка
      result = 0;
      break;
    default:
      //непонятный тип пакета прекращаем обработку
       return  ERROR_UNDEFINED_FRAME_TYPE;
       break;
    }
  }
  kp_cfg->MasterPackCnt++;
  return  result;
}


/*Разбор пакета постоянной длинны*/
int  analize_const_frame(char *in_buf, int in_len, kp_cfg_t *kp_cfg, iec3_buf_data_send_t *iec3_buf){
  int code;
  #ifdef DEBUG
  printf("Адрес опрашиваемого КП: %d\n",in_buf[KP_ADDR_CFL] ); 
  #endif
  if(in_buf[SERVICE_CODE_CFL] && PRM_MASK != PRM_MASK){
    #ifdef DEBUG
    printf("Этот запрос пришол не от контролирующей станции игнорируем его\n");
    #endif
    return 0;
  }
  //Пакет от первичной станции
  #ifdef DEBUG
  printf("Пакет пришол от базовой станции\n"); 
  #endif
      
  if(in_buf[KP_ADDR_CFL] != kp_cfg->addr && in_buf[KP_ADDR_CFL] != kp_ALL){
    //пакет адресован не нам
    #ifdef DEBUG
    printf("Этот запрос пришол не нам\n");
    #endif
    return 0;
  }
  /*адрес КП совпадает с текущем адресом КП или направлен на все КП*/
  
  if(in_buf[END_OF_CFL] !=  END_OF_FRAME){
    //отсуствует символ конца постоянного пакета
    #ifdef DEBUG
    printf("Отсутствует конец пакета\n");
    #endif
    return ERROR_CFL_END;
  }
  /*есть символ конца пакета*/

  if(in_buf[CHECKSUM_CLF] != (in_buf[SERVICE_CODE_CFL] + in_buf[KP_ADDR_CFL])){
    //Ошибка контрольной суммы
    #ifdef DEBUG
    printf("Ошибка контрольной суммы\n");
    #endif
    return ERROR_CRC_CFL;
  }
  /*С контрольной суммой все ОК*/

 //анализируем контрольное поле
 return analize_control_feld(in_buf[SERVICE_CODE_CFL], kp_cfg, iec3_buf, &code);
}

/*Функция заталкивающая в выходной буфер подтверждающий кадр*/
void send_ACK(kp_cfg_t *kp_cfg, iec3_buf_data_send_t *iec3_buf){ 
  unsigned char temp_strig[5];
  
  if((kp_cfg->state & KP_CFG_ACK_MASK)== KP_CFG_SHORT_ACK){
    //короткий ответ
    
    #ifdef DEBUG
    printf("Используется короткий тип пакета для потверждения\n ");
    #endif
  
    temp_strig[START_BYTE_CFL] = ACK_POSITIVE_FRAME_TYPE;
    iec3_buf_wrt(temp_strig, SHORT_ACK_LEN, iec3_buf);
    
  }else{
    //длинный ответ
    
    #ifdef DEBUG
    printf("Используется длинный тип пакета для потверждения\n ");
    #endif

    temp_strig[START_BYTE_CFL] = CONST_LEN_FRAME_TYPE;
    temp_strig[SERVICE_CODE_CFL] = SERVICE_CODE_ACK;// | ( kp_cfg->FCB*FCB_MASk);
    temp_strig[KP_ADDR_CFL] = kp_cfg->addr;
    temp_strig[CHECKSUM_CLF] = kp_cfg->addr + SERVICE_CODE_ACK;
    temp_strig[END_OF_CFL] = END_OF_FRAME;
    iec3_buf_wrt(temp_strig, CONSTANT_FRAME_LEN, iec3_buf);
  }
}

/*Функция заталкивающая в выходной буфер отрицательного потверждения*/
void send_NACK(kp_cfg_t *kp_cfg, iec3_buf_data_send_t *iec3_buf){ 
  unsigned char temp_strig[5];
  if((kp_cfg->state & KP_CFG_ACK_MASK)== KP_CFG_SHORT_ACK){
    //короткий ответ
   
    #ifdef DEBUG
    printf("Используется короткий тип пакета для отрицательного потверждения\n ");
    #endif
 
    temp_strig[START_BYTE_CFL] = ACK_NEGATIVE_FRAME_TYPE;
    iec3_buf_wrt(temp_strig, SHORT_ACK_LEN, iec3_buf);
  }else{
    //длинный ответ
    
    #ifdef DEBUG
    printf("Используется длинный тип пакета для потверждения\n ");
    #endif
    temp_strig[START_BYTE_CFL] = CONST_LEN_FRAME_TYPE;
    temp_strig[SERVICE_CODE_CFL] = SERVICE_CODE_NACK;// | ( kp_cfg->FCB*FCB_MASk);
    temp_strig[KP_ADDR_CFL] = kp_cfg->addr;
    temp_strig[CHECKSUM_CLF] = kp_cfg->addr + SERVICE_CODE_ACK;
    temp_strig[END_OF_CFL] = END_OF_FRAME;
    iec3_buf_wrt(temp_strig, CONSTANT_FRAME_LEN, iec3_buf);
  }
}


/*Функция заталкивающая в выходной буфер кадр с информацией об обсутсвии данных*/
void send_no_data(kp_cfg_t *kp_cfg, iec3_buf_data_send_t *iec3_buf){ 
  unsigned char temp_strig[5];
  if((kp_cfg->state & KP_CFG_ACK_MASK)== KP_CFG_SHORT_ACK){
    //короткий ответ
    
    #ifdef DEBUG
    printf("Используется короткий тип пакета отрицательного ответа на запрос данных\n ");
    #endif
    temp_strig[START_BYTE_CFL] = ACK_NEGATIVE_FRAME_TYPE;
    iec3_buf_wrt(temp_strig, SHORT_ACK_LEN, iec3_buf);
  }else{
    //длинный ответ
    
    #ifdef DEBUG
    printf("Используется длинный тип пакета для отправки сообщения об отсуствии данных\n ");
    #endif
    temp_strig[START_BYTE_CFL] = CONST_LEN_FRAME_TYPE;
    temp_strig[SERVICE_CODE_CFL] = SERVICE_CODE_NO_DATA;// | ( kp_cfg->FCB*FCB_MASk);
    temp_strig[KP_ADDR_CFL] = kp_cfg->addr;
    temp_strig[CHECKSUM_CLF] = kp_cfg->addr + SERVICE_CODE_ACK;
    temp_strig[END_OF_CFL] = END_OF_FRAME;
    iec3_buf_wrt(temp_strig, CONSTANT_FRAME_LEN, iec3_buf);
  }
}

/*функция разбора пакета переменной длинны*/
int  analize_var_frame(char *in_buf, int in_len, kp_cfg_t *kp_cfg, int *pack_len, iec3_buf_data_send_t *iec3_buf){
  //Сначала необходимо убедиться в корректности принятых данных и в том, что эти данные адресованны нам
  //рассчет длины пакета общей
  int pl, i, code, result;
  unsigned char CRC;
  CRC = 0;
  *pack_len = 0;  
  if( in_buf[KP_ADDR_VAR] != kp_cfg->addr && in_buf[KP_ADDR_VAR] != kp_ALL){
    //Пакет адресован не нам, не будем его обрабатывать
    #ifdef DEBUG
    printf("Данные адресованы не этому КП\n ");
    #endif
    return 0;
  }
  //Пакет адресован нам или сразу всем КП
  #ifdef DEBUG
  printf("Данные адресованы этому КП или всем КП\n ");
  #endif
  
  if(in_buf[SIZE_LOW_VAR] !=  in_buf[SIZE_HIGH_VAR]){
    /*Старший и младший байт поля длина пакета не дублируют дргу друга!!!*/
    //У пакета не совпадают поля размер пакета -  ошибка ахтунг!
      #ifdef DEBUG
      printf("Ошибка не сопадают поля размер пакета!!!\n ");
      #endif
      return  ERROR_LEN_VAR;
  }
  //с размером пакета все ОК
  pl= in_buf[SIZE_LOW_VAR] + ADD_SIZE_VAR;
  *pack_len = pl;
  #ifdef DEBUG
  printf("Поля размер пакета переменной длины совпадают\n");
  printf("Общая длина пакета перменной длинны составляет %d\n", pl);
  #endif
  
  if(in_buf[START_BYTE2_VAR] !=  VAR_LEN_FRAME_TYPE){
    #ifdef DEBUG
    printf("Неправильное значение второго стартового поля в пакете перменной длины\n");
    #endif
    return ERROR_BAD_SECOND_START_FEILD;
  }
  
  //Опа и второе стартовое поле есть
  #ifdef DEBUG
  printf("Со вторым котрольным полем пакета перменной длины все впорядке\n");
  #endif
  
  if(in_buf[pl - 1] !=  END_OF_FRAME){
    //Нет символа - конца пакета
    #ifdef DEBUG
    printf("Неправильный символ конца пакета у пакета переменной длины\n");
    #endif
    return ERROR_VAR_END;
  }
  //имеется верный символ завершающий пакет
  #ifdef DEBUG
  printf("С симолом конца пакета пакета переменной длины все впорядке\n");
  #endif
  
  //расчитываем контрольную сумму
  for(i = SERVICE_CODE_VAR; i < pl - SUB_CRC_VAR; i ++){
    CRC += (unsigned char)in_buf[i];
  }
  #ifdef DEBUG
  printf("Рассчитали следующую контрольную сумму 0x%x\n", CRC);
  printf("Полученная контрольная сумма 0x%x\n", in_buf[pl - SUB_CRC_VAR]);
  #endif
  if(in_buf[pl - SUB_CRC_VAR] != CRC){
    //ошибка контрольной суммы
    #ifdef DEBUG
    printf("Неравильная контрольная сумма у пакета перменной длины\n");
    #endif
    return ERROR_CRC_VAR;
  }
  //контрольная сумма совпала
  #ifdef DEBUG
  printf("Пакет переменной длины: контрольная сумма совпала\n");
  #endif
  
  //проверки кончились можно разбирать управляющее поле
  result =  analize_control_feld(in_buf[SERVICE_CODE_VAR], kp_cfg, iec3_buf,  &code);
  if(result != 0) return result;// ошибочка вышла
  
  //анализируем код функции
  switch(in_buf[COMMAND_FELD]){
  case  GENERAL_POLL:
    //пришла команда обшего опроса
    #ifdef DEBUG
    printf("Пришла команда общего опроса\n");
    #endif
    send_ACK(kp_cfg, iec3_buf); //шлем подтверждение принятие команды
    kltm_wrt_common_poll_cmd(kp_cfg->kltm_out_buf);
    kp_cfg->flags |= KP_CFG_FLAGS_MASTER_QUERY;
    //kp_cfg->state |= KP_CFG_GENERAL_POLL; //запоминамем, что приняли команду общего опроса 
    return 0;
  case SIGNAL_POLL:
    //пришол опрос конкретного сигнала
    #ifdef DEBUG
    printf("Пришола команда опроса конкретного сигнала\n");
    #endif
    send_ACK(kp_cfg, iec3_buf); //шлем подтверждение принятие команды
    kp_cfg->flags |= KP_CFG_FLAGS_MASTER_QUERY;
    kltm_wrt_sign_poll_cmd(kp_cfg, in_buf[ADDR_LOW_BYTE], in_buf[ADDR_HIGT_BYTE]);
    //kp_cfg->state |= SIGNAL_POLL;
    return 0;
  case TELECONTROL:
    //пришола команда телеуправления
    #ifdef DEBUG
    printf("Пришла команда телеуправления\n");
    #endif
    send_ACK(kp_cfg, iec3_buf); //шлем подтверждение принятие команды
    kp_cfg->flags |= KP_CFG_FLAGS_MASTER_QUERY;
    kltm_wrt_tu_tr_cmd(kp_cfg->kltm_out_buf, in_buf[ADDR_LOW_BYTE], in_buf[ADDR_HIGT_BYTE], &in_buf[DATA_BEGIN_VAR],in_buf[SIZE_LOW_VAR]);
    return 0;
  default: 
    #ifdef DEBUG
    printf("неизвестная команда для пакета переменной длины\n");
    #endif
    return ERROR_UNKNOWN_COMMAND;
  }
} 

//запись в буфре на передачу команд ПЛК команды общего опроса
void kltm_wrt_common_poll_cmd(kltm_out_buf_t *kltm_out_buf){
  unsigned char data[2];
  data[0] = GENERAL_POLL_LEN;
  data[1] = GENERAL_POLL;
  kltm_buf_wrt(data, GENERAL_POLL_LEN + PLC_SIZE_LEN, kltm_out_buf);
}

//запись в буфре на передачу команд ПЛК команды телеуправления или телерегулирования
void kltm_wrt_tu_tr_cmd(kltm_out_buf_t *kltm_out_buf, unsigned char addr_low, unsigned char addr_higt, unsigned char *data_pos, unsigned char pack_len){
  unsigned int i;
  //unsigned int CRC = 0;
  unsigned char len;
  unsigned char data[9];
  switch(addr_higt & PLC_SIG_TYPE_H){
  case PLC_SIG_TU_H:
    len = PLC_TU_LEN;
    break;
  case PLC_SIG_TR1_H:
    len = PLC_TR1_LEN;
    break;
  case PLC_SIG_TR2_H:
    len = PLC_TR2_LEN;
    break;
  case PLC_SIG_TR4_H:
    len = PLC_TR4_LEN;
    break;
  default:
    /*неполучилось определить длину типа принятых данных по адресу
     пробуем определить по длине пакета принятого от мастера*/
    len = pack_len - TELECONTROL_SUB;
    break;
  }
  data[0] = TELECONTROL_LEN + len;
  data[1] = TELECONTROL;
  data[2] = addr_low;
  data[3] = addr_higt & PLC_SIG_TYPE_H;
  
  //копируем данные и контрольную сумму
  for(i = 0; i < len + TELECONTROL_CRC_LEN; i++){
    data[4 + i] = *(data_pos + i);
  }
  
  /*for(i = 0; i < len + TELECONTROL_ADD; i++){
    CRC += data[i + 1];
  }
  CRC = ~CRC;
  data[4 + len] = (unsigned char) (CRC & 0xFF);
  data[5 + len] = (unsigned char) (8>>(CRC & 0xFF00));*/
  kltm_buf_wrt(data, len + TELECONTROL_ADD + TELECONTROL_CRC_LEN +  PLC_SIZE_LEN, kltm_out_buf);
}

//запись в буфре на передачу команд ПЛК команды опроса сигнала
void kltm_wrt_sign_poll_cmd(kp_cfg_t *kp_cfg, unsigned char addr_low, unsigned char addr_higt){
  unsigned char data[3];
  unsigned int last;
  unsigned int i;
  data[0] = SIGNAL_POLL_LEN;
  data[1] = SIGNAL_POLL;
  data[2] = addr_low;
  data[3] = addr_higt & PLC_SIG_TYPE_H;
  kltm_buf_wrt(data, SIGNAL_POLL_LEN + PLC_SIZE_LEN, kp_cfg->kltm_out_buf);
  //пихаем адрес запрощенного сигнала в специальный буфер
  //kp_cfg->asked_data[kp_cfg->asked_data_num] = addr_low | ((addr_higt & PLC_SIG_TYPE_H)<<8);
  //kp_cfg->asked_data_num++;
  last = kp_cfg->asked_data.last;
  for(i = 0; i < ASKED_DATA_MAX; i++){
    if(kp_cfg->asked_data.data[i].use == 0){
      //есть свободный элемент
      kp_cfg->asked_data.data[last].next = i;
      kp_cfg->asked_data.data[i].prev =  last;
      kp_cfg->asked_data.data[i].data = addr_low | ((addr_higt & PLC_SIG_TYPE_H)<<8);
      kp_cfg->asked_data.data[i].use = 1;
      kp_cfg->asked_data.asked_data_num++;
      kp_cfg->asked_data.last = i;
      break;
    }
  }
}

//функция разбора поля управления
int  analize_control_feld(char control_feld, kp_cfg_t *kp_cfg, iec3_buf_data_send_t *iec3_buf, int *code){
  if((control_feld & FCV_MASK) == FCV_MASK){
    //У каждого нового пакета от Мастера бит FCB инвертируется
    #ifdef DEBUG
    printf("Используется инвертирование бита FCB при передаче пакетовм от мастера\n ");
    printf("FCB = %d \n",  (kp_cfg->state & KP_CFG_FCB_MASK) == KP_CFG_FCB_MASK);
    #endif
    if(((control_feld & FCB_MASK) == FCB_MASK) ^  ((kp_cfg->state & KP_CFG_FCB_MASK) == KP_CFG_FCB_MASK)){
      /*нас услышали, следовательно данные были успешно переданы, можно смело удалять
	их из некого буфера передачи. где храним то, что требуется отослать*/
      #ifdef DEBUG
      printf("Прошлый ответ был принят мастером\n");
      #endif
    }else{
      /*проверка инверсии бита, если инверсии не было, то следовательно, если
	передовались данные нас не услышали, требуется повторить передачу еще раз!*/		
      #ifdef DEBUG
      printf("Прошлый ответ не был принят мастером требуется повтор передачи\n");
      #endif
    } 
    kp_cfg->state &= ~KP_CFG_FCB_MASK;
    if((control_feld & FCB_MASK) == FCB_MASK)  kp_cfg->state |= KP_CFG_FCB_MASK;
  }else{
    //инвертирование бита FCB не используется
    #ifdef DEBUG
    kp_cfg->state &= ~KP_CFG_FCB_MASK;
    if((control_feld & FCB_MASK) == FCB_MASK)  kp_cfg->state |= KP_CFG_FCB_MASK;;
    printf("Не используется инвертирование бита FCB при передаче пакетов от мастера\n "); 
    printf("FCB = %d \n",  (kp_cfg->state & KP_CFG_FCB_MASK) == KP_CFG_FCB_MASK);
    #endif
  }
  
  switch(control_feld & SERVICE_CODE_MASK){
    //определяем, что за сервисная функция получена
  case SERVICE_CODE_RESET_CONECTION:
    //сброс удаленной связи
    #ifdef DEBUG
    printf("Сервисная функция: сброс удаленной связи\n "); 
    #endif
    clear_send_buf(); 
    kp_cfg->state |= KP_CFG_FCB_MASK;
    send_ACK(kp_cfg, iec3_buf);
    kp_cfg->state |= KP_CFG_INIT;
    kp_cfg->flags &= ~KP_CFG_FLAGS_MASTER_NO_ACTIVITY;
    return 0;
    break;//май индусиш стайл :)
  case SERVICE_CODE_RESER_PROCES:
    //сброс пользовательского процесса
    #ifdef DEBUG
    printf("Сервисная функция: сброс пользовательского процесса\n ");
    #endif
    clear_send_buf(iec3_buf);
    kp_cfg->state &= ~KP_CFG_FCB_MASK;
    send_ACK(kp_cfg, iec3_buf);
    return 0;    
    break;
  case SERVICE_CODE_TS:
    //передача с подтверждением
    #ifdef DEBUG
    printf("Сервисная функция: передача с подтверждением\n "); 
    #endif
    return 0;
    break;
  case SERVICE_CODE_TS_NO_ACK:
    //передача без подтверждения
    #ifdef DEBUG
    printf("Сервисная функция: передача без подтверждения\n "); 
    #endif
    return 0;
    break;
  case SERVICE_CODE_ACD_NEED:
    //ожидается ответ с интересующим битом ACD
    #ifdef DEBUG
    printf("Сервисная функция: ожидается ответ с интересующим битом ACD\n "); 
    #endif
    return 0;
    break;
  case SERVICE_CODE_STATE_CONECTION:
    //запроса статуса связи
    #ifdef DEBUG
    printf("Сервисная функция: запроса статуса связи\n "); 
    #endif
    //формируем в буфере на передачу ответ на запрос статуса связи
    write_state_connection(kp_cfg, iec3_buf);
    return 0;
    break;
  case SERVICE_CODE_DATA1_NEED:
    //запрос данных первой категории
    #ifdef DEBUG
    printf("Сервисная функция: запрос данных первой категории\n "); 
    #endif
    if(iec3_buf->num == 0) send_no_data(kp_cfg, iec3_buf);
    return 0;
    break;
  case SERVICE_CODE_DATA2_NEED:
    //запрос данных второй категории
    #ifdef DEBUG
    printf("Сервисная функция: запрос данных второй категории\n "); 
    #endif
    send_no_data(kp_cfg, iec3_buf);
    return 0;
    break;
  default:
    #ifdef DEBUG
    printf("Ошибка сервисной функции: функция %d не найдена\n ", control_feld & SERVICE_CODE_MASK); 
    #endif
    return ERROR_BAD_SERVICE_CODE;
    break;
  }
}

/*Функция "очишающая" буфер на передачу*/
void clear_send_buf(iec3_buf_data_send_t *iec3_buf){
  iec3_buf->begin = 0;
  iec3_buf->end = 0;
  iec3_buf->len[FIST_PACK] = 0;
  iec3_buf->num = 0;
}

/*Функция записывающая в буфер ответ со статусом связи*/
void write_state_connection(kp_cfg_t *kp_cfg, iec3_buf_data_send_t *iec3_buf){
  unsigned char temp_strig[5];
  temp_strig[START_BYTE_CFL] = CONST_LEN_FRAME_TYPE;
  temp_strig[SERVICE_CODE_CFL] = SERVICE_CODE_CON_STAT;
  temp_strig[KP_ADDR_CFL] = kp_cfg->addr;
  temp_strig[CHECKSUM_CLF] = kp_cfg->addr + SERVICE_CODE_CON_STAT;
  temp_strig[END_OF_CFL] = END_OF_FRAME;
  iec3_buf_wrt(temp_strig, CONSTANT_FRAME_LEN, iec3_buf);
}

/*Функция перекладывающая из большого буфера данных на передачу в малый*/
void wrt_out_buf(iec3_buf_data_send_t *iec3_buf, iec3_buf_out_t *iec3_out_buf){
  unsigned i, j;
  unsigned int pack_len;
  #ifdef DEBUG
  printf("3: Начало кольцевого буфера длины пакетов: %d , значение в буфере: %x\n", iec3_buf->len_begin, iec3_buf->len[iec3_buf->len_begin]);
  printf("4: Конец  кольцевого буфера длины пакетов: %d , значение в буфере: %x\n", iec3_buf->len_end, iec3_buf->len[iec3_buf->len_end]);
  printf("5: Количество пакетов в большом буфере: %d \n", iec3_buf->num);
  #endif
  if(iec3_buf->num !=0 ){
    pack_len = iec3_buf->len[iec3_buf->len_begin];
    iec3_out_buf->len = pack_len;
    for(i = 0, j = iec3_buf->begin; i < pack_len; i++, j++){
      if(j  ==  MAX_IEC_OUT_ALL_BUF_LEN) {
	j = 0;
      }
      iec3_out_buf->data[i] = iec3_buf->data[j];
    }
    iec3_buf->begin = j;
    iec3_buf->len_begin++;
    if(iec3_buf->len_begin == MAX_IEC_OUT_ALL_BUF_LEN) iec3_buf->len_begin = 0;
    iec3_buf->num--;  //Уменьшаем количество пакетов буфере на 1  и размер буфера
    
  }else iec3_out_buf->len = 0;
  #ifdef DEBUG
  if(iec3_out_buf->len > 6){
    //printf("6: Начало кольцевого буфера длины пакетов: %d , значение в буфере: %x\n", iec3_buf->len_begin, iec3_buf->len[iec3_buf->len_begin]);
    //printf("7: Конец  кольцевого буфера длины пакетов: %d , значение в буфере: %x\n", iec3_buf->len_end, iec3_buf->len[iec3_buf->len_end]);
    printf("8: Записано в буфер на передачу сообщение длиной %d: ", iec3_out_buf->len);
    for(i = 0; i < iec3_out_buf->len; i++) printf("%x ",iec3_out_buf->data[i]);
    printf("\n");
  }
  #endif
}


void iec3_init(kp_cfg_t *kp_cfg, iec3_buf_data_send_t *iec3_buf, iec3_buf_out_t *iec3_out_buf){
  unsigned int i;
  kp_cfg->flags = 0;
  kp_cfg->MasterPackCnt = 0;
  //kp_cfg->asked_data_num;
  kp_cfg->asked_data.asked_data_num = 0;
  kp_cfg->asked_data.first = 0;
  kp_cfg->asked_data.last = 0;
  for(i = 0; i < ASKED_DATA_MAX; i++){
    kp_cfg->asked_data.data[i].use = 0;
  }
  iec3_buf->num = 0;
  iec3_buf->begin = 0;
  iec3_buf->end = 0;
  iec3_buf->len_begin = 0;
  iec3_buf->len_end = 0;

  for(i = 0; i < MAX_IEC_OUT_ALL_BUF_LEN; i ++){
    iec3_buf->data[i] = 0;
    iec3_buf->len[i] =0;
  }
  iec3_out_buf->len = 0;
  for(i = 0; i < MAX_IEC_OUT_BUF_LEN; i ++) iec3_out_buf->data[i] = 0;
  kp_cfg->kltm_out_buf->num =0;
  kp_cfg->kltm_out_buf->begin = 0;
  kp_cfg->kltm_out_buf->end = 0;
}

void kltm_buf_wrt(unsigned char *data, unsigned int len, kltm_out_buf_t *kltm_out_buf){
  unsigned int i, j;

  for(i = 0, j = kltm_out_buf->end; i < len; i++, j++){
    if(j  ==  MAX_KLTM_OUT_BUF_LEN) {
      j = 0;
    }
    kltm_out_buf->data[j] = data[i];
  } 
  kltm_out_buf->end = j;  
  kltm_out_buf->num++;
}

//вычитываем из буфера требуемое количество элементов не превышающее установленный размер
unsigned char kltm_buf_get_data(unsigned char *dest, unsigned int max_size, kltm_out_buf_t *kltm_out_buf){
  unsigned int i;
  unsigned int j;
  unsigned int p_size = 0; //полный размер пакета
  unsigned int cmd_size;   //размер одной каманды из пакета
  
  if(kltm_out_buf->num !=0 ){
    do{
      cmd_size = kltm_out_buf->data[kltm_out_buf->begin] + PLC_SIZE_LEN;
      p_size += cmd_size;
      if(p_size >= max_size){
	return p_size - cmd_size;
      }
      
      for(i = 0, j = kltm_out_buf->begin; i < cmd_size; i++, j++){
	if(j  ==  MAX_KLTM_OUT_BUF_LEN) {
	  j = 0;
	}
	dest[i] = kltm_out_buf->data[j];
      }
      kltm_out_buf->begin = j;
      kltm_out_buf->num--;  //Уменьшаем количество пакетов буфере на 1  и размер буфера
    } while(kltm_out_buf->num !=0);
    return p_size;
  }else {
    return 0;
  }
}


/*Функция записи в большой буфер передачи*/
void iec3_buf_wrt(unsigned char *data, unsigned int len, iec3_buf_data_send_t *iec3_buf){
  unsigned int i, j;
  iec3_buf->len[iec3_buf->len_end] = len;
  iec3_buf->len_end++;
  if(iec3_buf->len_end == MAX_IEC_OUT_ALL_BUF_LEN) iec3_buf->len_end = 0;
  for(i = 0, j = iec3_buf->end; i < len; i++, j++){
    if(j  ==  MAX_IEC_OUT_ALL_BUF_LEN) {
      j = 0;
    }
    iec3_buf->data[j] = data[i];
  } 
  iec3_buf->end = j;  
  iec3_buf->num++;
}



//определяем длину данных в пакете на передачу сначала идет длина а затем блок данных длиной равной длине(тафталогия мать ее)
unsigned char get_iec3_sig_len_in_pack(unsigned char addr_higt, unsigned char len){
  if((addr_higt & PLC_SIG_TIME_H) == PLC_SIG_TIME_H){
    len += PLC_TIME_LEN;
  }
  
  if((addr_higt & PLC_SIG_PR_H) == PLC_SIG_PR_H){
    len += PLC_PR_LEN;
  }
  len += CONTROL_FEILD_SIZE + ADR_OF_SIG_FEILD_SIZE;
  #ifdef DEBUG
  printf("get_iec3_sig_len_in_pack::len = %d\n", len);
  #endif
  return len;
}

//проверить что нужно ставить сигналу в поле управления
int check_addr4resp(kp_cfg_t *kp_cfg ,unsigned int addr){
  unsigned int i;
  unsigned int prev;
  unsigned int next;
  unsigned int cur;
  unsigned int data;
  printf("addr = %X\n", addr);
  /*if(kp_cfg->asked_data_num !=0){
    for(i = 0; i < kp_cfg->asked_data_num; i++){
      printf("kp_cfg->asked_data[i] = %X\n", kp_cfg->asked_data[i]);
      if(kp_cfg->asked_data[i] == addr) {
	//есть сигнал
	printf("kp_cfg->asked_data[i] == addr\n");
	kp_cfg->asked_data_num--;
	return 1;
      }
    }
    }*/
  if(kp_cfg->asked_data.asked_data_num !=0){
    //есть элементы в списке
    if(kp_cfg->asked_data.data[kp_cfg->asked_data.first].data == addr){
      //нашолся сразу
      kp_cfg->asked_data.data[kp_cfg->asked_data.first].use = 0;
      kp_cfg->asked_data.first = kp_cfg->asked_data.data[kp_cfg->asked_data.first].next;
      kp_cfg->asked_data.asked_data_num--;
      return 1;
    }
    cur = kp_cfg->asked_data.first;
    for(i = 0; i < kp_cfg->asked_data.asked_data_num; i++){
      next = kp_cfg->asked_data.data[cur].next;
      prev = kp_cfg->asked_data.data[cur].prev;
      data = kp_cfg->asked_data.data[cur].data;
      if(data == addr) {
	//есть сигнал
	printf("data == addr\n");
	kp_cfg->asked_data.data[prev].next = next;
	kp_cfg->asked_data.data[next].prev = prev;
	kp_cfg->asked_data.data[cur].use = 0;
	kp_cfg->asked_data.asked_data_num--;
	if(cur == kp_cfg->asked_data.last){
	  kp_cfg->asked_data.last = prev;
	}
	return 1;
      }
      cur = next; 
    }
  }
  //ничего не найдено
  return 0;
}

void copy_plc_sig2iec3_sig(kp_cfg_t *kp_cfg, unsigned char *pos_plc_data, unsigned int *plc_sig_len, unsigned char *pos_temp_sign, unsigned int *temp_sig_len){
  unsigned char pr;
  unsigned int sig_len = 0;
  unsigned char iec3_addr_low, iec3_addr_higt;
  unsigned int addr, i;
  unsigned int iec3_tz_size;       //размер данных передоваемого типа

  //вычисляем адрес сигнала  
  iec3_addr_low =  *(pos_plc_data + PLC_ADDR_LOW);   //Извлекаем адресс
  iec3_addr_higt = *(pos_plc_data + PLC_ADDR_HIGT);
  addr = iec3_addr_low | (iec3_addr_higt<<8);
  pr = *(pos_plc_data + PLC_PR);                    //Извлекаем признак
  sig_len = *(pos_plc_data + PLC_LEN);
  
  pos_plc_data += PLC_DATA_ADD; 

  *pos_temp_sign =  get_iec3_sig_len_in_pack(iec3_addr_higt, sig_len); 
  //#ifdef DEBUG
  printf("*pos_temp_sign =  get_iec3_sig_len_in_pack(iec3_addr_higt, sig_len) = %X\n", *pos_temp_sign);
  //#endif
  pos_temp_sign++;	 

  //CONTROL_TYPE_FELD
  if(check_addr4resp(kp_cfg, addr & PLC_SIG_ADDR_WITH_TYPE)){ 
    #ifdef DEBUG
    printf("check_addr4resp(kp_cfg, addr & PLC_SIG_ADDR_WITH_TYPE)\n");
    #endif
    *pos_temp_sign = CONTROL_TYPE_FELD_RESP;
  }else{
    #ifdef DEBUG
    printf("check_addr4resp(kp_cfg, addr & PLC_SIG_ADDR_WITH_TYPE) == 0\n");
    #endif
    *pos_temp_sign = CONTROL_TYPE_FELD_SPON;
  } 
  
  pos_temp_sign++;
  *pos_temp_sign = iec3_addr_low; pos_temp_sign++;                         //ADDRSIG_LOW_BYTE
  *pos_temp_sign = iec3_addr_higt; pos_temp_sign++;                        //ADDRSIG_HIGT_BYTE
  *temp_sig_len = BEGIN_DATA_ADD;
  *plc_sig_len = PLC_ADD_PACK_SIZE;

  if((iec3_addr_higt & PLC_SIG_TIME_H) == PLC_SIG_TIME_H){
    //с меткой времени
    for(i = 0; i < sig_len; i++){
      //копируем сигнал
      *pos_temp_sign = *(pos_plc_data + i + PLC_ADD_TIME);
      pos_temp_sign++;
    }
    
    if((iec3_addr_higt & PLC_SIG_PR_H) == PLC_SIG_PR_H){
    //со статусом
    *pos_temp_sign = pr;
    *pos_temp_sign++;
    *temp_sig_len += PLC_PR_LEN;
    }
    for(i = 0; i < PLC_TIME_LEN; i++){
      //копируем метку времени
      *pos_temp_sign = *(pos_plc_data + i);
      pos_temp_sign++;
    }
    *temp_sig_len += sig_len + PLC_TIME_LEN;
    *plc_sig_len += sig_len + PLC_TIME_LEN;
  }else{
    //без метки времени
    for(i = 0; i < sig_len; i++){
      *pos_temp_sign = *(pos_plc_data + i);
      pos_temp_sign++;
    }
    *temp_sig_len += sig_len;
    if((iec3_addr_higt & PLC_SIG_PR_H) == PLC_SIG_PR_H){
    //со статусом
    *pos_temp_sign = pr;
    *pos_temp_sign++;
    *temp_sig_len += PLC_PR_LEN;
    }
    *plc_sig_len += sig_len;
  }
  if((*plc_sig_len)%2 !=0) (*plc_sig_len)++;
  #ifdef DEBUG
  printf("АДРЕС СТРАНИЙ:%X\n", iec3_addr_higt);
  printf("АДРЕС МЛАДШИЙ:%X\n", iec3_addr_low);
  printf("ПРИЗНАК СИГНАЛА:%X\n", pr);
  printf("ДЛИНА СИГНАЛА:%X\n", sig_len);
  printf("ДЛИНА СИГНАЛА В ПАКЕТЕ:%X\n", *(pos_plc_data + PLC_LEN - PLC_DATA_ADD));
  printf("ДЛИНА ПАКЕТА ОТ ПЛК:%X\n", *plc_sig_len);
  #endif
}

unsigned char* write_begin_of_iec3_big_pack(kp_cfg_t *kp_cfg, unsigned char *pack, unsigned int *pack_size, unsigned char *CRC){
  //возможно этот кусоче следует сделать отдельной функцией чтобы не копипастить
  *pack_size = ADD_PACK_SIZE;
  pack[START_BYTE1_VAR] = VAR_LEN_FRAME_TYPE;
  pack[START_BYTE2_VAR] = VAR_LEN_FRAME_TYPE;
  *CRC = pack[SERVICE_CODE_VAR] = 0x88;       //SERVICE_CODE_USER_DATA | DFC_MASK;
  *CRC += pack[KP_ADDR_VAR] = kp_cfg->addr;
  return pack + KP_ADDR_VAR + 1;
}

/*Функция преобразующая данные из КЛТМ (ПЛК) к виду, требуемому для их передачи с записью в буфер*/
unsigned int iec3_convert_plc2kltm(char *plc_data, unsigned int len,  kp_cfg_t *kp_cfg, iec3_buf_data_send_t *iec3_buf){
  unsigned int i;
  unsigned int j;
  unsigned int k;
  unsigned int plc_sig_len;
  unsigned int temp_sig_len;       //столько занимает обработаный пакет 
  unsigned int pack_size = 0;
  unsigned char CRC = 0;          //контрольная сумма
  unsigned char *pos_temp_pack;   //позиция для вставки в подготовливаемом буфере на передачу
  unsigned char *pos_plc_data;
  unsigned char temp_pack[MAX_IEC_OUT_BUF_LEN]; //пакет будет формироваться здеся
  unsigned char temp_sign[MAX_SIGN_SIZE]; //место хранения сигнала в пакете
  
  j = 0; //счетчик - защита от бесконечного цикал
  i = 0; //счеткик - размер данных от ПЛК
  temp_sig_len = 0;
  pos_plc_data = plc_data;
  
  pos_temp_pack = write_begin_of_iec3_big_pack(kp_cfg, temp_pack,  &pack_size, &CRC); //подготовливаем начала пакета на передачу
  do{

    copy_plc_sig2iec3_sig(kp_cfg, pos_plc_data, &plc_sig_len, temp_sign, &temp_sig_len);//преобразовываем данные из контроллера в формат блока передачи

    if((temp_sig_len + pack_size + ADD_END_OF_PACK) >= kp_cfg->pack_size){
      //если добавить нельзя - превысим максимальный размер передоваемого пакета
      //запихиваем его в буфер на передачу с предварительной подготовкой

      //Добавляем длину пакета
      temp_pack[SIZE_LOW_VAR] = pack_size  - SUB_PACK_SIZE; 
      temp_pack[SIZE_HIGH_VAR] = pack_size - SUB_PACK_SIZE;
      for(k = 0; k < pack_size  - ADD_PACK_SIZE; k++) CRC += temp_pack[k + DATA_VAR]; //считаем контрольную сумму
      *pos_temp_pack = CRC;  //добавляем в конец контрольную сумму 
      pos_temp_pack++;
      *pos_temp_pack = END_OF_FRAME; //добавляем в конец признак завершения кадра
      iec3_buf_wrt(temp_pack, pack_size + CRC_END_LEN, iec3_buf);
      pos_temp_pack = write_begin_of_iec3_big_pack(kp_cfg, temp_pack,  &pack_size, &CRC);
    }
    
    //добавляем в конец пакета на передачу пакет с данными
    memcpy(pos_temp_pack, temp_sign, temp_sig_len);
    
    //сдвигаемся в пакете на отправку на величену добавленных в него данных
    pos_temp_pack += temp_sig_len;
    //#ifdef DEBUG
    printf("Длина малого пакета с данными %d\n", temp_sig_len);
    printf("Пременная pos_temp_pack указывает на следующий элемент массива: %d\n", pos_temp_pack - temp_pack);
    //#endif
    j++;
    if( plc_sig_len%2 == 0){
      i += plc_sig_len;
    }else{
      i += plc_sig_len + 1;
    }
    pos_plc_data = plc_data + i; //переходми на следующий блок данных от ПЛК 
    pack_size += temp_sig_len;   //увеличиваем на добавку 
  }while(i < len && j < PLC_MAX_PACK_LEN);
 
  //Добавляем длину пакета
  temp_pack[SIZE_LOW_VAR] = pack_size  - SUB_PACK_SIZE; 
  temp_pack[SIZE_HIGH_VAR] = pack_size - SUB_PACK_SIZE;

  for(k = 0; k < pack_size  - ADD_PACK_SIZE; k++) CRC += temp_pack[k + DATA_VAR]; //считаем контрольную сумму

  *pos_temp_pack = CRC;  //добавляем в конец контрольную сумму 

  pos_temp_pack++;
  #ifdef DEBUG
  printf("Пременная pos_temp_pack указывает на следующий элемент массива: %d\n", pos_temp_pack - temp_pack);
  printf("Размер большого пакета для записи в буфер на передачу равен: %d\n", pack_size + CRC_END_LEN);
  #endif
  *pos_temp_pack = END_OF_FRAME; //добавляем в конец признак завершения кадра
  iec3_buf_wrt(temp_pack, pack_size + CRC_END_LEN, iec3_buf);
  return j;
}
