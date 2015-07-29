/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///*********************** МОДУЛЬ ИНТЕРФЕЙСОВ ШЛЮЗА ****************************
///*** МЕХАНИЗМ ОПРОСА MODBUS TCP MASTER

///=== INTERFACES_H MODULE IMPLEMENTATION

#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

#include <fcntl.h>
#include <errno.h>

#include <stdio.h>
#include <string.h>

#include "interfaces.h"
#include "moxagate.h"
#include "messages.h"
#include "modbus.h"
#include "tsc.h"

int do_data_exchange(GW_Iface *tcp_master, int port_id, int *timeout_counter,
	                   u8 *req_adu, u16  req_adu_len,
	                   u8 *rsp_adu, u16 *rsp_adu_len);

///-----------------------------------------------------------------------------------------------------------------
void *iface_tcp_master(void *arg)
  {
	u8  req_adu[MB_UNIVERSAL_ADU_LEN];
	u16 req_adu_len;
	u8  rsp_adu[MB_UNIVERSAL_ADU_LEN];
	u16 rsp_adu_len;

  // приемный буфер для единичных измерений параметров (блоков ЕИП),
  // размер позволяет за один раз вычитать все содержимое буфера истории
  HB_EIP_Block rcv_eip[HAGENT_BUFEIP_MAX];

  int port_id=((unsigned)arg)>>8;
  int client_id, device_id;

	int status;
	int i, j;
	unsigned Q, bufptr, curlen, curptr;
	int connum, csd, active_connection=0, timeout_counter=0;
	int trans_id;

	GW_Iface	*tcp_master;
  HB_Header rcvhdr, lclhdr;

	struct timeval tv, tv_old, tv_res, tvchk, tv_wait, tveip;
	struct timezone tz;

  struct tm tmp_tm;
  char putstr[128];
  char prmtms[16];
  float prmval;

	int first_scan=1, rcv_header, lcl_header, eip2read, eip2store;
  int eip_size_reg, buf_size_eip, max_read_eip, start_addr, length_reg;

  connum = port_id-GATEWAY_T01;
	tcp_master = &IfaceTCP[connum];
	
  memset(&rcvhdr, 0, sizeof(HB_Header));
  memset(&lclhdr, 0, sizeof(HB_Header));

  status=init_tsc_h(port_id); // инициализация конфигурационного массива параметров истории
  if(status!=0) printf("init_tsc_h error %d\n", status);
//  printf("metric %s\n", TSC_Param[connum][0].metric);
//  printf("adc_max %d\n", TSC_Param[connum][0].adc_max);
//  printf("eng_max %f\n", TSC_Param[connum][0].eng_max);

	//tcp_master->clients[client_id].stat.request_time_min=10000; // 10 seconds, must be "this->serial.timeout"
	//tcp_master->clients[client_id].stat.request_time_max=0;
	//tcp_master->clients[client_id].stat.request_time_average=0;
	//for(i=0; i<MAX_LATENCY_HISTORY_POINTS; i++) tcp_master->clients[client_id].stat.latency_history[i]=1000; //ms
	//tcp_master->clients[client_id].stat.clp=0;

	/// semaphore
	tcp_master->queue.operations[0].sem_op=-1;
	semop(semaphore_id, tcp_master->queue.operations, 1);
	tcp_master->queue.operations[0].sem_flg=IPC_NOWAIT;

  // THREAD STARTED
	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|GATEWAY_LANTCP, IFACE_THREAD_STARTED, port_id, client_id, 0, port_id);

	//reset_tcpmaster_conn(tcp_master, 1);

		gettimeofday(&tvchk, &tz);

		tv_old.tv_sec = tvchk.tv_sec ;
		tv_old.tv_usec= tvchk.tv_usec;

