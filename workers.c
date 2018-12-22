/*
MOXA7GATE MODBUS GATEWAY SOFTWARE
SEM-ENGINEERING
                    BRYANSK 2009
*/

#include "workers.h"

#include "modbus_rtu.h"
#include "modbus_tcp.h"
#include "global.h"

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
					// inputDATA->modbus_mode=MODBUS_PORT_ERROR; /// ������ ��� ������
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
	  
/*//---------- ����������� ������ ��� ������ ������ �� ���-7 ������, ������������
///--- ������� ������ � ����� ���� � ���������� ������ �� ������
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
					// inputDATA->modbus_mode=MODBUS_PORT_ERROR; /// ������ ��� ������
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


///-----------------------------------------------------------------------------------------------------------------
void *srvr_tcp_child2(void *arg) //����� - �������� ������ �� Modbus TCP
  {
	u8			tcp_adu[MB_TCP_MAX_ADU_LENGTH];// TCP ADU
	u16			tcp_adu_len;
	u8			serial_adu[MB_SERIAL_MAX_ADU_LEN];/// SERIAL ADU
	u16			serial_adu_len;

	int		status;
	int device_id, client_id, device_id2, exception_on;

  int port_id=((unsigned)arg)>>8;
//  int client_id=((unsigned)arg)&0xff;

//	int		tcsd = iDATA[port_id].clients[client_id].csd;
	GW_StaticData tmpstat;
	
	input_cfg	*inputDATA;
	inputDATA = &iDATA[port_id];
	
	struct timeval tv1, tv2;
	struct timezone tz;

	unsigned i;

///!!!
//	inputDATA->clients[client_id].stat.request_time_min=10000; // 10 seconds, must be "this->serial.timeout"
//	inputDATA->clients[client_id].stat.request_time_max=0;
//	inputDATA->clients[client_id].stat.request_time_average=0;
//	for(i=0; i<MAX_LATENCY_HISTORY_POINTS; i++) inputDATA->clients[client_id].stat.latency_history[i]=1000; //ms
//	inputDATA->clients[client_id].stat.clp=0;

	/// semaphore
	struct sembuf operations[1];
	operations[0].sem_num=port_id;
	operations[0].sem_op=-1;
	operations[0].sem_flg=0;
	semop(semaphore_id, operations, 1);
	
	int fd=inputDATA->serial.fd;

  // THREAD STARTED
	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|port_id, 42, iDATA[port_id].modbus_mode, DEFAULT_CLIENT, 0, 0);

	while (1) {
		
		clear_stat(&tmpstat);

		status=get_query_from_queue(&inputDATA->queue, &client_id, &device_id, tcp_adu, &tcp_adu_len);
		if(status!=0) continue;

///---------------------
//		gettimeofday(&tv2, &tz);
//		inputDATA->clients[client_id].stat.scan_rate=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
//		if(inputDATA->clients[client_id].stat.scan_rate>MB_SCAN_RATE_INFINITE)
//		  inputDATA->clients[client_id].stat.scan_rate=MB_SCAN_RATE_INFINITE;
		gettimeofday(&tv1, &tz);
		tmpstat.accepted++;

			exception_on=0;
			if(inputDATA->modbus_mode==GATEWAY_RTM) {
				//port_id=		i/0x2000; // 8 com ports
				device_id2=tcp_adu[TCPADU_ADDRESS];
				///!!!tcp_adu[6]=(i-port_id*0x2000)/0x200+1; // [1..16] device addresses
				tcp_adu[TCPADU_ADDRESS]=vslave[device_id].device;
				///!!!i-=(port_id*0x2000+(tcp_adu[6]-1)*0x200);

				i=((tcp_adu[TCPADU_START_HI]<<8)|tcp_adu[TCPADU_START_LO])-
					vslave[device_id].start+
					vslave[device_id].address_shift;
//				printf("result RTM adress: [%2.2X]\n", i);
				tcp_adu[TCPADU_START_HI]=(i>>8)&0xff;
				tcp_adu[TCPADU_START_LO]=i&0xff;

				};

		if(gate502.show_data_flow==1)
			show_traffic(TRAFFIC_RTU_SEND, port_id, client_id, &tcp_adu[6], tcp_adu_len-6);

		status = mb_serial_send_adu(fd, &tmpstat, &tcp_adu[6], tcp_adu_len-6, serial_adu, &serial_adu_len);

		switch(status) {
		  case 0:
		  	break;
		  case MB_SERIAL_WRITE_ERR:
		  	tmpstat.errors++;
				func_res_err(tcp_adu[TCPADU_ADDRESS], &tmpstat);
  			// POLLING: RTU SEND
			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_ERR|port_id, 183, (unsigned) status, client_id, 0, 0);

//				update_stat(&inputDATA->clients[client_id].stat, &tmpstat);
				update_stat(&iDATA[port_id].stat, &tmpstat);
				continue;
		  	break;
		  default:;
		  };

    /// ����� ��������� ������ �� ������������ ������ ������ "serial_adu[]"
	  status = mb_serial_receive_adu(fd, &tmpstat, serial_adu, &serial_adu_len, &tcp_adu[MB_TCP_ADU_HEADER_LEN], inputDATA->serial.timeout, inputDATA->serial.ch_interval_timeout);

		if(gate502.show_data_flow==1)
			show_traffic(TRAFFIC_RTU_RECV, port_id, client_id, serial_adu, serial_adu_len);
//	  pthread_mutex_unlock(&inputDATA->serial_mutex);
	  
		switch(status) {
		  case 0:
		  	break;
		  case MB_SERIAL_COM_TIMEOUT:
		  case MB_SERIAL_ADU_ERR_MIN:
		  case MB_SERIAL_ADU_ERR_MAX:
		  case MB_SERIAL_CRC_ERROR:
		  case MB_SERIAL_PDU_ERR:
				exception_on=1;
		  case MB_SERIAL_READ_FAILURE:
		  	tmpstat.errors++;
				func_res_err(tcp_adu[TCPADU_ADDRESS], &tmpstat);
  			// POLLING: RTU RECV
			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_ERR|port_id, 182, (unsigned) status, client_id, 0, 0);
//				update_stat(&inputDATA->clients[client_id].stat, &tmpstat);
				update_stat(&iDATA[port_id].stat, &tmpstat);
				if(status==MB_SERIAL_READ_FAILURE) {
					iDATA[port_id].modbus_mode=MODBUS_PORT_ERROR;
					goto EndRun;
					}
				if(exception_on==0) continue;
		  	break;
		  default:;
		  };

///###-----------------------------			
///$$$usleep(2000000);
//			status = mb_tcp_make_adu(&OPC,oDATA,mb_tcp_adu);

			///�������� ����������� ������ � �������
			//port_id=tcp_adu[6]/30;
			//tcp_adu[6]-=port_id*30;

			if(inputDATA->modbus_mode==GATEWAY_RTM)
				serial_adu[RTUADU_ADDRESS]=device_id2;
				else serial_adu[RTUADU_ADDRESS]+=port_id*30;

			if(exception_on==1) {
				serial_adu[1]|=0x80;
				serial_adu[2]=0x0b;
				serial_adu_len=3+2;
				}

			if(gate502.show_data_flow==1)
				show_traffic(TRAFFIC_TCP_SEND, port_id, client_id, serial_adu, serial_adu_len-2);

			status = mb_tcp_send_adu(gate502.clients[client_id].csd,
																&tmpstat, serial_adu, serial_adu_len-2, tcp_adu, &tcp_adu_len);

		switch(status) {
		  case 0:
		  	if(exception_on!=1) tmpstat.sended++;
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

	if(exception_on==1) continue;
//	update_stat(&inputDATA->clients[client_id].stat, &tmpstat);
	update_stat(&iDATA[port_id].stat, &tmpstat);

	gettimeofday(&tv2, &tz);

	//inputDATA->stat.request_time_average=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
//	inputDATA->clients[client_id].stat.latency_history[inputDATA->clients[client_id].stat.clp]=inputDATA->clients[client_id].stat.request_time_average;
//	inputDATA->clients[client_id].stat.clp=inputDATA->clients[client_id].stat.clp<MAX_LATENCY_HISTORY_POINTS?inputDATA->clients[client_id].stat.clp+1:0;

//	if(inputDATA->clients[client_id].stat.request_time_min>inputDATA->clients[client_id].stat.request_time_average)
//	  inputDATA->clients[client_id].stat.request_time_min=inputDATA->clients[client_id].stat.request_time_average;
//	if(inputDATA->clients[client_id].stat.request_time_max<inputDATA->clients[client_id].stat.request_time_average)
//	  inputDATA->clients[client_id].stat.request_time_max=inputDATA->clients[client_id].stat.request_time_average;

//	inputDATA->clients[client_id].stat.request_time_average=0;
//	for(i=0; i<MAX_LATENCY_HISTORY_POINTS; i++)
//	  inputDATA->clients[client_id].stat.request_time_average+=inputDATA->clients[client_id].stat.latency_history[i];
//	inputDATA->clients[client_id].stat.request_time_average/=MAX_LATENCY_HISTORY_POINTS;
	
	}
	EndRun: ;
	// THREAD STOPPED
 	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|port_id, 43, 0, 0, 0, 0);
	inputDATA->clients[client_id].rc=1;
	pthread_exit (0);	
}

///-----------------------------------------------------------------------------------------------------------------
void *srvr_tcp_bridge(void *arg) //����� - �������� ������ �� Modbus TCP
  {

	u8			tcp_adu[MB_TCP_MAX_ADU_LENGTH];// TCP ADU
	u16			tcp_adu_len;
	u8			serial_adu[MB_SERIAL_MAX_ADU_LEN];/// SERIAL ADU
	u16			serial_adu_len;
	u8			exception_adu[MB_SERIAL_MAX_ADU_LEN];/// SERIAL ADU
	u16			exception_adu_len;

	int		status;

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

  // THREAD STARTED
	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|port_id, 42, iDATA[port_id].modbus_mode, DEFAULT_CLIENT, 0, 0);

	for(i=0; i<iDATA[port_id].accepted_connections_number; i++)
		bridge_reset_tcp(&iDATA[port_id], i);

///-----------------------------------------

	while (1) {
		
		//if(inputDATA->clients[0].connection_status!=MB_CONNECTION_ESTABLISHED) pthread_exit (0);
		
		clear_stat(&tmpstat);
		
		exception_adu_len=0;

///-----------------------------------
    /// kazhetsya net zaschity ot perepolneniya bufera priema "serial_adu[]"
	  status = serial_receive_adu(fd, &tmpstat, serial_adu, &serial_adu_len, &tcp_adu[MB_TCP_ADU_HEADER_LEN], inputDATA->serial.timeout, inputDATA->serial.ch_interval_timeout);

		if(gate502.show_data_flow==1)
			show_traffic(TRAFFIC_RTU_RECV, port_id, client_id, serial_adu, serial_adu_len);

		gettimeofday(&tv2, &tz);
		inputDATA->clients[client_id].stat.scan_rate=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
		if(inputDATA->clients[client_id].stat.scan_rate>MB_SCAN_RATE_INFINITE)
		  inputDATA->clients[client_id].stat.scan_rate=MB_SCAN_RATE_INFINITE;
//	inputDATA->clients[client_id].stat.request_time_average=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
//	printf("cycle begins after %d msec\n", inputDATA->clients[client_id].stat.request_time_average);
		gettimeofday(&tv1, &tz);

//	  pthread_mutex_unlock(&inputDATA->serial_mutex);
	  
		tmpstat.accepted++;

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
				iDATA[port_id].modbus_mode=MODBUS_PORT_ERROR;
				if(status==MB_SERIAL_READ_FAILURE) {
					inputDATA->modbus_mode=MODBUS_PORT_ERROR;
					goto EndRun;
					}
		  	break;
		  default:;
		  };

///###-----------------------------
//			status = mb_tcp_make_adu(&OPC,oDATA,mb_tcp_adu);

    tcp_adu[0]=0x03; ///!!!
    tcp_adu[1]=0x03; 
    tcp_adu[2]=0x00;
    tcp_adu[3]=0x00; 
    tcp_adu[4]=0x00;

    for(i=0; i<inputDATA->accepted_connections_number; i++)
      if(inputDATA->clients[i].mb_slave==serial_adu[0]) {
      	client_id=i;
      	break;
        }
//    printf("i=%d, total=%d\n", i, inputDATA->accepted_connections_number);

    if(i==inputDATA->accepted_connections_number) {
  		tmpstat.errors_serial_adu++;
	  	tmpstat.errors++;
			update_stat(&inputDATA->clients[client_id].stat, &tmpstat);
			update_stat(&iDATA[port_id].stat, &tmpstat);
			exception_adu_len=9;
			tcp_adu[7]=serial_adu[1]|0x80;
			tcp_adu[8]=0x0a;
      } else if(iDATA[port_id].clients[client_id].connection_status!=MB_CONNECTION_ESTABLISHED) {
				exception_adu_len=9;
				tcp_adu[7]=serial_adu[1]|0x80;
				tcp_adu[8]=0x0b;
				bridge_reset_tcp(&iDATA[port_id], client_id);
			  }

    tcp_adu[6]=serial_adu[0]; /// MODBUS DEVICE UID
    
		if(exception_adu_len==0) {
			
	  if(inputDATA->clients[client_id].address_shift!=MB_ADDRESS_NO_SHIFT) {
	  	status=(serial_adu[2]<<8)|serial_adu[3];
	  	status+=inputDATA->clients[client_id].address_shift;
	  	serial_adu[2]=(status>>8)&0xff;
	  	serial_adu[3]=status&0xff;
	  	}

		if(gate502.show_data_flow==1)
			show_traffic(TRAFFIC_TCP_SEND, port_id, client_id, serial_adu, serial_adu_len-2);

		status = mb_tcp_send_adu(inputDATA->clients[client_id].csd, &tmpstat, serial_adu, serial_adu_len-2, tcp_adu, &tcp_adu_len);

//	gettimeofday(&tv2, &tz);
//	inputDATA->clients[client_id].stat.request_time_average=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
//	printf("tcp sended after %d msec\n", inputDATA->clients[client_id].stat.request_time_average);

		switch(status) {
		  case 0:
		  	break;
		  case TCP_COM_ERR_SEND:
		  	tmpstat.errors++;
  			// POLLING: TCP SEND
			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_ERR|port_id, 185, (unsigned) status, client_id, 0, 0);
				shutdown(inputDATA->clients[client_id].csd, SHUT_RDWR);
				close(inputDATA->clients[client_id].csd);
				inputDATA->clients[client_id].csd=-1;
				iDATA[port_id].clients[client_id].connection_status=MB_CONNECTION_ERROR;
				iDATA[port_id].current_connections_number--;
				continue;
		  	break;
		  default:;
		  };

		status = mb_tcp_receive_adu(inputDATA->clients[client_id].csd, &tmpstat, tcp_adu, &tcp_adu_len);

		if(gate502.show_data_flow==1)
			show_traffic(TRAFFIC_TCP_RECV, port_id, client_id, tcp_adu, tcp_adu_len);
//	gettimeofday(&tv2, &tz);
//	inputDATA->clients[client_id].stat.request_time_average=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
//	printf("tcp received after %d msec\n", inputDATA->clients[client_id].stat.request_time_average);

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
		  	tmpstat.errors++;
  			// POLLING: TCP RECV ERROR
			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_ERR|port_id, 184, (unsigned) status, client_id, 0, 0);
				update_stat(&inputDATA->clients[client_id].stat, &tmpstat);
				update_stat(&iDATA[port_id].stat, &tmpstat);
				continue;
		  	break;
		  default:;
		  };
		
		}

/*  ������ ����, ��� ������ �� ����
	  if(	(inputDATA->clients[client_id].address_shift!=MB_ADDRESS_NO_SHIFT)&&
	  		((tcp_adu[6+1]&0x80)==0)	) {
	  	status=(tcp_adu[6+2]<<8)|tcp_adu[6+3];
	  	status-=inputDATA->clients[client_id].address_shift;
	  	tcp_adu[6+2]=(status>>8)&0xff;
	  	tcp_adu[6+3]=status&0xff;
	  	}*/

		if(gate502.show_data_flow==1)
			show_traffic(TRAFFIC_RTU_SEND, port_id, client_id, &tcp_adu[6], tcp_adu_len-6);

		status = mb_serial_send_adu(fd, &tmpstat, &tcp_adu[6], tcp_adu_len-6, serial_adu, &serial_adu_len);

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
//	close(tcsd);
//	inputDATA->current_connections_number--;
	inputDATA->clients[client_id].rc=1;
	// THREAD STOPPED
 	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|port_id, 43, 0, 0, 0, 0);
	pthread_exit (0);	
}
	
