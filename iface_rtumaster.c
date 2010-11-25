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

///----------------------------------------------------------------------------
void *srvr_tcp_child2(void *arg) //РТЙЕН - РЕТЕДБЮБ ДБООЩИ РП Modbus TCP
  {
	u8			tcp_adu[MB_TCP_MAX_ADU_LENGTH];// TCP ADU
	u16			tcp_adu_len;
	u8			serial_adu[MB_SERIAL_MAX_ADU_LEN];/// SERIAL ADU
	u16			serial_adu_len;

	int		status;
	int device_id, client_id, device_id2, exception_on;

  int port_id=((unsigned)arg)>>8;
//  int client_id=((unsigned)arg)&0xff;

//	int		tcsd = IfaceRTU[port_id].clients[client_id].csd;
	GW_StaticData tmpstat;
	
	GW_Iface	*inputDATA;
	inputDATA = &IfaceRTU[port_id];
	
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
	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|port_id, 42, IfaceRTU[port_id].modbus_mode, 0, 0, 0);

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
					vslave[device_id].offset;
//				printf("result RTM adress: [%2.2X]\n", i);
				tcp_adu[TCPADU_START_HI]=(i>>8)&0xff;
				tcp_adu[TCPADU_START_LO]=i&0xff;

				};

		if(Security.show_data_flow==1)
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
				update_stat(&IfaceRTU[port_id].stat, &tmpstat);
				continue;
		  	break;
		  default:;
		  };

    /// нужно проверить защиту от переполнения буфера приема "serial_adu[]"
	  status = mb_serial_receive_adu(fd, &tmpstat, serial_adu, &serial_adu_len, &tcp_adu[MB_TCP_ADU_HEADER_LEN], inputDATA->serial.timeout, inputDATA->serial.ch_interval_timeout);

		if(Security.show_data_flow==1)
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
				update_stat(&IfaceRTU[port_id].stat, &tmpstat);
				if(status==MB_SERIAL_READ_FAILURE) {
					IfaceRTU[port_id].modbus_mode=MODBUS_PORT_ERROR;
					goto EndRun;
					}
				if(exception_on==0) continue;
		  	break;
		  default:;
		  };

///###-----------------------------			
///$$$usleep(2000000);
//			status = mb_tcp_make_adu(&OPC,oDATA,mb_tcp_adu);

			///Обратное отображение адреса в запросе
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

			if(Security.show_data_flow==1)
				show_traffic(TRAFFIC_TCP_SEND, port_id, client_id, serial_adu, serial_adu_len-2);

			status = mb_tcp_send_adu(Client[client_id].csd,
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
	update_stat(&IfaceRTU[port_id].stat, &tmpstat);

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
	//inputDATA->clients[client_id].rc=1;
	pthread_exit (0);	
}

///-----------------------------------------------------------------------------------------------------------------
// Функция ассоциированна с последовательным интерфейсом, реализует режим GATEWAY_PROXY (циклический опрос)
void *iface_rtu_master(void *arg)
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

