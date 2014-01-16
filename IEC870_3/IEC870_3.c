/************************************************
ОАО НПО Спецэлетромеханика Брянск 2014
Модуль с функциями для работы по протоколу 
Элеси МЭК-3
 ************************************************/
#include "IEC870_3.h"
#include "IEC870_PLC.h"

/*Разбор принятого пакета*/
int read_iec_data(char *in_buf, int in_len, kp_cfg_t *kp_cfg){
  int i = 0;
  int pack_len = 0;
  int result = 0;
  while(i<in_len){
    switch(in_buf[i]){
    case VAR_LEN_FRAME_TYPE:
      //разбор кадра переменной длинны
      result = analize_var_frame(&in_buf[i], in_len - i, kp_cfg, &pack_len);
      if(!pack_len) i += pack_len; //длина не нулевая продолжаем обработку 
      else {
	wrt_out_buf();
	return  result; //ошибка длины пакета выходим нах дальше понять, что в буфере невозможно
      }
      break;
    case CONST_LEN_FRAME_TYPE:
      //разбор кадра постоянной длинны
       result = analize_const_frame(&in_buf[i], in_len - i, kp_cfg);
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
  wrt_out_buf();
  return  result;
}


/*Разбор пакета постоянной длинны*/
int  analize_const_frame(char *in_buf, int in_len, kp_cfg_t *kp_cfg){
  int code;
  #ifdef DEBUG
  printf("Адрес опрашиваемого КП: %d\n",in_buf[KP_ADDR_CFL] ); 
  #endif
    if(in_buf[SERVICE_CODE_CFL] && PRM_MASK == PRM_MASK){
      //Пакет от первичной станции
      #ifdef DEBUG
      printf("Пакет пришол от базовой станции\n"); 
      #endif
      
      if(in_buf[KP_ADDR_CFL] == kp_cfg->addr || in_buf[KP_ADDR_CFL] == kp_ALL){
	/*адрес КП совпадает с текущем адресом КП или направлен на все КП*/
	if(in_buf[END_OF_CFL] ==  END_OF_FRAME){
	  /*есть символ конца пакета*/
	  if(in_buf[CHECKSUM_CLF] == (in_buf[SERVICE_CODE_CFL] + in_buf[KP_ADDR_CFL])){
	    /*С контрольной суммой все ОК*/
	    //анализируем контрольное поле
	    return analize_control_feld(in_buf[SERVICE_CODE_CFL], kp_cfg, &code);
	  } else{//Ошибка контрольной суммы
            #ifdef DEBUG
	    printf("Ошибка контрольной суммы\n");
            #endif
	    return ERROR_CRC_CFL;
	  }
	}else{//отсуствует символ конца постоянного пакета
          #ifdef DEBUG
	  printf("Отсутствует конец пакета\n");
          #endif
	  return ERROR_CFL_END;
	}
      }else{//пакет адресован не нам
        #ifdef DEBUG
	printf("Этот запрос пришол не нам\n");
        #endif
	return 0;
      }
    }else{
        #ifdef DEBUG
	printf("Этот запрос пришол не от контролирующей станции игнорируем его\n");
        #endif
	return 0;
    }
}

/*Функция заталкивающая в выходной буфер подтверждающий кадр*/
void send_ACK(kp_cfg_t *kp_cfg){ 
  unsigned char temp_strig[5];
  
  if((kp_cfg->state & KP_CFG_ACK_MASK)== KP_CFG_SHORT_ACK){
    //короткий ответ
    
    #ifdef DEBUG
    printf("Используется короткий тип пакета для потверждения\n ");
    #endif
  
    temp_strig[START_BYTE_CFL] = ACK_POSITIVE_FRAME_TYPE;
    iec3_buf_wrt(temp_strig, SHORT_ACK_LEN);
    
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
    iec3_buf_wrt(temp_strig, CONSTANT_FRAME_LEN);
  }
}

/*Функция заталкивающая в выходной буфер отрицательного потверждения*/
void send_NACK(kp_cfg_t *kp_cfg){ 
  unsigned char temp_strig[5];
  if((kp_cfg->state & KP_CFG_ACK_MASK)== KP_CFG_SHORT_ACK){
    //короткий ответ
   
    #ifdef DEBUG
    printf("Используется короткий тип пакета для отрицательного потверждения\n ");
    #endif
 
    temp_strig[START_BYTE_CFL] = ACK_NEGATIVE_FRAME_TYPE;
    iec3_buf_wrt(temp_strig, SHORT_ACK_LEN);
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
    iec3_buf_wrt(temp_strig, CONSTANT_FRAME_LEN);
  }
}


/*Функция заталкивающая в выходной буфер кадр с информацией об обсутсвии данных*/
void send_no_data(kp_cfg_t *kp_cfg){ 
  unsigned char temp_strig[5];
  if((kp_cfg->state & KP_CFG_ACK_MASK)== KP_CFG_SHORT_ACK){
    //короткий ответ
    
    #ifdef DEBUG
    printf("Используется короткий тип пакета отрицательного ответа на запрос данных\n ");
    #endif
    temp_strig[START_BYTE_CFL] = ACK_NEGATIVE_FRAME_TYPE;
    iec3_buf_wrt(temp_strig, SHORT_ACK_LEN);
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
    iec3_buf_wrt(temp_strig, CONSTANT_FRAME_LEN);
  }
}

/*функция разбора пакета переменной длинны*/
int  analize_var_frame(char *in_buf, int in_len, kp_cfg_t *kp_cfg, int *pack_len){
  //Сначала необходимо убедиться в корректности принятых данных и в том, что эти данные адресованны нам
  //рассчет длины пакета общей
  int pl, i, code, result;
  unsigned char CRC;
  CRC = 0;
  *pack_len = 0;  
  if( in_buf[KP_ADDR_VAR]== kp_cfg->addr || in_buf[KP_ADDR_VAR] == kp_ALL){
    //Пакет адресован нам или сразу всем КП
    #ifdef DEBUG
    printf("Данные адресованы этому КП или всем КП\n ");
    #endif
    /*Старший и младший байт поля длина пакета дублируют дргу друга!!!*/
    if(in_buf[SIZE_LOW_VAR] ==  in_buf[SIZE_HIGH_VAR]){
      //с размером пакета все ОК
      pl= in_buf[SIZE_LOW_VAR] + ADD_SIZE_VAR;
      *pack_len = pl;
      #ifdef DEBUG
      printf("Поля размер пакета переменной длины совпадают\n");
      printf("Общая длина пакета перменной длинны составляет %d\n", pl);
      #endif
      if(in_buf[START_BYTE2_VAR] ==  VAR_LEN_FRAME_TYPE){
	//Опа и второе стартовое поле есть
        #ifdef DEBUG
	printf("Со вторым котрольным полем пакета перменной длины все впорядке\n");
        #endif
	if(in_buf[pl - 1] ==  END_OF_FRAME){
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
	  if(in_buf[pl - SUB_CRC_VAR] == CRC){
	    //контрольная сумма совпала
	    #ifdef DEBUG
	    printf("Пакет переменной длины: контрольная сумма совпала\n");
            #endif
	    //проверки кончились можно разбирать управляющее поле
	    result =  analize_control_feld(in_buf[SERVICE_CODE_VAR], kp_cfg, &code);
	    if(result != 0) return result;// ошибочка вышла
	    else{
	      //анализируем код функции
	      switch(in_buf[COMMAND_FELD]){
	      case  GENERAL_POLL:
                //пришла команда обшего опроса
                #ifdef DEBUG
		printf("Пришла команда общего опроса\n");
                #endif
		send_ACK(kp_cfg); //шлем подтверждение принятие команды
		kp_cfg->state |= KP_CFG_GENERAL_POLL; //запоминамем, что приняли команду общего опроса 
		return 0;
	      case SIGNAL_POLL:
		//пришол опрос конкретного сигнала
                #ifdef DEBUG
		printf("Пришола команда опроса конкретного сигнала\n");
                #endif
		send_ACK(kp_cfg); //шлем подтверждение принятие команды
		kp_cfg->state |= SIGNAL_POLL;
		return 0;
	      case TELECONTROL:
		//пришола команда телеуправления
                #ifdef DEBUG
		printf("Пришла команда телеуправления\n");
                #endif
		return 0;
	      case TR4_TYPE:
		//пришола команда телерегулирования четвертого типа
                #ifdef DEBUG
		printf("Пришла команда телерегулирования четвертого\n");
                #endif
		return 0;
	      default: 
		#ifdef DEBUG
		printf("неизвестная команда для пакета переменной длины\n");
                #endif
		return ERROR_UNKNOWN_COMMAND;
	      }
	    } 
	    
	  }else{
	    //ошибка контрольной суммы
	    #ifdef DEBUG
	    printf("Неравильная контрольная сумма у пакета перменной длины\n");
            #endif
	    return ERROR_CRC_VAR;
	  }
	}else{
	  //Нет символа - конца пакета
          #ifdef DEBUG
	  printf("Неправильный символ конца пакета у пакета переменной длины\n");
          #endif
	  return ERROR_VAR_END;
	}
      }else{
        #ifdef DEBUG
	printf("Неправильное значение второго стартового поля в пакете перменной длины\n");
        #endif
	return ERROR_BAD_SECOND_START_FEILD;
      }
    }else{
      //У пакета не совпадают поля размер пакета -  ошибка ахтунг!
      #ifdef DEBUG
      printf("Ошибка не сопадают поля размер пакета!!!\n ");
      #endif
      return  ERROR_LEN_VAR;
    }
  }else{
    //Пакет адресован не нам, не будем его обрабатывать
    #ifdef DEBUG
    printf("Данные адресованы не этому КП\n ");
    #endif
    return 0;
  }
  

}

//функция разбора поля управления
int  analize_control_feld(char control_feld, kp_cfg_t *kp_cfg, int *code){
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
    send_ACK(kp_cfg);
    kp_cfg->state |= KP_CFG_INIT;
    return 0;
    break;//май индусиш стайл :)
  case SERVICE_CODE_RESER_PROCES:
    //сброс пользовательского процесса
    #ifdef DEBUG
    printf("Сервисная функция: сброс пользовательского процесса\n ");
    #endif
    clear_send_buf();
    kp_cfg->state &= ~KP_CFG_FCB_MASK;
    send_ACK(kp_cfg);
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
    write_state_connection(kp_cfg);
    return 0;
    break;
  case SERVICE_CODE_DATA1_NEED:
    //запрос данных первой категории
    #ifdef DEBUG
    printf("Сервисная функция: запрос данных первой категории\n "); 
    #endif
    if(iec3_buf.num == 0) send_no_data(kp_cfg);
    return 0;
    break;
  case SERVICE_CODE_DATA2_NEED:
    //запрос данных второй категории
    #ifdef DEBUG
    printf("Сервисная функция: запрос данных второй категории\n "); 
    #endif
    send_no_data(kp_cfg);
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
void clear_send_buf(){
  iec3_buf.begin = 0;
  iec3_buf.end = 0;
  iec3_buf.len[FIST_PACK] = 0;
  iec3_buf.num = 0;
}

/*Функция записывающая в буфер ответ со статусом связи*/
void write_state_connection(kp_cfg_t *kp_cfg){
  unsigned char temp_strig[5];
  temp_strig[START_BYTE_CFL] = CONST_LEN_FRAME_TYPE;
  temp_strig[SERVICE_CODE_CFL] = SERVICE_CODE_CON_STAT;
  temp_strig[KP_ADDR_CFL] = kp_cfg->addr;
  temp_strig[CHECKSUM_CLF] = kp_cfg->addr + SERVICE_CODE_CON_STAT;
  temp_strig[END_OF_CFL] = END_OF_FRAME;
  iec3_buf_wrt(temp_strig, CONSTANT_FRAME_LEN);
}


/*Функция помещает в буфер данные подлежащие передачи*/
void iec3_send_data(kp_cfg_t *kp_cfg){
/*Начало: Тестовая попытка послать данные*/
  unsigned char temp_pack[MAX_IEC_OUT_BUF_LEN];
  unsigned char temp_sign[MAX_SIGN_SIZE];
  unsigned char *pos;

  unsigned char CRC = 0;
  unsigned int i, j;
  unsigned char pack_size = ADD_PACK_SIZE;
  
  
  temp_pack[START_BYTE1_VAR] = VAR_LEN_FRAME_TYPE;
  
  temp_pack[START_BYTE2_VAR] = VAR_LEN_FRAME_TYPE;
  CRC = temp_pack[SERVICE_CODE_VAR] = SERVICE_CODE_USER_DATA | DFC_MASK;
  CRC += temp_pack[KP_ADDR_VAR] = kp_cfg->addr;
  pos = &temp_pack[KP_ADDR_VAR + 1];
  #ifdef DEBUG
  printf("1: Контрольная сумма:  %x\n", CRC);
  #endif 
  for(i = 0;i < 3; i++){
    temp_sign[SIGNAL_TYPE_FELD] = TYPE_TI4_WITH_STAT;
    temp_sign[UNDEF_TYPE_FELD] = 0x47;
    temp_sign[ADDRSIG_LOW_BYTE] = 0x01 + i;
    temp_sign[ADDRSIG_HIGT_BYTE] = 0x14;
    temp_sign[ADDRSIG_HIGT_BYTE + SIG_TZ_TI4_1] = 0x00;
    temp_sign[ADDRSIG_HIGT_BYTE + SIG_TZ_TI4_2] = 0x00;
    temp_sign[ADDRSIG_HIGT_BYTE + SIG_TZ_TI4_3] = 0x00;
    temp_sign[ADDRSIG_HIGT_BYTE + SIG_TZ_TI4_4] = 0x00; 
    temp_sign[ADDRSIG_HIGT_BYTE + SIG_TZ_TI4_4 + PR_FELD] = 0x10;
    pack_size += SIZE_TI4_WITH_PR;
    #ifdef DEBUG
    printf("В подготовленом поле сигнала размера %d содержиться следующая сторока: ", SIZE_TI4_WITH_PR);
    for(j = 0; j < MAX_SIGN_SIZE; j++) printf("%x ",temp_sign[j]);
    printf("\n");
    #endif
    memcpy(pos, temp_sign, SIZE_TI4_WITH_PR);
    pos += TYPE_TI4_WITH_STAT + 1;
 }
  
  for(i = 0; i < pack_size  - ADD_PACK_SIZE; i++){
    CRC += temp_pack[i + DATA_VAR]; //считаем контрольную сумму
    #ifdef DEBUG
    printf("%x ", temp_pack[i + DATA_VAR]);
    #endif
  }
  #ifdef DEBUG
  printf("\n2: Контрольная сумма:  %x\n", CRC);
  #endif

  temp_pack[SIZE_LOW_VAR] = pack_size  - SUB_PACK_SIZE; 
  temp_pack[SIZE_HIGH_VAR] = pack_size - SUB_PACK_SIZE;

  temp_pack[pack_size + ADD_CRC_VAR] = CRC; 
  temp_pack[pack_size + ADD_END_VAR] = END_OF_FRAME;

  #ifdef DEBUG
  printf("В подготовленом пакете размера %d содержиться следующая сторока: ", pack_size);
  for(i = 0; i < MAX_IEC_OUT_BUF_LEN; i++) printf("%x ",temp_pack[i]);
  printf("\n");
  #endif
  iec3_buf_wrt(temp_pack, pack_size + CRC_END_LEN);
}


/*Функция перекладывающая из большого буфера данных на передачу в малый*/
void wrt_out_buf(){
  unsigned i, j;
  unsigned int pack_len;
  #ifdef DEBUG
  printf("3: Начало кольцевого буфера длины пакетов: %d , значение в буфере: %x\n", iec3_buf.len_begin, iec3_buf.len[iec3_buf.len_begin]);
  printf("4: Конец  кольцевого буфера длины пакетов: %d , значение в буфере: %x\n", iec3_buf.len_end, iec3_buf.len[iec3_buf.len_end]);
  printf("5: Количество пакетов в большом буфере: %d \n", iec3_buf.num);
  #endif
  if(iec3_buf.num !=0 ){
    pack_len = iec3_buf.len[iec3_buf.len_begin];
    iec3_out_buf.len = pack_len;
    for(i = 0, j = iec3_buf.begin; i < pack_len; i++, j++){
      if(j  ==  MAX_IEC_OUT_ALL_BUF_LEN) j = 0;
      iec3_out_buf.data[i] = iec3_buf.data[j];
    }
    iec3_buf.begin = j;
    iec3_buf.len_begin++;
    if(iec3_buf.len_begin == MAX_IEC_OUT_ALL_BUF_LEN) iec3_buf.len_begin = 0;
    iec3_buf.num--;  //Уменьшаем количество пакетов буфере на 1  и размер буфера
    
  }else iec3_out_buf.len = 0;
  #ifdef DEBUG
  printf("6: Начало кольцевого буфера длины пакетов: %d , значение в буфере: %x\n", iec3_buf.len_begin, iec3_buf.len[iec3_buf.len_begin]);
  printf("7: Конец  кольцевого буфера длины пакетов: %d , значение в буфере: %x\n", iec3_buf.len_end, iec3_buf.len[iec3_buf.len_end]);
  printf("8: Записано в буфер на передачу сообщение длиной %d: ", iec3_out_buf.len);
  for(i = 0; i < iec3_out_buf.len; i++) printf("%x ",iec3_out_buf.data[i]);
  printf("\n");
  #endif
}


void iec3_init(kp_cfg_t *kp_cfg){
  unsigned int i, j;
  kp_cfg->state = 0;
  kp_cfg->addr_sig_low = 0;
  kp_cfg->addr_sig_higt = 0;
  iec3_buf.num = 0;
  iec3_buf.begin = 0;
  iec3_buf.end = 0;
  iec3_buf.len_begin = 0;
  iec3_buf.len_end = 0;
  for(i = 0; i < MAX_IEC_OUT_ALL_BUF_LEN; i ++){
    iec3_buf.data[i] = 0;
    iec3_buf.len[i] =0;
  }
  iec3_out_buf.len = 0;
  for(i = 0; i < MAX_IEC_OUT_ALL_BUF_LEN; i ++) iec3_out_buf.data[i] = 0;
}

/*Функция записи в большой буфер передачи*/
void iec3_buf_wrt(unsigned char *data, unsigned int len){
  unsigned int i, j;
  iec3_buf.len[iec3_buf.len_end] = len;
  iec3_buf.len_end++;
  if(iec3_buf.len_end == MAX_IEC_OUT_ALL_BUF_LEN) iec3_buf.len_end = 0;
  for(i = 0, j = iec3_buf.end; i < len; i++, j++){
    if(j  ==  MAX_IEC_OUT_ALL_BUF_LEN) j = 0;
    iec3_buf.data[j] = data[i];
  } 
  iec3_buf.end = j;  
  iec3_buf.num++;
}


/*Функция преобразующая данные из КЛТМ (ПЛК) к виду, требуемому для их передачи с записью в буфер*/
unsigned int iec3_convert_plc2kltm(char *plc_data, unsigned int len,  kp_cfg_t *kp_cfg){
  unsigned int i;
  unsigned int j;
  unsigned int k;
  unsigned int l;
  unsigned int m;
  unsigned int n;
  unsigned int addr;
  unsigned int plc_sig_len;
  unsigned int temp_sig_len;       //столько занимает обработаный пакет 
  unsigned int add_size_time;      //если время передается то тут размер поля метка времени, иначе ноль
  unsigned int add_size_pr;        //если признак передается тут его размер, если нет, то ноль
  unsigned char pr;
  unsigned int krit;
  unsigned int iec3_tz_size;       //размер данных передоваемого типа
  unsigned int pack_size;
  unsigned char iec3_addr_low, iec3_addr_higt;
  unsigned char tz[4];
  unsigned char time[4];          
  unsigned char iec3_sig_type;    //тип сигнала на передачу тут храниться будет
  unsigned char CRC = 0;          //контрольная сумма
  unsigned char *pos_temp_pack;   //позиция для вставки в подготовливаемом буфере на передачу
  unsigned char *pos_temp_sign;    //позиция в подготовлеваемом сигнале, который будет вставляться в подготовливаемый буфер на передачу
  unsigned char *pos_plc_data;

  unsigned char temp_pack[MAX_IEC_OUT_BUF_LEN]; //пакет будет формироваться здеся
  unsigned char temp_sign[MAX_SIGN_SIZE]; //место хранения сигнала в пакете
  
  j = 0; //счетчик - защита от бесконечного цикал
  i = 0;
  
  //возможно этот кусоче следует сделать отдельной функцией чтобы не копипастить
  pack_size = ADD_PACK_SIZE;
  temp_pack[START_BYTE1_VAR] = VAR_LEN_FRAME_TYPE;
  temp_pack[START_BYTE2_VAR] = VAR_LEN_FRAME_TYPE;
  CRC = temp_pack[SERVICE_CODE_VAR] = SERVICE_CODE_USER_DATA | DFC_MASK;
  CRC += temp_pack[KP_ADDR_VAR] = kp_cfg->addr;
  pos_temp_pack = &temp_pack[KP_ADDR_VAR + 1];
  
  temp_sig_len = 0;
  pos_temp_sign = temp_sign;
  pos_plc_data = plc_data;
  do{
    //вычисляем адрес сигнала  
    iec3_addr_low =  *(pos_plc_data + PLC_ADDR_LOW);   //Извлекаем адресс
    iec3_addr_higt = *(pos_plc_data + PLC_ADDR_HIGT);
    addr = iec3_addr_low | (iec3_addr_higt<<8);
    pr = *(pos_plc_data + PLC_PR);                    //Извлекаем признак
    plc_sig_len = *(pos_plc_data + PLC_LEN);          //длина пакета
    
    #ifdef DEBUG
    printf("Прочитано %d блоков\n", j);
    printf("Длина первого прочитанного из ПЛК блока: %d\n", plc_sig_len);
    printf("Старший байт адрес: %x\n Младший байт адреса %x\n", iec3_addr_higt, iec3_addr_low);
    printf("Адрес объекта информации: %x\n", addr);
    printf("Признак передаваемого сигнала: %x\n", pr);
    #endif
    
    //устанавливаем указатель на позициию с данными или временем
    pos_plc_data += PLC_DATA_ADD;

    //сигнал имеет метку времени или статус?
    if(((addr) & PLC_SIG_CRIT) == PLC_SIG_CRIT){
      //оба метка времени и статус
      krit = PLC_SIG_WITH_BOTH;
      add_size_time = PLC_TIME_LEN;
      add_size_pr = PLC_PR_LEN;
      //Извлекаем метку времени она понадобиться позже
      for(k = 0; k < PLC_ADD_TIME; k++) {
	time[k] = *pos_plc_data;
	pos_plc_data++;
      }
      #ifdef DEBUG
      printf("Пакет от ПЛК со статусом и меткой времени\n");
      #endif
    }else if((addr & PLC_SIG_CRIT) == PLC_SIG_PR){
      //только статус
      krit = PLC_SIG_WITH_PR;
      add_size_pr = PLC_PR_LEN;
      add_size_time = 0;
	}else if((addr & PLC_SIG_CRIT) == PLC_SIG_TIME){
      //только время
      krit = PLC_SIG_WITH_TIME;
      add_size_time = PLC_TIME_LEN;
      add_size_pr = 0;
      #ifdef DEBUG
      printf("Пакет от ПЛК со статусом\n");
      #endif
      //Извлекаем метку времени она понадобиться позже
      for(k = 0; k < PLC_ADD_TIME; k++) {
	time[k] = *pos_plc_data;
	pos_plc_data++;
	#ifdef DEBUG
	printf("Пакет от ПЛК с  меткой времени\n");
        #endif
      }
    }else{
      //ни того ни другого
      krit = PLC_SIG_WITH_NONE;
      add_size_time = 0;
      add_size_pr = 0;
      #ifdef DEBUG
      printf("Пакет от ПЛК без метки времени и статуса\n");
      #endif
    }

    //определение длины сигнала в зависимости от типа
    switch(addr & PLC_SIG_TYPE){
    case PLC_SIG_TS:    
      GET_PLC_KRIT(addr, TYPE_TS_WITH_STAT_AND_TIME, TYPE_TS_WITH_STAT, TYPE_TS_WITH_TIME, TYPE_TS, iec3_sig_type);
      iec3_tz_size = PLC_TS_LEN;
      break;
    case PLC_SIG_TI1:    
      GET_PLC_KRIT(addr, TYPE_TI1_WITH_STAT_AND_TIME, TYPE_TI1_WITH_STAT, TYPE_TI1_WITH_TIME, TYPE_TI1, iec3_sig_type);
      iec3_tz_size = PLC_TI1_LEN;
      break;
    case PLC_SIG_TI2:    
      GET_PLC_KRIT(addr, TYPE_TI2_WITH_STAT_AND_TIME, TYPE_TI2_WITH_STAT, TYPE_TI2_WITH_TIME, TYPE_TI2, iec3_sig_type);
      iec3_tz_size = PLC_TI2_LEN;
      break;
    case PLC_SIG_TI4:
      GET_PLC_KRIT(addr, TYPE_TI4_WITH_STAT_AND_TIME, TYPE_TI4_WITH_STAT, TYPE_TI4_WITH_TIME, TYPE_TI4, iec3_sig_type);
      iec3_tz_size = PLC_TI4_LEN;
      break;
    default: return 0;
    }
    
    #ifdef DEBUG
    printf("Добавочный размер поля время составляет: %d\n", add_size_time);
    printf("Добавочный пазмер поля признак составляет: %d\n", add_size_pr);
    printf("Длина поля данные равна: %d\n", iec3_tz_size);
    #endif
    *pos_temp_sign = iec3_sig_type; pos_temp_sign++;  //SIGNAL_TYPE_FELD
    *pos_temp_sign = 0x47; pos_temp_sign++;           //UNDEF_TYPE_FELD
    *pos_temp_sign = iec3_addr_low; pos_temp_sign++;  //ADDRSIG_LOW_BYTE
    *pos_temp_sign =iec3_addr_higt; pos_temp_sign++;  //ADDRSIG_HIGT_BYTE

    //pos_plc_data += PLC_DATA_ADD + add_size_time;
    #ifdef DEBUG
    printf("Копирую следующий блок данных:");
    #endif
    for(k = 0; k < iec3_tz_size;  k++){
      *pos_temp_sign = *pos_plc_data; 
      #ifdef DEBUG
      printf("%x ", *pos_plc_data);
      #endif
      pos_temp_sign++;
      pos_plc_data++;
    }
    #ifdef DEBUG
    printf("\n");
    #endif

    switch(krit){
    case PLC_SIG_WITH_BOTH:
      *pos_temp_sign = pr;
      pos_temp_sign++;
      #ifdef DEBUG
      printf("Копирую следующий блок данных времени:");
      #endif
      for(k = 0; k < PLC_ADD_TIME; k++) {
	*pos_temp_sign = time[k];
	printf("%x ", *pos_temp_sign);
	pos_temp_sign++;
      }
      #ifdef DEBUG
      printf("\n");
      #endif
      break;
    case PLC_SIG_WITH_PR:
      *pos_temp_sign = pr;
      pos_temp_sign++;
      break;
    case PLC_SIG_WITH_TIME:
      for(k = 0; k < PLC_ADD_TIME; k++) {
	*pos_temp_sign = time[k];
	pos_temp_sign++;
      }
      break;
    case PLC_SIG_WITH_NONE:
      break;
    }
    
    //длина подготовленной части, для включения ее в большой пакет
    temp_sig_len = iec3_tz_size +  add_size_time + add_size_pr + BEGIN_DATA_ADD;
    #ifdef DEBUG
    printf("Длина малого пакета с данными %d\n", temp_sig_len);
    printf("Длина малого блока данных от ПЛК %d\n", plc_sig_len);
    printf("Пременная pos_temp_pack указывает на следующий элемент массива: %d\n", pos_temp_pack - temp_pack);
    printf("Общий размер формируемого на передачу пакета: %d\n", temp_sig_len + pack_size + ADD_END_OF_PACK);
    #endif

    if((temp_sig_len + pack_size + ADD_END_OF_PACK) >= MAX_IEC_OUT_BUF_LEN){
      //если добавить нельзя - превысим максимальный размер передоваемого пакета
      //запихиваем его в буфер на передачу с предварительной подготовкой

      //Добавляем длину пакета
      temp_pack[SIZE_LOW_VAR] = pack_size  - SUB_PACK_SIZE; 
      temp_pack[SIZE_HIGH_VAR] = pack_size - SUB_PACK_SIZE;

      for(k = 0; k < pack_size  - ADD_PACK_SIZE; k++) CRC += temp_pack[k + DATA_VAR]; //считаем контрольную сумму
      *pos_temp_pack = CRC;  //добавляем в конец контрольную сумму 
      pos_temp_pack++;
      *pos_temp_pack = END_OF_FRAME; //добавляем в конец признак завершения кадра

      iec3_buf_wrt(temp_pack, pack_size + CRC_END_LEN);
      
      //подготавливаем новый буфер для создания нового пакета с данными
      pack_size = ADD_PACK_SIZE;
      temp_pack[START_BYTE1_VAR] = VAR_LEN_FRAME_TYPE;
      temp_pack[START_BYTE2_VAR] = VAR_LEN_FRAME_TYPE;
      CRC = temp_pack[SERVICE_CODE_VAR] = SERVICE_CODE_USER_DATA | DFC_MASK;
      CRC += temp_pack[KP_ADDR_VAR] = kp_cfg->addr;
      pos_temp_pack = &temp_pack[KP_ADDR_VAR + 1];
    }
    
    //добавляем в конец пакета на передачу пакет с данными
    memcpy(pos_temp_pack, temp_sign, temp_sig_len);
    
    //Пакет скопрровали, теперь возвращаемся на исходную позицию
    pos_temp_sign = temp_sign;

    //сдвигаемся в пакете на отправку на величену добавленных в него данных
    pos_temp_pack += temp_sig_len;
    #ifdef DEBUG
    printf("Длина малого пакета с данными %d\n", temp_sig_len);
    printf("Пременная pos_temp_pack указывает на следующий элемент массива: %d\n", pos_temp_pack - temp_pack);
    #endif
    j++;
    i += plc_sig_len;
    pos_plc_data = plc_data + i; //переходми на следующий блок данных от ПЛК 
    
    pack_size += temp_sig_len;   //увеличиваем на добавку 
  }while(i < len && j < PLC_MAX_PACK_LEN);
 
  //Добавляем длину пакета
  temp_pack[SIZE_LOW_VAR] = pack_size  - SUB_PACK_SIZE; 
  temp_pack[SIZE_HIGH_VAR] = pack_size - SUB_PACK_SIZE;

  for(k = 0; k < pack_size  - ADD_PACK_SIZE; k++) CRC += temp_pack[k + DATA_VAR]; //считаем контрольную сумму
  *pos_temp_pack = CRC;  //добавляем в конец контрольную сумму 
  #ifdef DEBUG
  printf("Пременная pos_temp_pack указывает на следующий элемент массива: %d\n", pos_temp_pack - temp_pack);
  printf("Контрольная сумма большого пакета:  %x\n", CRC);
  #endif
  pos_temp_pack++;
  #ifdef DEBUG
  printf("Пременная pos_temp_pack указывает на следующий элемент массива: %d\n", pos_temp_pack - temp_pack);
  printf("Размер большого пакета для записи в буфер на передачу равен: %d\n", pack_size + CRC_END_LEN);
  #endif
  *pos_temp_pack = END_OF_FRAME; //добавляем в конец признак завершения кадра
  iec3_buf_wrt(temp_pack, pack_size + CRC_END_LEN);
  return 0;
}
