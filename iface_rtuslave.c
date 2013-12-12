/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///*********************** ћќƒ”Ћ№ »Ќ“≈–‘≈…—ќ¬ ЎЋё«ј ****************************
///*** ћ≈’јЌ»«ћ ќЅ—Ћ”∆»¬јЌ»я MODBUS RTU SLAVE

///=== INTERFACES_H MODULE IMPLEMENTATION

#include <pthread.h>

#include "interfaces.h"
#include "moxagate.h"
#include "messages.h"
#include "modbus.h"

///-----------------------------------------------------------------------------------------------------------------
void *iface_rtu_slave(void *arg)
  {
	u8  req_adu[MB_UNIVERSAL_ADU_LEN];
	u16 req_adu_len;
	u8  rsp_adu[MB_UNIVERSAL_ADU_LEN];
	u16 rsp_adu_len;

	int status;

  int port_id=((unsigned)arg)>>8;
  int client_id=((unsigned)arg)&0xff;

	GW_Iface	*rtu_slave;

	struct timeval tv1, tv2;
	struct timezone tz;

	rtu_slave = &IfaceRTU[port_id];

	//rtu_slave->clients[client_id].stat.request_time_min=10000; // 10 seconds, must be "this->serial.timeout"
	//rtu_slave->clients[client_id].stat.request_time_max=0;
	//rtu_slave->clients[client_id].stat.request_time_average=0;
	//for(i=0; i<MAX_LATENCY_HISTORY_POINTS; i++) rtu_slave->clients[client_id].stat.latency_history[i]=1000; //ms
	//rtu_slave->clients[client_id].stat.clp=0;

///-----------------------------------------
  // THREAD STARTED
	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|port_id, 42, IfaceRTU[port_id].modbus_mode, client_id, 0, 0);

	while (1) {
		
///-----------------------------------

	  status = mbcom_rtu_recv_req(rtu_slave->serial.fd,
	  														&req_adu[TCPADU_ADDRESS],
	  														&req_adu_len);

		rtu_slave->stat.accepted++;
		Client[client_id].stat.accepted++;

		if(Security.show_data_flow==1)
			show_traffic(TRAFFIC_RTU_RECV, port_id, client_id, &req_adu[TCPADU_ADDRESS], req_adu_len);

		// приводим в соответствие TCP ADU
		req_adu_len-=MB_SERIAL_CRC_LEN;
		make_tcp_adu(req_adu, req_adu_len);
		req_adu_len+=TCPADU_ADDRESS;

//		gettimeofday(&tv2, &tz);
//		Client[client_id].stat.scan_rate=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
//		if(Client[client_id].stat.scan_rate>MB_SCAN_RATE_INFINITE)
//		  Client[client_id].stat.scan_rate=MB_SCAN_RATE_INFINITE;
//	rtu_slave->clients[client_id].stat.request_time_average=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
//	printf("cycle begins after %d msec\n", rtu_slave->clients[client_id].stat.request_time_average);
		gettimeofday(&tv1, &tz);

//	  pthread_mutex_unlock(&rtu_slave->serial_mutex);
	  
		switch(status) {
		  case 0:
		  	break;

		  case MB_SERIAL_READ_FAILURE:
		  case MB_SERIAL_COM_TIMEOUT:
		  case MB_SERIAL_ADU_ERR_MIN:
		  case MB_SERIAL_ADU_ERR_MAX:
		  case MB_SERIAL_CRC_ERROR:
		  case MB_SERIAL_PDU_ERR:

				rtu_slave->stat.errors++;
				Client[client_id].stat.errors++;
				func_res_err(req_adu[TCPADU_FUNCTION], &rtu_slave->stat);
				func_res_err(req_adu[TCPADU_FUNCTION], &Client[client_id].stat);
				stage_to_stat((MBCOM_REQ<<16) | (MBCOM_RTU_RECV<<8) | status, &rtu_slave->stat);
				stage_to_stat((MBCOM_REQ<<16) | (MBCOM_RTU_RECV<<8) | status, &Client[client_id].stat);

			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_WRN|port_id, POLL_RTU_RECV, (unsigned) status, client_id, 0, 0);

				if(status==MB_SERIAL_READ_FAILURE) {
					rtu_slave->modbus_mode=IFACE_ERROR;
					goto EndRun;
					}
				continue;
		  	break;

		  default:;
		  };

