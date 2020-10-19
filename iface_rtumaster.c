/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///*********************** МОДУЛЬ ИНТЕРФЕЙСОВ ШЛЮЗА ****************************
///*** МЕХАНИЗМ ОПРОСА MODBUS RTU MASTER

///=== INTERFACES_H MODULE IMPLEMENTATION

#include "interfaces.h"
#include "moxagate.h"
#include "messages.h"
#include "modbus.h"

///-----------------------------------------------------------------------------------------------------------------
void *iface_rtu_master(void *arg)
  {
	u8  req_adu[MB_UNIVERSAL_ADU_LEN];
	u16 req_adu_len;
	u8  rsp_adu[MB_UNIVERSAL_ADU_LEN];
	u16 rsp_adu_len;

  int port_id=((long)arg)>>8;
  int client_id, device_id;

	int status;
	unsigned i, j, Q;

	GW_Iface	*rtu_master;
	struct timeval tv1, tv2;
	struct timezone tz;

	int queue_has_query;

	rtu_master = &IfaceRTU[port_id];
	
///!!!
//	rtu_master->clients[client_id].stat.request_time_min=10000; // 10 seconds, must be "this->serial.timeout"
//	rtu_master->clients[client_id].stat.request_time_max=0;
//	rtu_master->clients[client_id].stat.request_time_average=0;
//	for(i=0; i<MAX_LATENCY_HISTORY_POINTS; i++) rtu_master->clients[client_id].stat.latency_history[i]=1000; //ms
//	rtu_master->clients[client_id].stat.clp=0;

	/// semaphore
	rtu_master->queue.operations[0].sem_op=-1;
	semop(semaphore_id, rtu_master->queue.operations, 1);
	rtu_master->queue.operations[0].sem_flg=IPC_NOWAIT;

  // THREAD STARTED
	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|port_id, IFACE_THREAD_STARTED, rtu_master->modbus_mode, 0, 0, 0);
///-----------------------------------------

	while (1) for(i=0; i<=rtu_master->PQueryIndex[MAX_QUERY_ENTRIES]; i++) {

		Q=rtu_master->PQueryIndex[i];
		j=i;

		status=get_query_from_queue(&rtu_master->queue, &client_id, &device_id, req_adu, &req_adu_len);
		
		if(status==1) { // внутренняя ошибка в программе
			rtu_master->stat.accepted++;
			rtu_master->stat.errors++;
			rtu_master->stat.frwd_queue_out++;
		  }

		queue_has_query=status==-1?0:1;
		if(queue_has_query==1) i=rtu_master->PQueryIndex[MAX_QUERY_ENTRIES];

		// эта итерация цикла сканирования нужна для проверки очереди порта при пустой таблице опроса
		if( ((queue_has_query!=1) || (status==1)) &&
		     (i==rtu_master->PQueryIndex[MAX_QUERY_ENTRIES])) continue;

	if(queue_has_query!=1) {//формируем следующий запрос из таблицы опроса

		if(PQuery[Q].delay!=0) usleep(PQuery[Q].delay*1000);

		create_proxy_request(Q, req_adu, &req_adu_len);
		client_id=GW_CLIENT_MOXAGATE;

		rtu_master->Security.stat.accepted++;

		} else { //обрабатываем запрос из очереди последовательного порта

			i=j; // после обработки запроса из очереди будет обработан прерванный запрос из таблицы опроса

			prepare_request (device_id, req_adu, req_adu_len);

			}

	  ///---------------------
		rtu_master->stat.accepted++;
		gettimeofday(&tv1, &tz);

		req_adu_len-=TCPADU_ADDRESS;

		status = crc(&req_adu[TCPADU_ADDRESS], 0, req_adu_len);
		req_adu[TCPADU_ADDRESS+req_adu_len+0] = status >> 8;
		req_adu[TCPADU_ADDRESS+req_adu_len+1] = status & 0x00FF;
		req_adu_len+=MB_SERIAL_CRC_LEN;

		if(Security.show_data_flow==1)
			show_traffic(TRAFFIC_RTU_SEND, port_id, client_id, &req_adu[TCPADU_ADDRESS], req_adu_len);

//  gettimeofday(&tv1, NULL);
//  printf("send %lu; ", tv1.tv_sec*1000 + tv1.tv_usec/1000);

		status = mbcom_rtu_send(rtu_master->serial.fd,
														&req_adu[TCPADU_ADDRESS],
														req_adu_len);

		switch(status) {
		  case 0:
		  	break;

		  case MB_SERIAL_WRITE_ERR:

				rtu_master->stat.errors++;
				func_res_err(req_adu[TCPADU_FUNCTION], &rtu_master->stat);
				stage_to_stat((MBCOM_REQ<<16) | (MBCOM_RTU_SEND<<8) | status, &rtu_master->stat);

			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_WRN|port_id, POLL_RTU_SEND, (unsigned) status, client_id, 0, 0);

				if(client_id==GW_CLIENT_MOXAGATE) {

					rtu_master->Security.stat.errors++;
					func_res_err(req_adu[TCPADU_FUNCTION], &rtu_master->Security.stat);
					stage_to_stat((MBCOM_REQ<<16) | (MBCOM_RTU_SEND<<8) | status, &rtu_master->Security.stat);

					PQuery[Q].err_counter++;
					if(PQuery[Q].err_counter >= PQuery[Q].critical) {
						PQuery[Q].status_bit=0;
            VSlave[device_id&0xff].status_bit=0;
					  }
					///!!! добавить сообщение о пропадании связи с modbus-rtu сервером

				  } else if(client_id==GW_CLIENT_KM400) {

            VSlave[device_id&0xff].err_counter++;
            if(VSlave[device_id&0xff].err_counter >= VSlave[device_id&0xff].critical)
              VSlave[device_id&0xff].status_bit=0;

				  } else {
						Client[client_id].stat.errors++;
						func_res_err(req_adu[TCPADU_FUNCTION], &Client[client_id].stat);
						stage_to_stat((MBCOM_REQ<<16) | (MBCOM_RTU_SEND<<8) | status, &Client[client_id].stat);
				    }
				continue;
		  	break;

		  default:;
		  };

    rsp_adu_len=modbus_response_lenght(req_adu, req_adu_len);
    