///-----------------------------------------------------------------------------------------------------------------
int bridge_reset_tcp(input_cfg *bridge, int client)
	{
	if (bridge->clients[client].csd < 0)
	  bridge->clients[client].csd = socket(AF_INET, SOCK_STREAM, 0);
	if (bridge->clients[client].csd < 0) {
		perror("bridge socket");
		//bridge->modbus_mode=MODBUS_PORT_ERROR;
    //strcpy(bridge->bridge_status, "ERR");
		return 1;
		} // else printf("\nPORT2 BRIDGE #%d socket OK...\n", client);

	struct sockaddr_in server;
	//struct hostent *h;
	//h = gethostbyname ("server.domain.ru");
	//memcpy ((char *)kserver.sin_addr,h->h_addr,h->h_length);
	// ���������� ��������� ����������
	server.sin_family = AF_INET;
	// ���������� IP-����� �������
	//server.sin_addr.s_addr = 0x0a0006f0; // 10.0.0.240
	server.sin_addr.s_addr = bridge->clients[client].ip;
  //	server.sin_addr.s_addr = 0xc00000fc; // 192.0.0.252
	// ���������� ���� �������
	server.sin_port =  htons(bridge->clients[client].port);

	struct timeval tvs, tvr;
	int optlen=sizeof(tvs);
	tvs.tv_sec=0; tvs.tv_usec=500000;
	tvr.tv_sec=0; tvr.tv_usec=500000;

	if(
	(setsockopt(bridge->clients[client].csd, SOL_SOCKET, SO_SNDTIMEO, &tvs, optlen)!=0)||
	(setsockopt(bridge->clients[client].csd, SOL_SOCKET, SO_RCVTIMEO, &tvr, optlen)!=0)) 
	// SOCKET INITIALIZED
 	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|bridge->serial.p_num, 65, 2, client, 0, 0);