///###-----------------------------			
		// дл€ обеспечени€ нормальной работы устройств св€зи устанавливаем
		// выдержку времени перед отправкой ответа на запрос
		usleep(rtu_slave->serial.timeout);
///###-----------------------------			

			status=forward_query(client_id, req_adu, req_adu_len);

			if((status & FRWD_RESULT_MASK)!=FRWD_RESULT_OK ) { // запрос завершилс€ ошибкой

				rtu_slave->stat.errors++;
				Client[client_id].stat.errors++;
				func_res_err(req_adu[TCPADU_FUNCTION], &rtu_slave->stat);
				func_res_err(req_adu[TCPADU_FUNCTION], &Client[client_id].stat);
				stage_to_stat((MBCOM_REQ<<16) | (MBCOM_FRWD_REQ<<8) | status, &rtu_slave->stat);
				stage_to_stat((MBCOM_REQ<<16) | (MBCOM_FRWD_REQ<<8) | status, &Client[client_id].stat);

				continue;
			  }
			  
			if((status & FRWD_TYPE_MASK)  !=FRWD_TYPE_PROXY) { // запрос был перенаправлен

				switch(status&FRWD_TYPE_MASK) {
	
					case FRWD_TYPE_REGISTER: 
						rtu_slave->stat.frwd_r++;
						Client[client_id].stat.frwd_r++;
						break;

					case FRWD_TYPE_ADDRESS:    
						rtu_slave->stat.frwd_a++;
						Client[client_id].stat.frwd_a++;
						break;

					default:;
					}

				func_res_ok(req_adu[TCPADU_FUNCTION], &rtu_slave->stat);
				//func_res_ok(req_adu[TCPADU_FUNCTION], &Client[client_id].stat); // запрос пошел дальше

				continue;
			  }

			// если запрос адресуетс€ к внутренней пам€ти шлюза, то обрабатываем его
			status = process_moxamb_request(client_id, req_adu, req_adu_len, rsp_adu, &rsp_adu_len);

			if(status!=0) { // запрос был перенаправлен на другой порт

				if(status!=3) { // учет ошибочных запросов
					rtu_slave->stat.errors++;
					Client[client_id].stat.errors++;
					func_res_err(req_adu[TCPADU_FUNCTION], &rtu_slave->stat);
					func_res_err(req_adu[TCPADU_FUNCTION], &Client[client_id].stat);
					stage_to_stat((MBCOM_REQ<<16) | (MBCOM_FRWD_REQ<<8) | FRWD_TYPE_PROXY, &rtu_slave->stat);
					stage_to_stat((MBCOM_REQ<<16) | (MBCOM_FRWD_REQ<<8) | FRWD_TYPE_PROXY, &Client[client_id].stat);
					
				 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_WRN|port_id, FRWD_TRANS_PQUERY, client_id, (unsigned) status, 0, 0);
					continue;
					}

				rtu_slave->stat.frwd_p++;
				Client[client_id].stat.frwd_p++;
				func_res_ok(req_adu[TCPADU_FUNCTION], &rtu_slave->stat);
				//func_res_ok(req_adu[TCPADU_FUNCTION], &Client[client_id].stat); // запрос пошел дальше
				continue;
				}

//		gettimeofday(&tv1, &tz);

