/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///*********************** ÌÎÄÓËÜ ÈÍÒÅÐÔÅÉÑÎÂ ØËÞÇÀ ****************************
///*** ÌÅÕÀÍÈÇÌ ÏÅÐÅÍÀÏÐÀÂËÅÍÈß GATEWAY_SIMPLE

///=== INTERFACES_H MODULE IMPLEMENTATION

#include "messages.h"
#include "interfaces.h"
#include "moxagate.h"
#include "modbus.h"

///----------------------------------------------------------------------------
void *iface_tcp_server(void *arg)
  {
	u8  req_adu[MB_UNIVERSAL_ADU_LEN];
	u16 req_adu_len;
	u8  rsp_adu[MB_UNIVERSAL_ADU_LEN];
	u16 rsp_adu_len;

	int status;

  int port_id=((unsigned)arg)>>8;
  int client_id=((unsigned)arg)&0xff;

	GW_Iface	*tcp_server;

	struct timeval tv1, tv2;
	struct timezone tz;

	tcp_server = &IfaceRTU[port_id];
	
	//tcp_server->clients[client_id].stat.request_time_min=10000; // 10 seconds, must be "this->serial.timeout"
	//tcp_server->clients[client_id].stat.request_time_max=0;
	//tcp_server->clients[client_id].stat.request_time_average=0;
	//for(i=0; i<MAX_LATENCY_HISTORY_POINTS; i++) tcp_server->clients[client_id].stat.latency_history[i]=1000; //ms
	//tcp_server->clients[client_id].stat.clp=0;

  // THREAD STARTED
	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|port_id, IFACE_THREAD_STARTED, tcp_server->modbus_mode, client_id, 0, 0);

	while (1) {

		status = mbcom_tcp_recv(Client[client_id].csd,
														req_adu,
														&req_adu_len);

		tcp_server->stat.accepted++;
		Client[client_id].stat.accepted++;

		if(Security.show_data_flow==1)
			show_traffic(TRAFFIC_TCP_RECV, port_id, client_id, req_adu, req_adu_len);

//		gettimeofday(&tv2, &tz);
//		Client[client_id].stat.scan_rate=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
//		if(Client[client_id].stat.scan_rate>MB_SCAN_RATE_INFINITE)
//		  Client[client_id].stat.scan_rate=MB_SCAN_RATE_INFINITE;
//		gettimeofday(&tv1, &tz);

		switch(status) {
		  case 0:
		  	break;
		  case TCP_COM_ERR_NULL:
		  case TCP_ADU_ERR_MIN:
		  case TCP_ADU_ERR_MAX:
		  case TCP_ADU_ERR_PROTOCOL:
		  case TCP_ADU_ERR_LEN:
		  case TCP_ADU_ERR_UID:
		  case TCP_PDU_ERR:

				tcp_server->stat.errors++;
				Client[client_id].stat.errors++;
				func_res_err(req_adu[TCPADU_FUNCTION], &tcp_server->stat);
				func_res_err(req_adu[TCPADU_FUNCTION], &Client[client_id].stat);
				stage_to_stat((MBCOM_REQ<<16) | (MBCOM_TCP_RECV<<8) | status, &tcp_server->stat);
				stage_to_stat((MBCOM_REQ<<16) | (MBCOM_TCP_RECV<<8) | status, &Client[client_id].stat);

			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_WRN|port_id, POLL_TCP_RECV, (unsigned) status, client_id, 0, 0);

				if(status==TCP_COM_ERR_NULL) {
					// tcp_server->modbus_mode=MODBUS_PORT_ERROR; /// íåëüçÿ òàê äåëàòü
					goto EndRun;
					}
				continue;
		  	break;
		  default:;
		  };
		
	  pthread_mutex_lock(&tcp_server->serial_mutex);

		req_adu_len-=TCPADU_ADDRESS;

		status = crc(&req_adu[TCPADU_ADDRESS], 0, req_adu_len);
		req_adu[TCPADU_ADDRESS+req_adu_len+0] = status >> 8;
		req_adu[TCPADU_ADDRESS+req_adu_len+1] = status & 0x00FF;
		req_adu_len+=MB_SERIAL_CRC_LEN;

		if(Security.show_data_flow==1)
			show_traffic(TRAFFIC_RTU_SEND, port_id, client_id, &req_adu[TCPADU_ADDRESS], req_adu_len);

		status = mbcom_rtu_send(tcp_server->serial.fd,
														&req_adu[TCPADU_ADDRESS],
														req_adu_len);

		switch(status) {
		  case 0:
		  	break;
		  case MB_SERIAL_WRITE_ERR:

				tcp_server->stat.errors++;
				Client[client_id].stat.errors++;
				func_res_err(req_adu[TCPADU_FUNCTION], &tcp_server->stat);
				func_res_err(req_adu[TCPADU_FUNCTION], &Client[client_id].stat);
				stage_to_stat((MBCOM_REQ<<16) | (MBCOM_RTU_SEND<<8) | status, &tcp_server->stat);
				stage_to_stat((MBCOM_REQ<<16) | (MBCOM_RTU_SEND<<8) | status, &Client[client_id].stat);

			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_WRN|port_id, POLL_RTU_SEND, (unsigned) status, client_id, 0, 0);

				pthread_mutex_unlock(&tcp_server->serial_mutex);
				continue;
		  	break;
		  default:;
		  };

		status = mbcom_rtu_recv_rsp(tcp_server->serial.fd,
																&rsp_adu[TCPADU_ADDRESS],
																&rsp_adu_len,
																tcp_server->serial.timeout,
																tcp_server->serial.ch_interval_timeout);

	  pthread_mutex_unlock(&tcp_server->serial_mutex);
	  
		if(Security.show_data_flow==1)
			show_traffic(TRAFFIC_RTU_RECV, port_id, client_id, &rsp_adu[TCPADU_ADDRESS], rsp_adu_len);

		switch(status) {
		  case 0:
		  	break;
		  case MB_SERIAL_READ_FAILURE:
		  case MB_SERIAL_COM_TIMEOUT:
		  case MB_SERIAL_ADU_ERR_MIN:
		  case MB_SERIAL_ADU_ERR_MAX:
		  case MB_SERIAL_CRC_ERROR:
		  case MB_SERIAL_PDU_ERR:

				tcp_server->stat.errors++;
				Client[client_id].stat.errors++;
				func_res_err(rsp_adu[TCPADU_FUNCTION], &tcp_server->stat);
				func_res_err(rsp_adu[TCPADU_FUNCTION], &Client[client_id].stat);
				stage_to_stat((MBCOM_RSP<<16) | (MBCOM_RTU_RECV<<8) | status, &tcp_server->stat);
				stage_to_stat((MBCOM_RSP<<16) | (MBCOM_RTU_RECV<<8) | status, &Client[client_id].stat);

			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_WRN|port_id, POLL_RTU_RECV, (unsigned) status, client_id, 0, 0);

				if(status==MB_SERIAL_READ_FAILURE) {
					// tcp_server->modbus_mode=MODBUS_PORT_ERROR; /// íåëüçÿ òàê äåëàòü
					goto EndRun;
					}
				continue;
		  	break;
		  default:;
		  };