//	printf("for client%d send_timeout=%dms, receive_timeout=%dms\n", client, tvs.tv_sec*1000+tvs.tv_usec/1000, tvr.tv_sec*1000+tvr.tv_usec/1000);

	// ����� ������� connect()
	if(connect(bridge->clients[client].csd, (struct sockaddr *)&server, sizeof(server))==-1) {
		perror("bridge tcp connection");																			
		// CONNECTION FAILED
	 	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|bridge->serial.p_num, 69, bridge->clients[client].ip, client, 0, 0);
		return 2;
		} else // CONNECTION ESTABLISHED
		 	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|bridge->serial.p_num, 68, bridge->clients[client].ip, client, 0, 0);


	bridge->clients[client].connection_status=MB_CONNECTION_ESTABLISHED;
  bridge->current_connections_number++;
	time(&bridge->clients[client].connection_time);
	return 0;
	}
///-----------------------------------------------------------------------------------------------------------------
// ������� �������������� � ���������������� �����������, ��������� ����� GATEWAY_PROXY (����������� �����)
void *gateway_proxy_thread(void *arg)
  {
	u8			tcp_adu[MB_TCP_MAX_ADU_LENGTH];// TCP ADU
	u16			tcp_adu_len;
	u8			serial_adu[MB_SERIAL_MAX_ADU_LEN];/// SERIAL ADU
	u16			serial_adu_len;

	u16			*mem_start;
	u8			*m_start;
	u8 mask_src, mask_dst;

	int		status;

  int port_id=((unsigned)arg)>>8;
  int client_id, device_id; //=((unsigned)arg)&0xff;

//	int		tcsd = iDATA[port_id].clients[client_id].csd;
	GW_StaticData tmpstat;
	
	input_cfg	*inputDATA;
	inputDATA = &iDATA[port_id];
	
	struct timeval tv1, tv2;
	struct timezone tz;

	unsigned i, j, n;

///!!!
//	inputDATA->clients[client_id].stat.request_time_min=10000; // 10 seconds, must be "this->serial.timeout"
//	inputDATA->clients[client_id].stat.request_time_max=0;
//	inputDATA->clients[client_id].stat.request_time_average=0;
//	for(i=0; i<MAX_LATENCY_HISTORY_POINTS; i++) inputDATA->clients[client_id].stat.latency_history[i]=1000; //ms
//	inputDATA->clients[client_id].stat.clp=0;

	/// semaphore
	struct sembuf operations[1];
	operations[0].sem_num=port_id;
	operations[0].sem_op=-1;
	operations[0].sem_flg=0;

	semop(semaphore_id, operations, 1);
	int fd=inputDATA->serial.fd;
	inputDATA->queue.operations[0].sem_flg=IPC_NOWAIT;

  // THREAD STARTED
	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|port_id, 42, iDATA[port_id].modbus_mode, DEFAULT_CLIENT, 0, 0);

	int queue_has_query;
	int queue_current;

	while (1) for(i=0; i<=MAX_QUERY_ENTRIES; i++) {

		// status = semop ( semaphore_id, operations, 1);
		status=get_query_from_queue(&iDATA[port_id].queue, &client_id, &device_id, tcp_adu, &tcp_adu_len);

		//queue_has_query=(status==-1)&&(errno==EAGAIN)?0:1;
		queue_has_query=status==-1?0:1;
		if(queue_has_query==1) i=MAX_QUERY_ENTRIES;
		if((queue_has_query!=1)&&(i==MAX_QUERY_ENTRIES)) continue;

		clear_stat(&tmpstat);

	if(i!=MAX_QUERY_ENTRIES) {//��������� ��������� ������ �� ������� ������
		if(query_table[i].port!=port_id) continue; ///!!! ����� ����������� ������ � ���������
    if(
			(query_table[i].length==0) ||
			(query_table[i].mbf==0) ||
			(query_table[i].device==0)
			) continue; ///!!! ����� ����������� ������ � ���������

		usleep(query_table[i].delay*1000);
		//printf("P%d, query #%d from table\n", port_id+1, i);

		tcp_adu[TCPADU_ADDRESS]=query_table[i].device;
		tcp_adu[TCPADU_FUNCTION]=query_table[i].mbf;

		tcp_adu[TCPADU_START_HI]=(query_table[i].start>>8)&0xff;
		tcp_adu[TCPADU_START_LO]= query_table[i].start&0xff;
		tcp_adu[TCPADU_LEN_HI]=(query_table[i].length>>8)&0xff;
		tcp_adu[TCPADU_LEN_LO]= query_table[i].length&0xff;
																																	
		tcp_adu_len=12;

		} else { //������������ ������ �� ������� ����������������� �����

//			tcp_adu_len+=6;
//			for(j=tcp_adu_len-1; j>=6; j--) tcp_adu[j]=tcp_adu[j-6];

			tcp_adu[TCPADU_ADDRESS]=query_table[device_id].device;
			j=	(((tcp_adu[TCPADU_START_HI]<<8) | tcp_adu[TCPADU_START_LO])&0xffff)-
					query_table[device_id].offset+
					query_table[device_id].start;
			tcp_adu[TCPADU_START_HI]=(j>>8)&0xff;
			tcp_adu[TCPADU_START_LO]=j&0xff;

//			printf("P%d query #%d from queue\n", port_id+1, queue_current);
			}

	  ///---------------------
		gettimeofday(&tv1, &tz);
		tmpstat.accepted++;
		///!!! ���������� ��������� ��������������� � ������� int refresh_shm(void *arg) ��� ��������
		//gate502.wData4x[gate502.status_info+3*port_id+0]++;

		if(gate502.show_data_flow==1)
			show_traffic(TRAFFIC_RTU_SEND, port_id, i, &tcp_adu[6], tcp_adu_len-6);

		status = mb_serial_send_adu(fd, &tmpstat, &tcp_adu[6], tcp_adu_len-6, serial_adu, &serial_adu_len);

		switch(status) {
		  case 0:
		  	break;
		  case MB_SERIAL_WRITE_ERR:
		  	tmpstat.errors++;
				func_res_err(tcp_adu[TCPADU_FUNCTION], &tmpstat);
  			// POLLING: RTU SEND
			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_ERR|port_id, 183, (unsigned) status, i, 0, 0);
//				update_stat(&inputDATA->clients[client_id].stat, &tmpstat);
				update_stat(&iDATA[port_id].stat, &tmpstat);

		///!!! �����-��������� ����� ���� ��������� � ������� ������ 4x �����
		//gate502.wData4x[query_table[i].status_register]&=(~query_table[i].status_bit);
				query_table[i].err_counter++;
				if(i!=MAX_QUERY_ENTRIES)
					if(query_table[i].err_counter >= query_table[i].critical)
						query_table[i].status_bit=0; ///!!! �������� ��������� � ���������� ����� � modbus-rtu ��������
				continue;
		  	break;
		  default:;
		  };

    /// kazhetsya net zaschity ot perepolneniya bufera priema "serial_adu[]"
	  status = mb_serial_receive_adu(fd, &tmpstat, serial_adu, &serial_adu_len, &tcp_adu[MB_TCP_ADU_HEADER_LEN], inputDATA->serial.timeout, inputDATA->serial.ch_interval_timeout);