//  gettimeofday(&tv1, NULL);
//  printf("recv %lu; ", tv1.tv_sec*1000 + tv1.tv_usec/1000);

	  status = mbcom_rtu_recv_rsp(rtu_master->serial.fd,
	  														&rsp_adu[TCPADU_ADDRESS],
	  														&rsp_adu_len,
	  														rtu_master->serial.timeout,
	  														rtu_master->serial.ch_interval_timeout);

//  gettimeofday(&tv1, NULL);
//  printf("proc %lu; ms\n", tv1.tv_sec*1000 + tv1.tv_usec/1000);

		if(Security.show_data_flow==1)
			show_traffic(TRAFFIC_RTU_RECV, port_id, client_id, &rsp_adu[6], rsp_adu_len);

		switch(status) {
		  case 0:
		  	break;
		  case MB_SERIAL_COM_TIMEOUT:
		  case MB_SERIAL_ADU_ERR_MIN:
		  case MB_SERIAL_ADU_ERR_MAX:
		  case MB_SERIAL_CRC_ERROR:
		  case MB_SERIAL_PDU_ERR:
		  case MB_SERIAL_READ_FAILURE:

				rtu_master->stat.errors++;
				func_res_err(rsp_adu[TCPADU_FUNCTION], &rtu_master->stat);
				stage_to_stat((MBCOM_RSP<<16) | (MBCOM_RTU_RECV<<8) | status, &rtu_master->stat);

			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_WRN|port_id, POLL_RTU_RECV, (unsigned) status, client_id, 0, 0);

				if(client_id==GW_CLIENT_MOXAGATE) {
					rtu_master->Security.stat.errors++;
					func_res_err(rsp_adu[TCPADU_FUNCTION], &rtu_master->Security.stat);
					stage_to_stat((MBCOM_RSP<<16) | (MBCOM_RTU_RECV<<8) | status, &rtu_master->Security.stat);

					PQuery[Q].err_counter++;
					if(PQuery[Q].err_counter >= PQuery[Q].critical) {
						PQuery[Q].status_bit=0;
            VSlave[device_id&0xff].status_bit=0;
					  }
					///!!! добавить сообщение о пропадании связи с modbus-rtu сервером

				  } else if(client_id==GW_CLIENT_KM400) {

            VSlave[device_id&0xff].err_counter++;
            if(VSlave[device_id&0xff].err_counter >= VSlave[device_id&0xff].critical)
              VSlave[device_id&0xff].status_bit=0;

				  } else {
						Client[client_id].stat.errors++;
						func_res_err(rsp_adu[TCPADU_FUNCTION], &Client[client_id].stat);
						stage_to_stat((MBCOM_RSP<<16) | (MBCOM_RTU_RECV<<8) | status, &Client[client_id].stat);
				    }
				continue;
		  	break;
		  
		  default:;
		  };

///###-----------------------------			
	rsp_adu_len-=MB_SERIAL_CRC_LEN; // получаем и сохраняем длину PDU+1

	if(client_id==GW_CLIENT_MOXAGATE) { // сохраняем локально полученные данные

#ifdef MOXA7GATE_KM400
    pthread_mutex_lock(&rtu_master->serial_mutex);
#endif
    process_proxy_response(Q, rsp_adu, rsp_adu_len+MB_TCP_ADU_HEADER_LEN-1);
#ifdef MOXA7GATE_KM400
    pthread_mutex_unlock(&rtu_master->serial_mutex);
#endif

		rtu_master->stat.sended++;
		rtu_master->Security.stat.sended++;
		func_res_ok(rsp_adu[TCPADU_FUNCTION], &rtu_master->stat);
		func_res_ok(rsp_adu[TCPADU_FUNCTION], &rtu_master->Security.stat);

		} else if(client_id==GW_CLIENT_RTU_SLV) { //отправляем результат запроса непосредственно клиенту

		// функция преобразования ответа в соответствии с контекстом
		prepare_response(device_id, rsp_adu, rsp_adu_len);
		forward_response(port_id, client_id, req_adu, req_adu_len, rsp_adu, rsp_adu_len);

		} else if(client_id==GW_CLIENT_KM400) {

      // отдельный бит статуса связи для операции записи в КМ-400
      VSlave[device_id&0xff].status_bit=1;
      VSlave[device_id&0xff].err_counter=0;

		}

//	gettimeofday(&tv2, &tz);
//	rtu_master->stat.request_time=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
	}
	EndRun: ;
	// THREAD STOPPED
 	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|port_id, 43, 0, 0, 0, 0);
	pthread_exit (0);	
}