//	int		tcsd = IfaceRTU[port_id].clients[client_id].csd;
	GW_StaticData tmpstat;
	
	GW_Iface	*inputDATA;
	inputDATA = &IfaceRTU[port_id];
	
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
	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|port_id, 42, IfaceRTU[port_id].modbus_mode, 0, 0, 0);

	int queue_has_query;
	int queue_current;

	while (1) for(i=0; i<=MAX_QUERY_ENTRIES; i++) {

		// status = semop ( semaphore_id, operations, 1);
		status=get_query_from_queue(&IfaceRTU[port_id].queue, &client_id, &device_id, tcp_adu, &tcp_adu_len);

		//queue_has_query=(status==-1)&&(errno==EAGAIN)?0:1;
		queue_has_query=status==-1?0:1;
		if(queue_has_query==1) i=MAX_QUERY_ENTRIES;
		if((queue_has_query!=1)&&(i==MAX_QUERY_ENTRIES)) continue;

		clear_stat(&tmpstat);

	if(i!=MAX_QUERY_ENTRIES) {//формируем следующий запрос из таблицы опроса
		if(query_table[i].port!=port_id) continue; ///!!! нужно реализовать массив с индексами
    if(
			(query_table[i].length==0) ||
			(query_table[i].mbf==0) ||
			(query_table[i].device==0)
			) continue; ///!!! нужно реализовать массив с индексами

		usleep(query_table[i].delay*1000);
		//printf("P%d, query #%d from table\n", port_id+1, i);

		tcp_adu[TCPADU_ADDRESS]=query_table[i].device;
		tcp_adu[TCPADU_FUNCTION]=query_table[i].mbf;

		tcp_adu[TCPADU_START_HI]=(query_table[i].start>>8)&0xff;
		tcp_adu[TCPADU_START_LO]= query_table[i].start&0xff;
		tcp_adu[TCPADU_LEN_HI]=(query_table[i].length>>8)&0xff;
		tcp_adu[TCPADU_LEN_LO]= query_table[i].length&0xff;
																																	
		tcp_adu_len=12;

		} else { //обрабатываем запрос из очереди последовательного порта

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
		///!!! статистику обновляем централизованно в функции int refresh_shm(void *arg) или подобной
		//gate502.wData4x[gate502.status_info+3*port_id+0]++;

		if(Security.show_data_flow==1)
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
				update_stat(&IfaceRTU[port_id].stat, &tmpstat);

		///!!! Место-положение этого бита находится в области памяти 4x шлюза
		//gate502.wData4x[query_table[i].status_register]&=(~query_table[i].status_bit);
				query_table[i].err_counter++;
				if(i!=MAX_QUERY_ENTRIES)
					if(query_table[i].err_counter >= query_table[i].critical)
						query_table[i].status_bit=0; ///!!! добавить сообщение о пропадании связи с modbus-rtu сервером
				continue;
		  	break;
		  default:;
		  };

    /// kazhetsya net zaschity ot perepolneniya bufera priema "serial_adu[]"
	  status = mb_serial_receive_adu(fd, &tmpstat, serial_adu, &serial_adu_len, &tcp_adu[MB_TCP_ADU_HEADER_LEN], inputDATA->serial.timeout, inputDATA->serial.ch_interval_timeout);

///---------- специальный случай при подаче команд на СКС-7 Диоген, обрабатываем
///--- убираем третий с конца байт в полученном ответе на запрос
if((exceptions&EXCEPTION_DIOGEN)!=0)					
		if(serial_adu[RTUADU_FUNCTION]==0x06)
		if((except_prm[0]&(1 << port_id))!=0) {
			serial_adu[serial_adu_len-3]=serial_adu[serial_adu_len-2];
			serial_adu[serial_adu_len-2]=serial_adu[serial_adu_len-1];
			serial_adu_len--;
			//if(status==MB_SERIAL_PDU_ERR) status=0;
			status=0;
			}
///-----------------------------------------------------------------------------

		if(Security.show_data_flow==1)
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
				update_stat(&IfaceRTU[port_id].stat, &tmpstat);

//				if(status==MB_SERIAL_READ_FAILURE) goto EndRun; ///!!!
		///!!! Место-положение этого бита находится в области памяти 4x шлюза
		//gate502.wData4x[query_table[i].status_register]&=(~query_table[i].status_bit);
				query_table[i].err_counter++;
				if(i!=MAX_QUERY_ENTRIES)
					if(query_table[i].err_counter >= query_table[i].critical)
						query_table[i].status_bit=0;  ///!!! добавить сообщение о пропадании связи с modbus-rtu сервером
				continue;
		  	break;
		  default:;
		  };