/*//---------- ����������� ������ ��� ������ ������ �� ���-7 ������, ������������
///--- ������� ������ � ����� ���� � ���������� ������ �� ������
		if((port_id==SERIAL_P3) && (serial_adu[RTUADU_FUNCTION]==0x06)) {
			serial_adu[serial_adu_len-3]=serial_adu[serial_adu_len-2];
			serial_adu[serial_adu_len-2]=serial_adu[serial_adu_len-1];
			serial_adu_len--;
			//if(status==MB_SERIAL_PDU_ERR) status=0;
			status=0;
			}
*///-----------------------------------------------------------------------------

		if(gate502.show_data_flow==1)
			show_traffic(TRAFFIC_RTU_RECV, port_id, i, serial_adu, serial_adu_len);

//		printf("status: %d\n", status);

		switch(status) {
		  case 0:
		  	break;
		  case MB_SERIAL_COM_TIMEOUT:
		  case MB_SERIAL_ADU_ERR_MIN:
		  case MB_SERIAL_ADU_ERR_MAX:
		  case MB_SERIAL_CRC_ERROR:
		  case MB_SERIAL_PDU_ERR:
		  case MB_SERIAL_READ_FAILURE:
		  	tmpstat.errors++;
				func_res_err(tcp_adu[TCPADU_FUNCTION], &tmpstat);
  			// POLLING: RTU RECV
			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_ERR|port_id, 182, (unsigned) status, i, 0, 0);