///-----------------------------------------
	// в таблице опроса для данного порта д.б. ровно одна строка
	if(tcp_master->PQueryIndex[MAX_QUERY_ENTRIES]==1 && status==0) {
		
		Q=tcp_master->PQueryIndex[0];

		req_adu[TCPADU_ADDRESS]=   tcp_master->ethernet.unit_id;
		req_adu[TCPADU_FUNCTION]=	 PQuery[Q].mbf;

		req_adu_len=12;

		client_id=GW_CLIENT_MOXAGATE;

	while (1) {

		gettimeofday(&tv, &tz);

    timersub(&tv, &tv_old, &tv_res); // определение периода чтения буфера истории
		tv_old.tv_sec = tv.tv_sec ;
		tv_old.tv_usec= tv.tv_usec;

    MoxaDevice.wData4x[PQuery[Q].offset + HAGENT_HEADER_LEN + 0]= // период чтения буфера истории в милисекундах
      1000*tv_res.tv_sec + tv_res.tv_usec / 1000;
    MoxaDevice.wData4x[PQuery[Q].offset + HAGENT_HEADER_LEN + 1]++; // общее количество итераций

		// с установленной периодичностью проверяем состояние сетевых соединений
    if( (	(tv.tv_sec -tvchk.tv_sec )*1000000 +
    			(tv.tv_usec-tvchk.tv_usec)         ) >= TCP_RECONN_INTVL) {
    	if(tcp_master->ethernet.status ==TCPMSTCON_ESTABLISHED)
		    check_tcpmaster_conn(tcp_master, 1);
    	if(tcp_master->ethernet.status2==TCPMSTCON_ESTABLISHED)
		    check_tcpmaster_conn(tcp_master, 2);
			gettimeofday(&tvchk, &tz);
			}

    // выполняется выдержка времени для обеспечения заданной периодичности при опросе
		if(PQuery[Q].delay!=0) {
			// usleep(PQuery[Q].delay*1000);
			tv_wait.tv_sec  = PQuery[Q].delay / 1000;
			tv_wait.tv_usec =(PQuery[Q].delay % 1000) * 1000;
			select(0, NULL, NULL, NULL, &tv_wait);
		  }

		// проверка готовности tcp-соединения для связи с контроллером
		connum = tcp_master->ethernet.status==TCPMSTCON_ESTABLISHED ? 1:0;
		active_connection = tcp_master->ethernet.active_connection = connum;
		if(connum==0) {
      MoxaDevice.wData4x[PQuery[Q].offset + HAGENT_HEADER_LEN + 2]++; // ошибка. сетевое подключение не готово
			continue; // переход к следующей итерации
		  }

	  // получение заголовка буфера истории

		req_adu[TCPADU_START_HI]=	(PQuery[Q].start  >>8)&0xff;
		req_adu[TCPADU_START_LO]=  PQuery[Q].start      &0xff;
		req_adu[TCPADU_LEN_HI]=		(HAGENT_HEADER_LEN>>8)&0xff;
		req_adu[TCPADU_LEN_LO]= 	 HAGENT_HEADER_LEN    &0xff;
																																
		status = do_data_exchange(tcp_master, port_id, &timeout_counter,
	                            req_adu,  req_adu_len,
	                            rsp_adu, &rsp_adu_len);

		if(status!=0) { // если обмен данными с контроллером не был выполнен
      MoxaDevice.wData4x[PQuery[Q].offset + HAGENT_HEADER_LEN + 3]++; // ошибка. заголовок буфера не прочитан
		  continue; // переход к следующей итерации
		  }
	                   
	// сохраняем локально полученные данные
	process_proxy_response(Q, rsp_adu, rsp_adu_len);
	
  // получение значений из заголовка буфера истории
  status=check_header(rsp_adu, &rcvhdr);
	if(status!=0) {
    MoxaDevice.wData4x[PQuery[Q].offset + HAGENT_HEADER_LEN + 4]++; // ошибка. заголовок буфера не корректен
		continue; // переход к следующей итерации
		}

//  if(rcvhdr.header != lclhdr.header ||
//     rcvhdr.cycles != lclhdr.cycles ||
//     rcvhdr.min    != lclhdr.min    ||
//     rcvhdr.hour   != lclhdr.hour   ||
//     rcvhdr.day    != lclhdr.day    ||
//     rcvhdr.month  != lclhdr.month  ||
//     rcvhdr.year   != lclhdr.year   ||
//     rcvhdr.prmnum != lclhdr.prmnum ||
//     rcvhdr.msprec != lclhdr.msprec  )
//    printf("hdr%d, cycl%d; min%d, hour%d, day%d, month%d, year%d; prmnum%d, msprec%d;\n", 
//          rcvhdr.header, rcvhdr.cycles,
//          rcvhdr.min, rcvhdr.hour, rcvhdr.day, rcvhdr.month, rcvhdr.year,
//          rcvhdr.prmnum, rcvhdr.msprec);

  // continue; /// этап 1 выполнен, чтение заголовка буфера истории завершено

  if((rcvhdr.msprec != lclhdr.msprec) || // если критичные параметры буфера истории изменились, то
     (rcvhdr.prmnum != lclhdr.prmnum)  ) {
    first_scan=1; // инициализация переменных
    MoxaDevice.wData4x[PQuery[Q].offset + HAGENT_HEADER_LEN + 5]++; // изменены критичные параметры буфера истории
    }
											
  // определяется количество данных, поступивших в буфер истории со времени последнего чтения
  eip2read = (rcvhdr.cycles + 0x10000 - lclhdr.cycles) % 0x10000;     // разница в циклах
  eip2read = eip2read * buf_size_eip + rcvhdr.header - lclhdr.header; // разница в блоках ЕИП
  eip2store= eip2read; // запоминается количество блоков ЕИП, которое должно быть прочитано

  // количество не прочитанных блоков ЕИП в контроллере должно быть
  // не меньше нуля и не больше размера буфера истории в блоках ЕИП
  if(eip2read < 0             || // если была произведена переинициализация буфера истории или
     eip2read > buf_size_eip)  { // не прочитанные данные в буфере истории были перезаписаны (overrun)
    first_scan=1;                  // инициализация переменных
    if(eip2read < 0           ) MoxaDevice.wData4x[PQuery[Q].offset + HAGENT_HEADER_LEN + 6]++; // была произведена переинициализация буфера истории
    if(eip2read > buf_size_eip) MoxaDevice.wData4x[PQuery[Q].offset + HAGENT_HEADER_LEN + 7]++; // не прочитанные данные в буфере истории были перезаписаны (overrun)
    }

  if(first_scan==1) { // инициализация переменных
    first_scan=0;
    copy_header(&rcvhdr, &lclhdr);
    // определение общих характеристик буфера истории
    eip_size_reg = lclhdr.prmnum + 1;                                     // размер блока ЕИП в регистрах
    buf_size_eip = (PQuery[Q].length - HAGENT_HEADER_LEN) / eip_size_reg; // размер буфера истории в блоках ЕИП
    max_read_eip = MBF_0x03_MAX_QUANTITY / eip_size_reg;                  // максимальный размер блока данных для
                                                                          // чтения, выраженный в блоках ЕИП
    MoxaDevice.wData4x[PQuery[Q].offset + HAGENT_HEADER_LEN + 8]++; // произведена инициализация переменных
    continue; // переход к новому заголовку
    }

  if(rcvhdr.header >= buf_size_eip) { // указатель в заголовоке буфера истории не корректен
    MoxaDevice.wData4x[PQuery[Q].offset + HAGENT_HEADER_LEN + 9]++; // указатель в заголовоке буфера истории не корректен
  	continue; // переход к следующей итерации
    }

  if(eip2read == 0) { // в буфере истории нет новых данных
    MoxaDevice.wData4x[PQuery[Q].offset + HAGENT_HEADER_LEN + 10]++; // в буфере истории нет новых данных
  	continue; // переход к новому заголовку
    }

  MoxaDevice.wData4x[PQuery[Q].offset + HAGENT_HEADER_LEN + 11]++; // количество попыток чтения данных из буфера истории

  bufptr     = 0; // указатель на первую свободную ячейку в приемном буфере для блоков ЕИП
  lcl_header = lclhdr.header;
  while (eip2read > 0) { // в буфере истории есть не прочитанные данные

    // как минимум один параметр из имеющихся в буфере истории и не прочитанных должен быть достоверным

    rcv_header = rcvhdr.header;

    if(rcv_header <= lcl_header) { // достигнут конец буфера истории
      rcv_header = buf_size_eip;
      }

    // за один раз вычитывается блок данных размером не больше допустимого
    if((rcv_header - lcl_header) > max_read_eip) {
    	rcv_header = lcl_header + max_read_eip;
      }																						

    start_addr = PQuery[Q].start + HAGENT_HEADER_LEN + lcl_header * eip_size_reg;
    length_reg = (rcv_header - lcl_header) * eip_size_reg;
      
    // формирование запроса на чтение данных
		req_adu[TCPADU_START_HI]=	(start_addr>>8)&0xff;
		req_adu[TCPADU_START_LO]=  start_addr    &0xff;
		req_adu[TCPADU_LEN_HI]=		(length_reg>>8)&0xff;
		req_adu[TCPADU_LEN_LO]= 	 length_reg    &0xff;
																																
		status = do_data_exchange(tcp_master, port_id, &timeout_counter,
	                            req_adu,  req_adu_len,
	                            rsp_adu, &rsp_adu_len);

		if(status!=0) { // при ошибке пропускается весь не прочитанный блок
			eip2read = 0;
      MoxaDevice.wData4x[PQuery[Q].offset + HAGENT_HEADER_LEN + 12]++; // ошибка чтения данных
			continue; // завершение чтения данных
			}

    // полученные данные копируются в приемный буфер
    curlen = rcv_header - lcl_header;
    for(i=0; i<curlen; i++) {
    	rcv_eip[bufptr].validity = 0;
    	curptr = 2*eip_size_reg*i + 9;
      rcv_eip[bufptr].time_mark =    (rsp_adu[curptr          ]<<8) |
                                      rsp_adu[curptr       + 1]     ;
      for(j=0; j<lclhdr.prmnum; j++)
        rcv_eip[bufptr].prm[j] =     (rsp_adu[curptr + 2*j + 2]<<8) |
                                      rsp_adu[curptr + 2*j + 3]     ;
      bufptr++;
      }

    lcl_header = rcv_header>=buf_size_eip ? 0 : rcv_header;

    // определяется количество данных, еще не прочитанных из буфера истории
    eip2read -= curlen; // разница в блоках ЕИП

    MoxaDevice.wData4x[PQuery[Q].offset + HAGENT_HEADER_LEN + 13]++; // количество успешных операций чтения
    } // while (rcvhdr.header != lcl_header)

  // количество прочитанных блоков ЕИП должно быть больше нуля
  if(bufptr==0) { // новые данные не были прочитаны из буфера истории из-за ошибки
    copy_header(&rcvhdr, &lclhdr); // не прочитанные данные игнорируются
  	continue;                      // переход к новому заголовку
    }
  
  // количество прочитанных блоков ЕИП должно быть
  // равно количеству записанных в буфер контроллера со времени последнего чтения
  if(eip2store != bufptr) {        // новые данные не полностью были прочитаны из буфера истории
    copy_header(&rcvhdr, &lclhdr); // не прочитанные данные игнорируются, переход к новому заголовку
    MoxaDevice.wData4x[PQuery[Q].offset + HAGENT_HEADER_LEN + 14]++; // данные прочитаны из буфера не полностью
    continue;
    }
  
  MoxaDevice.wData4x[PQuery[Q].offset + HAGENT_HEADER_LEN + 15]++; // чтение очередного блока данных завершено
  // for(i=bufptr-1; i>=0; i--) {
  //   printf("tm%d; ", rcv_eip[i].time_mark);
  //   for(j=0; j<lclhdr.prmnum; j++) printf("%0.4X ", rcv_eip[i].prm[j]);
  //   printf("\n");
  //   }
  //  copy_header(&rcvhdr, &lclhdr); // новые данные успешно прочитаны из буфера, переход к новому заголовку
  //  continue; /// этап 2 выполнен, чтение очередного блока данных из буфера истории завершено

  // формирование и проверка меток времени блоков ЕИП
  for(i=bufptr-1; i>=0; i--) { // цикл по блокам ЕИП в обратном хронологическом порядке

    // разбор метки времени блока ЕИП, проверка на корректность
    if(rcvhdr.msprec==1) { // милисекундная точность
    	rcv_eip[i].ms  = rcv_eip[i].time_mark % 1000;
    	rcv_eip[i].sec = rcv_eip[i].time_mark / 1000;
    	rcv_eip[i].min = 0;
      if(rcv_eip[i].time_mark <= 59999) rcv_eip[i].validity = 1;
      } else { // секундная точность
    	  rcv_eip[i].ms  =((rcv_eip[i].time_mark & 0x03ff) %  10) * 100;
    	  rcv_eip[i].sec = (rcv_eip[i].time_mark & 0x03ff) /  10;
    	  rcv_eip[i].min = (rcv_eip[i].time_mark & 0xfc00) >> 10;
        if(rcv_eip[i].sec <= 59 &&
           rcv_eip[i].min <= 59  ) rcv_eip[i].validity = 1;
      	}

    // printf("valid:%d, ms:%d, sec:%d, min:%d\n", rcv_eip[i].validity, rcv_eip[i].ms, rcv_eip[i].sec, rcv_eip[i].min);

    if(i==(bufptr-1)) { // для хронологически последнего блока ЕИП

      // значения минут из метки времени в заголовке буфера и в блоке ЕИП должны совпадать
      if(rcvhdr.msprec==0 && rcvhdr.min!=rcv_eip[i].min) rcv_eip[i].validity = 0;

      if(rcv_eip[i].validity != 1) {
        MoxaDevice.wData4x[PQuery[Q].offset + HAGENT_HEADER_LEN + 16]++; // header time mark skip 1
      	break; // завершение проверки корректности данных
        }

      // формирование метки времени блока ЕИП в формате Linux
      rcvhdr.tm.tm_sec = rcv_eip[i].sec;
      rcvhdr.tv.tv_sec = rcv_eip[i].tv.tv_sec = mktime(&rcvhdr.tm);
      rcvhdr.tv.tv_usec= rcv_eip[i].tv.tv_usec= rcv_eip[i].ms * 1000;

      // время локального заголовка должно быть меньше времени полученного из контроллера заголовка
      if(timercmp(&lclhdr.tv, &rcvhdr.tv, >=) == 1) { // иначе
      	rcv_eip[i].validity = 0;
        MoxaDevice.wData4x[PQuery[Q].offset + HAGENT_HEADER_LEN + 17]++; // header time mark skip 2
      	break; // завершение проверки корректности данных
        }

     } else { // if(i==(bufptr-1)) ; для всех блоков ЕИП, записанных в буфер истории раньше

      // метка времени каждого хронологически предыдущего блока ЕИП должна отличаться от метки времени следующего
      if(rcv_eip[i+1].time_mark == rcv_eip[i].time_mark) rcv_eip[i].validity = 0;

      if(rcv_eip[i].validity != 1) {
        MoxaDevice.wData4x[PQuery[Q].offset + HAGENT_HEADER_LEN + 18]++; // eip time mark skip 1
      	break; // завершение проверки корректности данных
        }

      // вычисляется разность во времени в милисекундах между блоками ЕИП в предположении, что
      // при милисекундной точности период чтения не превышает одной минуты, а
      // при     секундной точности период чтения не превышает одного часа
      if(rcvhdr.msprec==1) {
        lcl_header = (rcv_eip[bufptr-1].time_mark  + 60000  // текущим и последним записанным
                    - rcv_eip[i       ].time_mark) % 60000;
        rcv_header = (rcv_eip[i+1     ].time_mark  + 60000  // текущим и хронологически следующим
                    - rcv_eip[i       ].time_mark) % 60000;
        } else {
        lcl_header = (60000*rcv_eip[bufptr-1].min + 1000*rcv_eip[bufptr-1].sec + rcv_eip[bufptr-1].ms  + 3600000
                    - 60000*rcv_eip[i       ].min - 1000*rcv_eip[i       ].sec - rcv_eip[i       ].ms) % 3600000;
        rcv_header = (60000*rcv_eip[i+1     ].min + 1000*rcv_eip[i+1     ].sec + rcv_eip[i+1     ].ms  + 3600000
                    - 60000*rcv_eip[i       ].min - 1000*rcv_eip[i       ].sec - rcv_eip[i       ].ms) % 3600000;
        }

      // printf("hdr:%d, eip:%d\n", lcl_header, rcv_header);

      // дополнительно проверяются временнЫе характеристики периода записи
      rcv_eip[i].validity =
        (rcvhdr.msprec==1 && rcv_header>=   1) ||
        (rcvhdr.msprec==0 && rcv_header>=1000)  ? 1 : 0;
      
      if(rcv_eip[i].validity == 0) {
        MoxaDevice.wData4x[PQuery[Q].offset + HAGENT_HEADER_LEN + 19]++; // eip time mark skip 2
      	break; // завершение проверки корректности данных
        }

      // формирование метки времени блока ЕИП в формате Linux
      // производится на основании метки времени заголовка
      tveip.tv_sec = lcl_header / 1000;
      tveip.tv_usec=(lcl_header % 1000) * 1000;
      timersub(&rcvhdr.tv, &tveip, &rcv_eip[i].tv);

      // метка времени каждого хронологически следующего блока ЕИП должна быть больше метки времени предыдущего
      rcv_eip[i].validity = timercmp(&rcv_eip[i+1].tv, &rcv_eip[i].tv, >);
      if(rcv_eip[i].validity == 0) {
        MoxaDevice.wData4x[PQuery[Q].offset + HAGENT_HEADER_LEN + 20]++; // eip time mark skip 3
      	break; // завершение проверки корректности данных
        }
      } // if(i==(bufptr-1))
 
    // метка времени каждого блока ЕИП должна быть больше метки времени локального заголовка
    rcv_eip[i].validity = timercmp(&rcv_eip[i].tv, &lclhdr.tv, >);
    if(rcv_eip[i].validity == 0) {
      MoxaDevice.wData4x[PQuery[Q].offset + HAGENT_HEADER_LEN + 21]++; // eip time mark skip 4
    	break; // завершение проверки корректности данных
      }
      
    // localtime_r(&rcv_eip[i].tv.tv_sec, &tmp_tm);
    // printf("usec:%d, sec:%d, min:%d, hour:%d, day:%d, month:%d, year:%d\n", 
    //   rcv_eip[i].tv.tv_usec, tmp_tm.tm_sec, tmp_tm.tm_min, tmp_tm.tm_hour, tmp_tm.tm_mday, tmp_tm.tm_mon, tmp_tm.tm_year);
      
    } // for(i=bufptr-1; i>=0; i--)

  if(rcv_eip[bufptr-1].validity == 0) { // метка времени хронологически последнего блока ЕИП не корректна
    copy_header(&rcvhdr, &lclhdr);      // все прочитанные данные игнорируются, переход к новому заголовку
    MoxaDevice.wData4x[PQuery[Q].offset + HAGENT_HEADER_LEN + 22]++; // header time mark skip 3
    continue;
    }
  
  MoxaDevice.wData4x[PQuery[Q].offset + HAGENT_HEADER_LEN + 23]++; // как минимум часть новых данных успешно прошла проверку на корректность метки времени
  copy_header(&rcvhdr, &lclhdr); // новые данные успешно прочитаны из буфера, переход к новому заголовку
//continue; /// этап 3 выполнен, очередной блок данных из буфера истории прошел проверку на корректность

  // вывод исторических параметров в консоль и в openTSDB

  for(i=0; i<bufptr; i++) if(rcv_eip[i].validity == 1) { // если текущий блок ЕИП имеет корректную метку времени

//  localtime_r(&rcv_eip[i].tv.tv_sec, &tmp_tm);
//  if(rcvhdr.msprec==1)
//         printf("%2.2d:%2.2d:%2.2d.%d ", tmp_tm.tm_hour, tmp_tm.tm_min, tmp_tm.tm_sec, (rcv_eip[i].tv.tv_usec+500) / 1000);
//    else printf("%2.2d:%2.2d:%2.2d ",    tmp_tm.tm_hour, tmp_tm.tm_min, tmp_tm.tm_sec);
    if(rcvhdr.msprec==1) // определяется значение метки времени в формате openTSDB в зависимости от точности
           sprintf(prmtms, "%d%d", rcv_eip[i].tv.tv_sec, (rcv_eip[i].tv.tv_usec + 500) / 1000);
      else sprintf(prmtms, "%d",   rcv_eip[i].tv.tv_sec);

  	for(j=0; j<lclhdr.prmnum; j++)
  		if(rcv_eip[i].prm[j] >= HB_Param[port_id-GATEWAY_T01][j].adc_min &&   // если достоверен код АЦП и
  		   rcv_eip[i].prm[j] <= HB_Param[port_id-GATEWAY_T01][j].adc_max &&   // установлен бит достоверности
  		   (lclhdr.fault & (1 << j)) == 0                                 ) { // в заголовке буфера истории

  		  // вычисляется значение наблюдаемого параметра в инженерных единицах измерения
  		  prmval = HB_Param[port_id-GATEWAY_T01][j].eng_min + rcv_eip[i].prm[j] *
  		           (HB_Param[port_id-GATEWAY_T01][j].eng_max - HB_Param[port_id-GATEWAY_T01][j].eng_min) /
  		           (HB_Param[port_id-GATEWAY_T01][j].adc_max - HB_Param[port_id-GATEWAY_T01][j].adc_min) ;

//    	  printf("%s:%2.2f, ", HB_Param[port_id-GATEWAY_T01][j].metric, prmval);

        sprintf(putstr, "put %s %s %2.2f %s\n",
        	HB_Param[port_id-GATEWAY_T01][j].metric,
        	prmtms,
        	prmval,
        	HB_Param[port_id-GATEWAY_T01][j].tags);

        // проверка готовности tcp-соединения для связи с TSD
        if(tcp_master->ethernet.status2==TCPMSTCON_ESTABLISHED) {

//        printf("%s", putstr);
          status = mbcom_tcp_send(tcp_master->ethernet.csd2, putstr, strlen(putstr));

          if(status != MBCOM_OK) {
				    shutdown(tcp_master->ethernet.csd2, SHUT_RDWR);
				    close(tcp_master->ethernet.csd2);
				    tcp_master->ethernet.csd2 =-1;
				    tcp_master->ethernet.status2 =TCPMSTCON_NOTMADE;
				           MoxaDevice.wData4x[PQuery[Q].offset + HAGENT_HEADER_LEN + 24]++; // операция записи завершилась ошибкой
            } else MoxaDevice.wData4x[PQuery[Q].offset + HAGENT_HEADER_LEN + 25]++; // количество успешных операций записи
          } else   MoxaDevice.wData4x[PQuery[Q].offset + HAGENT_HEADER_LEN + 26]++; // ошибка. сетевое подключение не готово

        } else {
        	         MoxaDevice.wData4x[PQuery[Q].offset + HAGENT_HEADER_LEN + 27]++; // параметр истории не достоверен
//        	printf("%s:  NaN, ", HB_Param[port_id-GATEWAY_T01][j].metric);
          }
//    printf("\n");
    } // цикл по блокам ЕИП

	//tcp_master->stat.proc_time=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
		///!!! статистику обновляем централизованно в функции int refresh_shm(void *arg) или подобной
	//gate502.wData4x[gate502.status_info+3*port_id+2]=tcp_master->stat.request_time_average;

	} // while (1)
	} // конфигурация корректна

	EndRun: ;

	tcp_master->rc=1;
	// THREAD STOPPED
 	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|GATEWAY_LANTCP, IFACE_THREAD_STOPPED, port_id, client_id, 0, 0);
	pthread_exit (0);	
}