///###-----------------------------			

		rsp_adu_len-=TCPADU_ADDRESS;

		status = crc(&rsp_adu[TCPADU_ADDRESS], 0, rsp_adu_len);
		rsp_adu[TCPADU_ADDRESS+rsp_adu_len+0] = status >> 8;
		rsp_adu[TCPADU_ADDRESS+rsp_adu_len+1] = status & 0x00FF;
		rsp_adu_len+=MB_SERIAL_CRC_LEN;

		if(Security.show_data_flow==1)
			show_traffic(TRAFFIC_RTU_SEND, port_id, client_id, &rsp_adu[TCPADU_ADDRESS], rsp_adu_len);

		status = mbcom_rtu_send(rtu_slave->serial.fd,
														&rsp_adu[TCPADU_ADDRESS],
														rsp_adu_len);

		switch(status) {
		  case 0:
				rtu_slave->stat.sended++;
				Client[client_id].stat.sended++;
				func_res_ok(rsp_adu[TCPADU_FUNCTION], &rtu_slave->stat);
				func_res_ok(rsp_adu[TCPADU_FUNCTION], &Client[client_id].stat);
		  	break;

		  case MB_SERIAL_WRITE_ERR:

				rtu_slave->stat.errors++;
				Client[client_id].stat.errors++;
				func_res_err(rsp_adu[TCPADU_FUNCTION], &rtu_slave->stat);
				func_res_err(rsp_adu[TCPADU_FUNCTION], &Client[client_id].stat);
				stage_to_stat((MBCOM_RSP<<16) | (MBCOM_RTU_SEND<<8) | status, &rtu_slave->stat);
				stage_to_stat((MBCOM_RSP<<16) | (MBCOM_RTU_SEND<<8) | status, &Client[client_id].stat);

			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_WRN|port_id, POLL_RTU_SEND, (unsigned) status, client_id, 0, 0);

				continue;
		  	break;

		  default:;
		  };

	gettimeofday(&tv2, &tz);

	//rtu_slave->clients[client_id].stat.request_time_average=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
	//rtu_slave->clients[client_id].stat.latency_history[rtu_slave->clients[client_id].stat.clp]=rtu_slave->clients[client_id].stat.request_time_average;
	//rtu_slave->clients[client_id].stat.clp=rtu_slave->clients[client_id].stat.clp<MAX_LATENCY_HISTORY_POINTS?rtu_slave->clients[client_id].stat.clp+1:0;

	//if(rtu_slave->clients[client_id].stat.request_time_min>rtu_slave->clients[client_id].stat.request_time_average)
	//  rtu_slave->clients[client_id].stat.request_time_min=rtu_slave->clients[client_id].stat.request_time_average;
	//if(rtu_slave->clients[client_id].stat.request_time_max<rtu_slave->clients[client_id].stat.request_time_average)
	//  rtu_slave->clients[client_id].stat.request_time_max=rtu_slave->clients[client_id].stat.request_time_average;

	//rtu_slave->clients[client_id].stat.request_time_average=0;
	//for(i=0; i<MAX_LATENCY_HISTORY_POINTS; i++)
	//  rtu_slave->clients[client_id].stat.request_time_average+=rtu_slave->clients[client_id].stat.latency_history[i];
	//rtu_slave->clients[client_id].stat.request_time_average/=MAX_LATENCY_HISTORY_POINTS;
	
//	printf("%d\n", rtu_slave->stat.request_time_average);
//	printf("%d:%d\n", tv1.tv_sec, tv1.tv_usec);
//	printf("%d:%d\n", tv2.tv_sec, tv2.tv_usec);
//	printf("noop\n");
//  usleep(100000);

//	gettimeofday(&tv2, &tz);
//	rtu_slave->clients[client_id].stat.request_time_average=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
//	printf("cycle ended after %d msec\n", rtu_slave->clients[client_id].stat.request_time_average);
	}

	EndRun: ;
	// THREAD STOPPED
 	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|port_id, 43, 0, 0, 0, 0);
  clear_client(client_id);
	pthread_exit (0);	
}
