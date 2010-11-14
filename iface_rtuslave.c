/*
MOXA7GATE MODBUS GATEWAY SOFTWARE
SEM-ENGINEERING
                    BRYANSK 2010
*/

#include "global.h"
#include "interfaces.h"

#include "modbus_rtu.h"
#include "modbus_tcp.h"

///-----------------------------------------------------------------------------------------------------------------
void *bridge_proxy_thread(void *arg)
  {

	u8			tcp_adu[MB_TCP_MAX_ADU_LENGTH+2];// TCP ADU
	u16			tcp_adu_len;
	u8			serial_adu[MB_SERIAL_MAX_ADU_LEN];/// SERIAL ADU
	u16			serial_adu_len;
	u8			exception_adu[MB_SERIAL_MAX_ADU_LEN];/// SERIAL ADU
	u16			exception_adu_len;
	u8			memory_adu[MB_SERIAL_MAX_ADU_LEN];
	u16			memory_adu_len;

	int		status, j, k;

  int port_id=((unsigned)arg)>>8;
  int client_id=((unsigned)arg)&0xff;

//  printf("port %d cliet %d\n", port_id, client_id);
	GW_StaticData tmpstat;
	
	input_cfg	*inputDATA;
	inputDATA = &iDATA[port_id];
	int fd=inputDATA->serial.fd;
	
	struct timeval tv1, tv2;
	struct timezone tz;

	int i;
	//inputDATA->clients[client_id].stat.request_time_min=10000; // 10 seconds, must be "this->serial.timeout"
	//inputDATA->clients[client_id].stat.request_time_max=0;
	//inputDATA->clients[client_id].stat.request_time_average=0;
	//for(i=0; i<MAX_LATENCY_HISTORY_POINTS; i++) inputDATA->clients[client_id].stat.latency_history[i]=1000; //ms
	//inputDATA->clients[client_id].stat.clp=0;

///-----------------------------------------
  // THREAD STARTED
	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|port_id, 42, iDATA[port_id].modbus_mode, client_id, 0, 0);

	while (1) {
		
		//if(inputDATA->clients[0].connection_status!=MB_CONNECTION_ESTABLISHED) pthread_exit (0);
		
		clear_stat(&tmpstat);
		
		exception_adu_len=0;

///-----------------------------------
    /// kazhetsya net zaschity ot perepolneniya bufera priema "serial_adu[]"
	  status = serial_receive_adu(fd, &tmpstat, &tcp_adu[TCPADU_ADDRESS], &tcp_adu_len, NULL, inputDATA->serial.timeout, inputDATA->serial.ch_interval_timeout);

		if(gate502.show_data_flow==1)
			show_traffic(TRAFFIC_RTU_RECV, port_id, client_id, &tcp_adu[TCPADU_ADDRESS], tcp_adu_len);
		// так как прием данных ведется в буфер tcp_adu, приводим его содержимое в соответствие
		tcp_adu[0]=0;
		tcp_adu[1]=1;
		tcp_adu[2]=0;
		tcp_adu[3]=0;
		tcp_adu_len-=2;
		tcp_adu[TCPADU_SIZE_HI]=(tcp_adu_len >> 8) & 0xff;
		tcp_adu[TCPADU_SIZE_LO]=tcp_adu_len & 0xff;
		tcp_adu_len+=6;

		gettimeofday(&tv2, &tz);
		inputDATA->clients[client_id].stat.scan_rate=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
		if(inputDATA->clients[client_id].stat.scan_rate>MB_SCAN_RATE_INFINITE)
		  inputDATA->clients[client_id].stat.scan_rate=MB_SCAN_RATE_INFINITE;
//	inputDATA->clients[client_id].stat.request_time_average=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
//	printf("cycle begins after %d msec\n", inputDATA->clients[client_id].stat.request_time_average);
		gettimeofday(&tv1, &tz);

//	  pthread_mutex_unlock(&inputDATA->serial_mutex);
	  
//		tmpstat.accepted++;

		switch(status) {
		  case 0:
		  	break;
		  case MB_SERIAL_READ_FAILURE:
		  case MB_SERIAL_COM_TIMEOUT:
		  case MB_SERIAL_ADU_ERR_MIN:
		  case MB_SERIAL_ADU_ERR_MAX:
		  case MB_SERIAL_CRC_ERROR:
		  case MB_SERIAL_PDU_ERR:
		  	tmpstat.errors++;
  			// POLLING: RTU RECV
			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_ERR|port_id, 182, (unsigned) status, client_id, 0, 0);
				update_stat(&inputDATA->clients[client_id].stat, &tmpstat);
				update_stat(&iDATA[port_id].stat, &tmpstat);
				if(status==MB_SERIAL_READ_FAILURE) {
					iDATA[port_id].modbus_mode=MODBUS_PORT_ERROR;
					goto EndRun;
					}
				continue;
		  	break;
		  default:;
		  };