///-----------------------------------------------------------------------------------------------------------------
int reset_tcpmaster_conn(GW_Iface *tcp_master, int connum)
	{

	int csd, status;
	
	struct sockaddr_in server;
	
	struct timeval tv;
	int optlen=sizeof(tv);

	int res=0;

  int optval = 1;
  
	csd=    connum==1? tcp_master->ethernet.csd :    tcp_master->ethernet.csd2;
	status= connum==1? tcp_master->ethernet.status : tcp_master->ethernet.status2;
	
	// Определяем семейство протоколов
	server.sin_family = AF_INET;

	// определяем IP-адрес сервера
	//server.sin_addr.s_addr = 0x0a00006f; // 10.0.0.240
  //server.sin_addr.s_addr = 0xc00000fc; // 192.0.0.252
	server.sin_addr.s_addr = connum==1? htonl(tcp_master->ethernet.ip) : htonl(tcp_master->ethernet.ip2);

	// Определяем порт сервера
	server.sin_port =  connum==1? htons(tcp_master->ethernet.port) : htons(tcp_master->ethernet.port2);

	if(status==TCPMSTCON_NOTMADE) {

		csd = socket(AF_INET, SOCK_STREAM, 0);
	
		if(csd < 0) {
			perror("tcp_master socket");
			return 1;
			}
	
		tv.tv_sec = tcp_master->ethernet.timeout / 1000000;
		tv.tv_usec= tcp_master->ethernet.timeout % 1000000;
		
		// устанавливаем значение таймаута на операции записи и чтения для сокета
		if(setsockopt(csd, SOL_SOCKET, SO_SNDTIMEO, &tv, optlen)!=0) res++;
		if(setsockopt(csd, SOL_SOCKET, SO_RCVTIMEO, &tv, optlen)!=0) res++;
		
    /* Set the KEEPALIVE option active */
    optlen = sizeof(optval);
    if(setsockopt(csd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen)!=0) res++;

    optval = 2; // the number of unacknowledged probes to send before considering the connection dead and notifying the application layer
    if(setsockopt(csd, SOL_TCP, TCP_KEEPCNT, &optval, optlen)!=0) res++;

    optval = TCP_RECONN_INTVL/1000000; // the interval between the last data packet sent and the first keepalive probe
    if(setsockopt(csd, SOL_TCP, TCP_KEEPIDLE, &optval, optlen)!=0) res++;

    optval = TCP_RECONN_INTVL/1000000; // the interval between subsequential keepalive probes
    if(setsockopt(csd, SOL_TCP, TCP_KEEPINTVL, &optval, optlen)!=0) res++;

		if(res!=0) { // SOCKET NOT INITIALIZED
			sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_LANTCP, TCPCON_INITIALIZED, 0, 0, 0, tcp_master->queue.port_id);
			return 3;
		  }

		// для обеспечения немедленного возврата из функции connect()
		fcntl(csd, F_SETFL, fcntl(csd, F_GETFL, 0) | O_NONBLOCK);

		status=TCPMSTCON_INPROGRESS;
		if(connum==1) {
			tcp_master->ethernet.csd=csd;
			tcp_master->ethernet.status=TCPMSTCON_INPROGRESS;
			} else {
			tcp_master->ethernet.csd2=csd;
			tcp_master->ethernet.status2=TCPMSTCON_INPROGRESS;
			}
	  }

	if (status==TCPMSTCON_INPROGRESS) {
		if(connect(csd, (struct sockaddr *)&server, sizeof(server))==-1) {
			// perror("perror: ");
			// CONNECTION FAILED
		 	// sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_LANTCP, TCPCON_FAILED, server.sin_addr.s_addr, 0, 0, tcp_master->queue.port_id);
			return 2;
			} 

		// CONNECTION ESTABLISHED
		sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|GATEWAY_LANTCP, TCPCON_ESTABLISHED, server.sin_addr.s_addr, 0, 0, tcp_master->queue.port_id);
	
		// для обеспечения нормальной работы потоковой функции TCP-интерфейса
		fcntl(csd, F_SETFL, fcntl(csd, F_GETFL, 0) & (~O_NONBLOCK));

		if(connum==1) {
			tcp_master->ethernet.status=TCPMSTCON_ESTABLISHED;
			time(&tcp_master->ethernet.connection_time);
			} else {
			tcp_master->ethernet.status2=TCPMSTCON_ESTABLISHED;
			time(&tcp_master->ethernet.connection_time2);
			}
    }

	return 0;
	}