///###-----------------------------			
	if(i!=MAX_QUERY_ENTRIES) { // сохраняем локально полученные данные
//		for(j=3; j<serial_adu_len-2; j++)
//			oDATA[2*query_table[i].offset+j-3]=serial_adu[j];
		if((serial_adu[RTUADU_FUNCTION]&0x80)==0) {
		switch(serial_adu[RTUADU_FUNCTION]) {

			case MBF_READ_HOLDING_REGISTERS:
				mem_start=MoxaDevice.wData4x;

			case MBF_READ_INPUT_REGISTERS:
				if(serial_adu[RTUADU_FUNCTION]==MBF_READ_INPUT_REGISTERS)
					mem_start=MoxaDevice.wData3x;

				for(j=3; j<serial_adu_len-2; j+=2)
					mem_start[query_table[i].offset+(j-3)/2]=
						(serial_adu[j] << 8) | serial_adu[j+1];

				break;

			case MBF_READ_COILS:
				m_start=MoxaDevice.wData1x;

			case MBF_READ_DECRETE_INPUTS:
				if(serial_adu[RTUADU_FUNCTION]==MBF_READ_DECRETE_INPUTS)
					m_start=MoxaDevice.wData2x;

				for(j=3; j<serial_adu_len-2; j++)
					for(n=0; n<8; n++) {

						mask_src = 0x01 << n; 								// битовая маска в исходном байте
						mask_dst = 0x01 << (query_table[i].offset + n + (j-3)*8) % 8;	// битовая маска в байте назначения
						status = (query_table[i].offset + n + (j-3)*8) / 8;						// индекс байта назначения в массиве

						m_start[status]= serial_adu[j] & mask_src ?\
							m_start[status] | mask_dst:\
							m_start[status] & (~mask_dst);

						}

				break;

			default:;
			}
		///!!! Место-положение этого бита находится в области памяти 4x шлюза
		//gate502.wData4x[query_table[i].status_register]|=query_table[i].status_bit;
		query_table[i].status_bit=1;
		query_table[i].err_counter=0;

  	tmpstat.sended++;
		func_res_ok(serial_adu[RTUADU_FUNCTION], &tmpstat);
		} else { // получено исключение
			func_res_err(serial_adu[RTUADU_FUNCTION], &tmpstat);
			
			query_table[i].err_counter++;
			if(query_table[i].err_counter >= query_table[i].critical)
				query_table[i].status_bit=0;
		  }

		///!!! статистику обновляем централизованно в функции int refresh_shm(void *arg) или подобной
		//gate502.wData4x[gate502.status_info+3*port_id+1]++;

		} else { //отправляем результат запроса непосредственно клиенту

			serial_adu[RTUADU_ADDRESS]=MoxaDevice.modbus_address;
			j=(((serial_adu[RTUADU_START_HI]<<8) | serial_adu[RTUADU_START_LO])&0xffff)+
				query_table[device_id].offset-
				query_table[device_id].start;
			serial_adu[RTUADU_START_HI]=(j>>8)&0xff;
			serial_adu[RTUADU_START_LO]=j&0xff;

			/// определяем тип клиента и соответственно функцию, используемую для отправки ответа
			if(Client[client_id].iface<=SERIAL_P8)
			if(	(IfaceRTU[Client[client_id].iface].modbus_mode==BRIDGE_PROXY) &&
					(Client[client_id].status==GW_CLIENT_RTU_SLV)) {
						
				if(Security.show_data_flow==1)
					show_traffic(TRAFFIC_RTU_SEND, Client[client_id].iface, client_id, serial_adu, serial_adu_len-2);
				///!!! необходимо сделать мьютексы для синхронизации работы нескольких потоков с одним портом
				status = mb_serial_send_adu(IfaceRTU[Client[client_id].iface].serial.fd, &tmpstat, serial_adu, serial_adu_len-2, tcp_adu, &tcp_adu_len);
		
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
					 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_ERR|Client[client_id].iface, 183, (unsigned) status, client_id, 0, 0);
						update_stat(&Client[client_id].stat, &tmpstat);
						update_stat(&IfaceRTU[Client[client_id].iface].stat, &tmpstat);
						continue;
				  	break;
				  default:;
				  };
						
				} else {
	
				if(Security.show_data_flow==1)
					show_traffic(TRAFFIC_TCP_SEND, port_id, client_id, serial_adu, serial_adu_len-2);
	//			status = mb_tcp_send_adu(tcsd, &tmpstat, serial_adu, serial_adu_len-2, tcp_adu, &tcp_adu_len);
				status = mb_tcp_send_adu(Client[client_id].csd,
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
	
			/// при отсутствии записей в очереди переходим к обработке следующей записи из таблицы опроса
			i=device_id;
			}

//	update_stat(&inputDATA->clients[client_id].stat, &tmpstat);
	update_stat(&IfaceRTU[port_id].stat, &tmpstat);

	gettimeofday(&tv2, &tz);
	inputDATA->stat.request_time=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
		///!!! статистику обновляем централизованно в функции int refresh_shm(void *arg) или подобной
	//gate502.wData4x[gate502.status_info+3*port_id+2]=inputDATA->stat.request_time_average;
	}
	EndRun: ;
	// THREAD STOPPED
 	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|port_id, 43, 0, 0, 0, 0);
	pthread_exit (0);	
}