///###-----------------------------			
// в режиме PROXY выдаем данные из локального буфера

			status = process_moxamb_request(client_id, tcp_adu, tcp_adu_len, memory_adu, &memory_adu_len);

			if(status!=0) { // запрос был перенаправлен на другой порт
				if(status!=3) { // учет ошибочных запросов
					tmpstat.accepted++;
					func_res_err(tcp_adu[TCPADU_FUNCTION], &tmpstat);
					update_stat(&iDATA[port_id].stat, &tmpstat);
					//update_stat(&gate502.stat, &tmpstat);
					}
				continue;
				}

//		gettimeofday(&tv1, &tz);
		// считаем статистику, только если явно отправляем ответ клиенту
		tmpstat.accepted++;

///--------------------------------------------------------

		if(gate502.show_data_flow==1)
			show_traffic(TRAFFIC_RTU_SEND, port_id, client_id, memory_adu, memory_adu_len-2);

		status = mb_serial_send_adu(fd, &tmpstat, memory_adu, memory_adu_len-2, serial_adu, &serial_adu_len);

		switch(status) {
		  case 0:
		  	if(exception_adu_len==0) tmpstat.sended++;
		  	  else {
			  		tmpstat.errors_serial_adu++;
				  	tmpstat.errors++;
		  	    }
		  	break;
		  case MB_SERIAL_WRITE_ERR:
		  	tmpstat.errors++;
  			// POLLING: RTU SEND
			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_ERR|port_id, 183, (unsigned) status, client_id, 0, 0);
				update_stat(&inputDATA->clients[client_id].stat, &tmpstat);
				update_stat(&iDATA[port_id].stat, &tmpstat);
				continue;
		  	break;
		  default:;
		  };

	update_stat(&inputDATA->clients[client_id].stat, &tmpstat);
	update_stat(&iDATA[port_id].stat, &tmpstat);

	gettimeofday(&tv2, &tz);

	//inputDATA->clients[client_id].stat.request_time_average=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
	//inputDATA->clients[client_id].stat.latency_history[inputDATA->clients[client_id].stat.clp]=inputDATA->clients[client_id].stat.request_time_average;
	//inputDATA->clients[client_id].stat.clp=inputDATA->clients[client_id].stat.clp<MAX_LATENCY_HISTORY_POINTS?inputDATA->clients[client_id].stat.clp+1:0;

	//if(inputDATA->clients[client_id].stat.request_time_min>inputDATA->clients[client_id].stat.request_time_average)
	//  inputDATA->clients[client_id].stat.request_time_min=inputDATA->clients[client_id].stat.request_time_average;
	//if(inputDATA->clients[client_id].stat.request_time_max<inputDATA->clients[client_id].stat.request_time_average)
	//  inputDATA->clients[client_id].stat.request_time_max=inputDATA->clients[client_id].stat.request_time_average;

	//inputDATA->clients[client_id].stat.request_time_average=0;
	//for(i=0; i<MAX_LATENCY_HISTORY_POINTS; i++)
	//  inputDATA->clients[client_id].stat.request_time_average+=inputDATA->clients[client_id].stat.latency_history[i];
	//inputDATA->clients[client_id].stat.request_time_average/=MAX_LATENCY_HISTORY_POINTS;
	
//	printf("%d\n", inputDATA->stat.request_time_average);
//	printf("%d:%d\n", tv1.tv_sec, tv1.tv_usec);
//	printf("%d:%d\n", tv2.tv_sec, tv2.tv_usec);
//	printf("noop\n");
//  usleep(100000);

//	gettimeofday(&tv2, &tz);
//	inputDATA->clients[client_id].stat.request_time_average=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
//	printf("cycle ended after %d msec\n", inputDATA->clients[client_id].stat.request_time_average);
	}

	EndRun: ;
	// THREAD STOPPED
 	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|port_id, 43, 0, 0, 0, 0);
	inputDATA->clients[client_id].rc=1;
	pthread_exit (0);	
}