///-----------------------------------------------------------------------------------------------------------------
int check_tcpmaster_conn(GW_Iface *tcp_master, int connum)
	{
	int csd, res, buf;

	csd = connum==1? tcp_master->ethernet.csd : tcp_master->ethernet.csd2;
	
	res = recv(csd, &buf, 1, MSG_PEEK | MSG_DONTWAIT);

  // возможны следующие ситуации при вызове функции recv в нашей ситуации:
  // соединение установлено, данные есть:	res > 0
  // таймаут при попытке чтения:					res==-1, errno==EAGAIN (11)
  // соединение разорвано:								res==-1, errno==ENOTCONN (107)
  // соединение закрыто сервером:					res==0

	if( (res==0) || ((res==-1)&&(errno!=EAGAIN)) ) {
		shutdown(csd, SHUT_RDWR);
		close(csd);
		if(connum==1) {
			tcp_master->ethernet.csd=-1;
			tcp_master->ethernet.status=TCPMSTCON_NOTMADE;
			} else {
				tcp_master->ethernet.csd2=-1;
				tcp_master->ethernet.status2=TCPMSTCON_NOTMADE;
				}
	  sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|GATEWAY_LANTCP, (res==0?TCPCON_CLOSED_REMSD:TCPCON_CLOSED_KPALV), 
	    (connum==1? tcp_master->ethernet.ip : tcp_master->ethernet.ip2), 0, 0, tcp_master->queue.port_id);

		return TCPMSTCON_NOTMADE;
	  }
	
	return TCPMSTCON_ESTABLISHED;
	}