//				update_stat(&inputDATA->clients[client_id].stat, &tmpstat);
				update_stat(&iDATA[port_id].stat, &tmpstat);

//				if(status==MB_SERIAL_READ_FAILURE) goto EndRun; ///!!!
		///!!! �����-��������� ����� ���� ��������� � ������� ������ 4x �����
		//gate502.wData4x[query_table[i].status_register]&=(~query_table[i].status_bit);
				query_table[i].err_counter++;
				if(i!=MAX_QUERY_ENTRIES)
					if(query_table[i].err_counter >= query_table[i].critical)
						query_table[i].status_bit=0;  ///!!! �������� ��������� � ���������� ����� � modbus-rtu ��������
				continue;
		  	break;
		  default:;
		  };

///###-----------------------------			
	if(i!=MAX_QUERY_ENTRIES) { // ��������� �������� ���������� ������
//		for(j=3; j<serial_adu_len-2; j++)
//			oDATA[2*query_table[i].offset+j-3]=serial_adu[j];
		if((serial_adu[RTUADU_FUNCTION]&0x80)==0) {
		switch(serial_adu[RTUADU_FUNCTION]) {

			case MBF_READ_HOLDING_REGISTERS:
				mem_start=gate502.wData4x;

			case MBF_READ_INPUT_REGISTERS:
				if(serial_adu[RTUADU_FUNCTION]==MBF_READ_INPUT_REGISTERS)
					mem_start=gate502.wData3x;

				for(j=3; j<serial_adu_len-2; j+=2)
					mem_start[query_table[i].offset+(j-3)/2]=
						(serial_adu[j] << 8) | serial_adu[j+1];

				break;

			case MBF_READ_COILS:
				m_start=gate502.wData1x;

			case MBF_READ_DECRETE_INPUTS:
				if(serial_adu[RTUADU_FUNCTION]==MBF_READ_DECRETE_INPUTS)
					m_start=gate502.wData2x;

				for(j=3; j<serial_adu_len-2; j++)
					for(n=0; n<8; n++) {

						mask_src = 0x01 << n; 								// ������� ����� � �������� �����
						mask_dst = 0x01 << (query_table[i].offset + n + (j-3)*8) % 8;	// ������� ����� � ����� ����������
						status = (query_table[i].offset + n + (j-3)*8) / 8;						// ������ ����� ���������� � �������

						m_start[status]= serial_adu[j] & mask_src ?\
							m_start[status] | mask_dst:\
							m_start[status] & (~mask_dst);

						}

				break;

			default:;
			}
		///!!! �����-��������� ����� ���� ��������� � ������� ������ 4x �����
		//gate502.wData4x[query_table[i].status_register]|=query_table[i].status_bit;
		query_table[i].status_bit=1;
		query_table[i].err_counter=0;

  	tmpstat.sended++;
		func_res_ok(serial_adu[RTUADU_FUNCTION], &tmpstat);
		} else { // �������� ����������
			func_res_err(serial_adu[RTUADU_FUNCTION], &tmpstat);
			
			query_table[i].err_counter++;
			if(query_table[i].err_counter >= query_table[i].critical)
				query_table[i].status_bit=0;
		  }

		///!!! ���������� ��������� ��������������� � ������� int refresh_shm(void *arg) ��� ��������
		//gate502.wData4x[gate502.status_info+3*port_id+1]++;

		} else { //���������� ��������� ������� ��������������� �������

			serial_adu[RTUADU_ADDRESS]=gate502.modbus_address;
			j=(((serial_adu[RTUADU_START_HI]<<8) | serial_adu[RTUADU_START_LO])&0xffff)+
				query_table[device_id].offset-
				query_table[device_id].start;
			serial_adu[RTUADU_START_HI]=(j>>8)&0xff;
			serial_adu[RTUADU_START_LO]=j&0xff;

			/// ���������� ��� ������� � �������������� �������, ������������ ��� �������� ������
			if(gate502.clients[client_id].p_num<=SERIAL_P8)
			if(	(iDATA[gate502.clients[client_id].p_num].modbus_mode==BRIDGE_PROXY) &&
					(gate502.clients[client_id].connection_status==MB_CONNECTION_ESTABLISHED)) {
						
				if(gate502.show_data_flow==1)
					show_traffic(TRAFFIC_RTU_SEND, gate502.clients[client_id].p_num, client_id, serial_adu, serial_adu_len-2);
				///!!! ���������� ������� �������� ��� ������������� ������ ���������� ������� � ����� ������
				status = mb_serial_send_adu(iDATA[gate502.clients[client_id].p_num].serial.fd, &tmpstat, serial_adu, serial_adu_len-2, tcp_adu, &tcp_adu_len);
		
				switch(status) {
				  case 0:
				  	tmpstat.sended++;
//				  	if(exception_adu_len==0) tmpstat.sended++;
//				  	  else {
//					  		tmpstat.errors_serial_adu++;
//						  	tmpstat.errors++;
//				  	    }
				  	break;
				  case MB_SERIAL_WRITE_ERR:
				  	tmpstat.errors++;
		  			// POLLING: RTU SEND
					 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_ERR|gate502.clients[client_id].p_num, 183, (unsigned) status, client_id, 0, 0);
						update_stat(&inputDATA->clients[client_id].stat, &tmpstat);
						update_stat(&iDATA[gate502.clients[client_id].p_num].stat, &tmpstat);
						continue;
				  	break;
				  default:;
				  };
						
				} else {
	
				if(gate502.show_data_flow==1)
					show_traffic(TRAFFIC_TCP_SEND, port_id, client_id, serial_adu, serial_adu_len-2);
	//			status = mb_tcp_send_adu(tcsd, &tmpstat, serial_adu, serial_adu_len-2, tcp_adu, &tcp_adu_len);
				status = mb_tcp_send_adu(gate502.clients[client_id].csd,
																	&tmpstat, serial_adu, serial_adu_len-2, tcp_adu, &tcp_adu_len);
	
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
				}
	
			/// ��� ���������� ������� � ������� ��������� � ��������� ��������� ������ �� ������� ������
			i=device_id;
			}