///###-----------------------------			

			rsp_adu[TCPADU_TRANS_HI]=req_adu[TCPADU_TRANS_HI];
			rsp_adu[TCPADU_TRANS_LO]=req_adu[TCPADU_TRANS_LO];
			rsp_adu[TCPADU_PROTO_HI]=req_adu[TCPADU_PROTO_HI];
			rsp_adu[TCPADU_PROTO_LO]=req_adu[TCPADU_PROTO_LO];

			rsp_adu_len -= MB_SERIAL_CRC_LEN;

			rsp_adu[TCPADU_SIZE_HI ]=rsp_adu_len>>8;
			rsp_adu[TCPADU_SIZE_LO ]=rsp_adu_len&0xff;

			rsp_adu_len += TCPADU_ADDRESS;

			if(Security.show_data_flow==1)
				show_traffic(TRAFFIC_TCP_SEND, port_id, client_id, rsp_adu, rsp_adu_len);

			status = mbcom_tcp_send(Client[client_id].csd,
															rsp_adu,
															rsp_adu_len);

		switch(status) {
		  case 0:

				tcp_server->stat.sended++;
				Client[client_id].stat.sended++;
				func_res_ok(rsp_adu[TCPADU_FUNCTION], &tcp_server->stat);
				func_res_ok(rsp_adu[TCPADU_FUNCTION], &Client[client_id].stat);

		  	break;

		  case TCP_COM_ERR_SEND:

				tcp_server->stat.errors++;
				Client[client_id].stat.errors++;
				func_res_err(rsp_adu[TCPADU_FUNCTION], &tcp_server->stat);
				func_res_err(rsp_adu[TCPADU_FUNCTION], &Client[client_id].stat);
				stage_to_stat((MBCOM_RSP<<16) | (MBCOM_TCP_SEND<<8) | status, &tcp_server->stat);
				stage_to_stat((MBCOM_RSP<<16) | (MBCOM_TCP_SEND<<8) | status, &Client[client_id].stat);

			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_WRN|port_id, POLL_TCP_SEND, (unsigned) status, client_id, 0, 0);

		  	break;

		  default:;
		  };

	gettimeofday(&tv2, &tz);

	//tcp_server->clients[client_id].stat.request_time_average=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
	//tcp_server->clients[client_id].stat.latency_history[tcp_server->clients[client_id].stat.clp]=tcp_server->clients[client_id].stat.request_time_average;
	//tcp_server->clients[client_id].stat.clp=tcp_server->clients[client_id].stat.clp<MAX_LATENCY_HISTORY_POINTS?tcp_server->clients[client_id].stat.clp+1:0;

	//if(tcp_server->clients[client_id].stat.request_time_min>tcp_server->clients[client_id].stat.request_time_average)
	//  tcp_server->clients[client_id].stat.request_time_min=tcp_server->clients[client_id].stat.request_time_average;
	//if(tcp_server->clients[client_id].stat.request_time_max<tcp_server->clients[client_id].stat.request_time_average)
	//  tcp_server->clients[client_id].stat.request_time_max=tcp_server->clients[client_id].stat.request_time_average;

//	tcp_server->clients[client_id].stat.request_time_average=0;
//	for(i=0; i<MAX_LATENCY_HISTORY_POINTS; i++)
//	  tcp_server->clients[client_id].stat.request_time_average+=tcp_server->clients[client_id].stat.latency_history[i];
//	tcp_server->clients[client_id].stat.request_time_average/=MAX_LATENCY_HISTORY_POINTS;
	
//	printf("%d\n", tcp_server->stat.request_time_average);
//	printf("%d:%d\n", tv1.tv_sec, tv1.tv_usec);
//	printf("%d:%d\n", tv2.tv_sec, tv2.tv_usec);
	}
	EndRun: ;
	clear_client(client_id);
	// THREAD STOPPED
 	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|port_id, IFACE_THREAD_STOPPED, 0, 0, 0, 0);
	pthread_exit (0);	
}
