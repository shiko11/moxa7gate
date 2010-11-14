/*
MOXA7GATE MODBUS GATEWAY SOFTWARE
SEM-ENGINEERING
                    BRYANSK 2010
*/

#include "global.h"
#include "interfaces.h"

#include "modbus_rtu.h"
#include "modbus_tcp.h"

void *srvr_tcp_child(void *arg)
  {
	u8			tcp_adu[MB_TCP_MAX_ADU_LENGTH];// TCP ADU
	u16			tcp_adu_len;
	u8			serial_adu[MB_SERIAL_MAX_ADU_LEN];/// SERIAL ADU
	u16			serial_adu_len;

	int		status;

  int port_id=((unsigned)arg)>>8;
  int client_id=((unsigned)arg)&0xff;

  // THREAD STARTED
	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|port_id, 42, iDATA[port_id].modbus_mode, client_id, 0, 0);

	int		tcsd = iDATA[port_id].clients[client_id].csd;
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

	while (1) {
		
		clear_stat(&tmpstat);

		status = mb_tcp_receive_adu(tcsd, &tmpstat, tcp_adu, &tcp_adu_len);

		if(gate502.show_data_flow==1)
			show_traffic(TRAFFIC_TCP_RECV, port_id, client_id, tcp_adu, tcp_adu_len);

		gettimeofday(&tv2, &tz);
		inputDATA->clients[client_id].stat.scan_rate=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
		if(inputDATA->clients[client_id].stat.scan_rate>MB_SCAN_RATE_INFINITE)
		  inputDATA->clients[client_id].stat.scan_rate=MB_SCAN_RATE_INFINITE;
		gettimeofday(&tv1, &tz);
		tmpstat.accepted++;

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
		  	//tmpstat.errors++;
  			// POLLING: TCP RECV ERROR
			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_ERR|port_id, 184, (unsigned) status, client_id, 0, 0);
				//update_stat(&inputDATA->clients[client_id].stat, &tmpstat);
				//update_stat(&iDATA[port_id].stat, &tmpstat);
				if(status==TCP_COM_ERR_NULL) {
					inputDATA->clients[client_id].connection_status=MB_CONNECTION_CLOSED;
					// inputDATA->modbus_mode=MODBUS_PORT_ERROR; /// нельзя так делать
					goto EndRun;
					}
				continue;
		  	break;
		  default:;
		  };
		
	pthread_mutex_lock(&inputDATA->serial_mutex);

		if(gate502.show_data_flow==1)
			show_traffic(TRAFFIC_RTU_SEND, port_id, client_id, &tcp_adu[6], tcp_adu_len-6);

		status = mb_serial_send_adu(fd, &tmpstat, &tcp_adu[6], tcp_adu_len-6, serial_adu, &serial_adu_len);

		switch(status) {
		  case 0:
		  	break;
		  case MB_SERIAL_WRITE_ERR:
		  	tmpstat.errors++;
				func_res_err(tcp_adu[TCPADU_FUNCTION], &tmpstat);
  			// POLLING: RTU SEND
			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_ERR|port_id, 183, (unsigned) status, client_id, 0, 0);
				update_stat(&inputDATA->clients[client_id].stat, &tmpstat);
				update_stat(&iDATA[port_id].stat, &tmpstat);
				continue;
		  	break;
		  default:;
		  };

    /// kazhetsya net zaschity ot perepolneniya bufera priema "serial_adu[]"
	  status = mb_serial_receive_adu(fd, &tmpstat, serial_adu, &serial_adu_len, &tcp_adu[MB_TCP_ADU_HEADER_LEN], inputDATA->serial.timeout, inputDATA->serial.ch_interval_timeout);

	  pthread_mutex_unlock(&inputDATA->serial_mutex);
	  
/*//---------- специальный случай при подаче команд на СКС-7 Диоген, обрабатываем
///--- убираем третий с конца байт в полученном ответе на запрос
		if((port_id==SERIAL_P3) && (serial_adu[RTUADU_FUNCTION]==0x06)) {
			serial_adu[serial_adu_len-3]=serial_adu[serial_adu_len-2];
			serial_adu[serial_adu_len-2]=serial_adu[serial_adu_len-1];
			serial_adu_len--;
			if(status==MB_SERIAL_PDU_ERR) status=0;
			}
*///-----------------------------------------------------------------------------

		if(gate502.show_data_flow==1)
			show_traffic(TRAFFIC_RTU_RECV, port_id, client_id, serial_adu, serial_adu_len);

//		printf("status: %d\n", status);

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
				func_res_err(serial_adu[RTUADU_FUNCTION], &tmpstat);
  			// POLLING: RTU RECV
			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_ERR|port_id, 182, (unsigned) status, client_id, 0, 0);
				update_stat(&inputDATA->clients[client_id].stat, &tmpstat);
				update_stat(&iDATA[port_id].stat, &tmpstat);
				if(status==MB_SERIAL_READ_FAILURE) {
					inputDATA->clients[client_id].connection_status=MB_CONNECTION_CLOSED;
					// inputDATA->modbus_mode=MODBUS_PORT_ERROR; /// нельзя так делать
					goto EndRun;
					}
				continue;
		  	break;
		  default:;
		  };

///###-----------------------------			
//			status = mb_tcp_make_adu(&OPC,oDATA,mb_tcp_adu);

			if(gate502.show_data_flow==1)
				show_traffic(TRAFFIC_TCP_SEND, port_id, client_id, serial_adu, serial_adu_len-2);

			status = mb_tcp_send_adu(tcsd, &tmpstat, serial_adu, serial_adu_len-2, tcp_adu, &tcp_adu_len);

		switch(status) {
		  case 0:
		  	tmpstat.sended++;
				func_res_ok(serial_adu[RTUADU_FUNCTION], &tmpstat);
		  	break;
		  case TCP_COM_ERR_SEND:
		  	tmpstat.errors++;
				func_res_err(serial_adu[RTUADU_FUNCTION], &tmpstat);
  			// POLLING: TCP SEND
			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_ERR|port_id, 185, (unsigned) status, client_id, 0, 0);
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

//	inputDATA->clients[client_id].stat.request_time_average=0;
//	for(i=0; i<MAX_LATENCY_HISTORY_POINTS; i++)
//	  inputDATA->clients[client_id].stat.request_time_average+=inputDATA->clients[client_id].stat.latency_history[i];
//	inputDATA->clients[client_id].stat.request_time_average/=MAX_LATENCY_HISTORY_POINTS;
	
//	printf("%d\n", inputDATA->stat.request_time_average);
//	printf("%d:%d\n", tv1.tv_sec, tv1.tv_usec);
//	printf("%d:%d\n", tv2.tv_sec, tv2.tv_usec);
	}
	EndRun: ;
	close(tcsd);
	inputDATA->current_connections_number--;
	inputDATA->clients[client_id].rc=1;
	// THREAD STOPPED
 	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|port_id, 43, 0, 0, 0, 0);
	pthread_exit (0);	
}