//	update_stat(&inputDATA->clients[client_id].stat, &tmpstat);
	update_stat(&iDATA[port_id].stat, &tmpstat);

	gettimeofday(&tv2, &tz);
	inputDATA->stat.request_time=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
		///!!! ���������� ��������� ��������������� � ������� int refresh_shm(void *arg) ��� ��������
	//gate502.wData4x[gate502.status_info+3*port_id+2]=inputDATA->stat.request_time_average;
	}
	EndRun: ;
	// THREAD STOPPED
 	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|port_id, 43, 0, 0, 0, 0);
	pthread_exit (0);	
}
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
		// ��� ��� ����� ������ ������� � ����� tcp_adu, �������� ��� ���������� � ������������
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
		  	break;
		  default:;
		  };

///###-----------------------------			
// � ������ PROXY ������ ������ �� ���������� ������

			status = process_moxamb_request(client_id, tcp_adu, tcp_adu_len, memory_adu, &memory_adu_len);

			if(status!=0) { // ������ ��� ������������� �� ������ ����
				if(status!=3) { // ���� ��������� ��������
					tmpstat.accepted++;
					func_res_err(tcp_adu[TCPADU_FUNCTION], &tmpstat);
					update_stat(&iDATA[port_id].stat, &tmpstat);
					//update_stat(&gate502.stat, &tmpstat);
					}
				continue;
				}