///-----------------------------------------------------------------------------------------------------------------
int do_data_exchange(GW_Iface *tcp_master, int port_id, int *timeout_counter,
	                   u8 *req_adu, u16  req_adu_len,
	                   u8 *rsp_adu, u16 *rsp_adu_len)
  {
	int status;
	u16 adu_len;
  	
		if(tcp_master->ethernet.status != TCPMSTCON_ESTABLISHED) return 1;

		tcp_master->stat.accepted++;

		make_tcp_adu(req_adu, req_adu_len-TCPADU_ADDRESS);

		if(Security.show_data_flow==1)
			show_traffic(TRAFFIC_TCP_SEND, port_id, GW_CLIENT_MOXAGATE, req_adu, req_adu_len);

		status = mbcom_tcp_send(tcp_master->ethernet.csd, req_adu, req_adu_len);

		switch(status) {
		  case 0:
		  	break;
		  case TCP_COM_ERR_SEND:

				tcp_master->stat.errors++;

			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_WRN|port_id, POLL_TCP_SEND, (unsigned) status, GW_CLIENT_MOXAGATE, 0, 0);

				shutdown(tcp_master->ethernet.csd, SHUT_RDWR);
				close(tcp_master->ethernet.csd);
				tcp_master->ethernet.csd =-1;
				tcp_master->ethernet.status =TCPMSTCON_NOTMADE;

				//if(client_id==GW_CLIENT_MOXAGATE) {}
				//PQuery[Q].status_bit=0;

				return 2;
		  	break;

		  default:;
		  };

		status = mbcom_tcp_recv(tcp_master->ethernet.csd, rsp_adu, &adu_len);
		*rsp_adu_len=adu_len;
		
		if(Security.show_data_flow==1)
			show_traffic(TRAFFIC_TCP_RECV, port_id, GW_CLIENT_MOXAGATE, rsp_adu, *rsp_adu_len);

		switch(status) {
		  case 0:
		  	break;

		  case TCP_COM_ERR_NULL:
		  case TCP_COM_ERR_TIMEOUT:
		  	
		  	*timeout_counter++;
		  	if((status==TCP_COM_ERR_NULL) || (*timeout_counter>=4)) {
					shutdown(tcp_master->ethernet.csd, SHUT_RDWR);
					close(tcp_master->ethernet.csd);
					tcp_master->ethernet.csd =-1;
					tcp_master->ethernet.status =TCPMSTCON_NOTMADE;

					*timeout_counter=0;

				  sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|GATEWAY_LANTCP, (status==TCP_COM_ERR_NULL?TCPCON_CLOSED_REMSD:TCPCON_CLOSED_TMOUT), 
				    tcp_master->ethernet.ip, 0, 0, 0);

		  	  }
		  	
		  case TCP_ADU_ERR_MIN:
		  case TCP_ADU_ERR_MAX:
		  case TCP_ADU_ERR_PROTOCOL:
		  case TCP_ADU_ERR_LEN:
		  case TCP_ADU_ERR_UID:
		  case TCP_PDU_ERR:

				tcp_master->stat.errors++;

			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_WRN|port_id, POLL_TCP_RECV, (unsigned) status, GW_CLIENT_MOXAGATE, 0, 0);

				return 3;
		  	break;

		  default:;
		  };
			 
  *timeout_counter=0;

	if((rsp_adu[TCPADU_FUNCTION]&0x80)>0) { // получено исключение
		tcp_master->stat.errors++;
		return 4;
	  }
	tcp_master->stat.sended++;

  return 0;
	}
///-----------------------------------------------------------------------------------------------------------------