//		gettimeofday(&tv1, &tz);
		// ������� ����������, ������ ���� ���� ���������� ����� �������
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
///-----------------------------------------------------------------------------------------------------------------
void *moxa_mb_thread(void *arg) //����� - �������� ������ �� Modbus TCP
  {
	u8			tcp_adu[MB_TCP_MAX_ADU_LENGTH];// TCP ADU
	u16			tcp_adu_len;
	u8			memory_adu[MB_SERIAL_MAX_ADU_LEN];
	u16			memory_adu_len;

	int		status;
	int		client_id, device_id;

  int port_id; //=MOXA_MB_DEVICE;
//  int client_id=((unsigned)arg)&0xff;

//	int		tcsd = iDATA[port_id].clients[client_id].csd;
	GW_StaticData tmpstat;
	
//	input_cfg	*inputDATA;
//	inputDATA = &iDATA[port_id];
	
	struct timeval tv1, tv2;
	struct timezone tz;

	unsigned i, j, k, n;

///!!!
//	inputDATA->clients[client_id].stat.request_time_min=10000; // 10 seconds, must be "this->serial.timeout"
//	inputDATA->clients[client_id].stat.request_time_max=0;
//	inputDATA->clients[client_id].stat.request_time_average=0;
//	for(i=0; i<MAX_LATENCY_HISTORY_POINTS; i++) inputDATA->clients[client_id].stat.latency_history[i]=1000; //ms
//	inputDATA->clients[client_id].stat.clp=0;

	/// semaphore
	struct sembuf operations[1];
	operations[0].sem_num=MOXA_MB_DEVICE;
	operations[0].sem_op=-1;
	operations[0].sem_flg=0;
	semop(semaphore_id, operations, 1);
	
//	int fd=inputDATA->serial.fd;

  // THREAD STARTED
	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|EVENT_SRC_MOXAMB, 42, MOXA_MB_DEVICE, DEFAULT_CLIENT, 0, 0);

	while (1) {
		
		status=get_query_from_queue(&gate502.queue, &client_id, &device_id, tcp_adu, &tcp_adu_len);
		if(status!=0) continue;

		clear_stat(&tmpstat);

	  ///---------------------
//			int exception_on;
//			exception_on=0;

///###-----------------------------			

//			if(exception_on==1) {
//				memory_adu[1]|=0x80;
//				memory_adu[2]=0x0b;
//				memory_adu_len=3+2;
//				}

			status = process_moxamb_request(client_id, tcp_adu, tcp_adu_len, memory_adu, &memory_adu_len);

			if(status!=0) { // ������ ��� ������������� �� ������ ����
				if(status!=3) { // ���� ��������� ��������
					tmpstat.accepted++;
					func_res_err(tcp_adu[TCPADU_FUNCTION], &tmpstat);
					update_stat(&gate502.stat, &tmpstat);
					}
				continue;
				}

//		gettimeofday(&tv1, &tz);
		// ������� ����������, ������ ���� ���� ���������� ����� �������
		tmpstat.accepted++;

			if(gate502.show_data_flow==1)
				show_traffic(TRAFFIC_TCP_SEND, EVENT_SRC_MOXAMB, client_id, memory_adu, memory_adu_len-2);

			status = mb_tcp_send_adu(gate502.clients[client_id].csd,
																&tmpstat, memory_adu, memory_adu_len-2, tcp_adu, &tcp_adu_len);

		switch(status) {
		  case 0:
		  	//if(exception_on!=1)
				tmpstat.sended++;
				func_res_ok(memory_adu[RTUADU_FUNCTION], &tmpstat);
		  	break;
		  case TCP_COM_ERR_SEND:
		  	tmpstat.errors++;
				func_res_err(memory_adu[RTUADU_FUNCTION], &tmpstat);
  			// POLLING: TCP SEND
			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_ERR|EVENT_SRC_MOXAMB, 185, (unsigned) status, client_id, 0, 0);
		  	break;
		  default:;
		  };

//	if(exception_on==1) continue;///!!! ��������� ������ �.�., ����� �� ���� ��� �������� ������?

	update_stat(&gate502.stat, &tmpstat);
//	gettimeofday(&tv2, &tz);
// ����� �� �������, ��� ��� ��� ��������� ���������� �������� ��� �������� ������� �����
//	gate502.stat.request_time_average=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
	}

	EndRun: ;
	// THREAD STOPPED
 	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|port_id, 43, 0, 0, 0, 0);
	pthread_exit (0);	
}
///-----------------------------------------------------------------------------------------------------------------
