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

#include "interfaces.h"
#include "moxagate.h"
#include "messages.h"
#include "modbus.h"

///-----------------------------------------------------------------------------------------------------------------
void *iface_tcp_master(void *arg) //РТЙЕН - РЕТЕДБЮБ ДБООЩИ РП Modbus TCP
  {

	u8			tcp_adu[MB_TCP_MAX_ADU_LENGTH];// TCP ADU
	u16			tcp_adu_len;
	u8			serial_adu[MB_SERIAL_MAX_ADU_LEN];/// SERIAL ADU
	u16			serial_adu_len;

///!!! обработку исключений оставляем до момента, когда будет реализован унифицированный ввод/вывод для клиентов и серверов различных типов
//	u8			exception_adu[MB_TCP_MAX_ADU_LENGTH];/// TCP ADU
//	u16			exception_adu_len;

	int		status;

//  printf("port %d cliet %d\n", port_id, client_id);
	GW_StaticData tmpstat;
	
	GW_Iface	*inputDATA;
	inputDATA = (GW_Iface	*) arg;
	//int fd=inputDATA->serial.fd;
  //int port_id=((unsigned)arg)>>8;
  int client_id=0; //=inputDATA->current_client;  ///!!! надо исключить массив tcp_servers
	int q_client, device_id;
	
	struct timeval tv1, tv2;
	struct timezone tz;

	u16			*mem_start;
	u8			*m_start;
	u8 mask_src, mask_dst;

	int i, j, n;
	//inputDATA->clients[client_id].stat.request_time_min=10000; // 10 seconds, must be "this->serial.timeout"
	//inputDATA->clients[client_id].stat.request_time_max=0;
	//inputDATA->clients[client_id].stat.request_time_average=0;
	//for(i=0; i<MAX_LATENCY_HISTORY_POINTS; i++) inputDATA->clients[client_id].stat.latency_history[i]=1000; //ms
	//inputDATA->clients[client_id].stat.clp=0;

	/// semaphore
	struct sembuf operations[1];
	operations[0].sem_num=MAX_MOXA_PORTS*2+client_id;
	operations[0].sem_op=-1;
	operations[0].sem_flg=0;

	semop(semaphore_id, operations, 1);
	inputDATA->queue.operations[0].sem_flg=IPC_NOWAIT;

	int queue_has_query;
	int queue_current;

  // THREAD STARTED
	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|EVENT_SRC_TCPBRIDGE, 42, inputDATA->modbus_mode, client_id, 0, 0);

//	for(i=0; i<IfaceRTU[port_id].accepted_connections_number; i++)
//		bridge_reset_tcp(&IfaceRTU[port_id], i);
	bridge_reset_tcp(inputDATA);

///-----------------------------------------

	while (1) for(i=0; i<=MAX_QUERY_ENTRIES; i++) {

		// status = semop ( semaphore_id, operations, 1);
		status=get_query_from_queue(&inputDATA->queue, &q_client, &device_id, tcp_adu, &tcp_adu_len);

		//queue_has_query=(status==-1)&&(errno==EAGAIN)?0:1;
		queue_has_query=status==-1?0:1;
		if(queue_has_query==1) i=MAX_QUERY_ENTRIES;
		if((queue_has_query!=1)&&(i==MAX_QUERY_ENTRIES)) continue;

		clear_stat(&tmpstat);

	if(i!=MAX_QUERY_ENTRIES) {//формируем следующий запрос из таблицы опроса
		if(query_table[i].port!=MAX_MOXA_PORTS*2+client_id) continue; ///!!! нужно реализовать массив с индексами
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

///-----------------------------------
//		gettimeofday(&tv2, &tz);
//		inputDATA->clients[client_id].stat.scan_rate=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
//		if(inputDATA->clients[client_id].stat.scan_rate>MB_SCAN_RATE_INFINITE)
//		  inputDATA->clients[client_id].stat.scan_rate=MB_SCAN_RATE_INFINITE;
//	inputDATA->clients[client_id].stat.request_time_average=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
//	printf("cycle begins after %d msec\n", inputDATA->clients[client_id].stat.request_time_average);
//		gettimeofday(&tv1, &tz);

//	  pthread_mutex_unlock(&inputDATA->serial_mutex);
	  
///###-----------------------------
//			status = mb_tcp_make_adu(&OPC,oDATA,mb_tcp_adu);

		serial_adu_len=tcp_adu_len;
		for(j=0; j<tcp_adu_len; j++) {
			serial_adu[j]	=tcp_adu[j];
			tcp_adu[j]		=tcp_adu[j+6];
			}
		tcp_adu_len-=6;

    serial_adu[0]=0x03; ///!!!
    serial_adu[1]=0x03; 
    serial_adu[2]=0x00;
    serial_adu[3]=0x00; 
    serial_adu[4]=0x00;
    serial_adu[5]=tcp_adu_len;

//    for(i=0; i<inputDATA->accepted_connections_number; i++)
//      if(inputDATA->clients[i].mb_slave==serial_adu[0]) {
//      	client_id=i;
//      	break;
//        }
//    printf("i=%d, total=%d\n", i, inputDATA->accepted_connections_number);

/*    if(i==inputDATA->accepted_connections_number) {
  		tmpstat.errors_serial_adu++;
	  	tmpstat.errors++;
			update_stat(&inputDATA->clients[client_id].stat, &tmpstat);
			update_stat(&IfaceRTU[port_id].stat, &tmpstat);
			exception_adu_len=9;
			tcp_adu[7]=serial_adu[1]|0x80;
			tcp_adu[8]=0x0a;
      } else  */
		if(Client[0].status!=GW_CLIENT_CLOSED) { // :$
//				exception_adu_len=9;
//				tcp_adu[7]=serial_adu[1]|0x80;
//				tcp_adu[8]=0x0b;
				usleep(4000*1000);
				bridge_reset_tcp(inputDATA);
				continue;
			  }

    //tcp_adu[6]=serial_adu[0]; /// MODBUS DEVICE UID
    
	  ///---------------------
		gettimeofday(&tv1, &tz);
		tmpstat.accepted++;
		///!!! статистику обновляем централизованно в функции int refresh_shm(void *arg) или подобной
		//gate502.wData4x[gate502.status_info+3*port_id+0]++;

		//if(inputDATA->clients[0].status!=MB_CONNECTION_ESTABLISHED) pthread_exit (0);
		
		clear_stat(&tmpstat);
//		exception_adu_len=0;

//		if(exception_adu_len==0) {
			
//	  if(inputDATA->clients[client_id].address_shift!=MB_ADDRESS_NO_SHIFT) {
//	  	status=(serial_adu[2]<<8)|serial_adu[3];
//	  	status+=inputDATA->clients[client_id].address_shift;
//	  	serial_adu[2]=(status>>8)&0xff;
//	  	serial_adu[3]=status&0xff;
//	  	}

		if(Security.show_data_flow==1)
			show_traffic(TRAFFIC_TCP_SEND, MAX_MOXA_PORTS*2+client_id, i, serial_adu, serial_adu_len);

		status = mb_tcp_send_adu(Client[0].csd, &tmpstat, tcp_adu, tcp_adu_len, serial_adu, &serial_adu_len);

//	gettimeofday(&tv2, &tz);
//	inputDATA->clients[client_id].stat.request_time_average=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
//	printf("tcp sended after %d msec\n", inputDATA->clients[client_id].stat.request_time_average);

		switch(status) {
		  case 0:
		  	break;
		  case TCP_COM_ERR_SEND:
		  	tmpstat.errors++;
  			// POLLING: TCP SEND
			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_ERR|EVENT_SRC_TCPBRIDGE, 185, (unsigned) status, client_id, 0, 0);
				shutdown(Client[0].csd, SHUT_RDWR);
				close(Client[0].csd);
				Client[0].csd=-1;
				Client[0].status=GW_CLIENT_ERROR;
//				inputDATA->current_connections_number--;
				if(i!=MAX_QUERY_ENTRIES) query_table[i].status_bit=0;
				continue;
		  	break;
		  default:;
		  };

		status = mb_tcp_receive_adu(Client[0].csd, &tmpstat, tcp_adu, &tcp_adu_len);

		if(Security.show_data_flow==1)
			show_traffic(TRAFFIC_TCP_RECV, client_id, client_id, tcp_adu, tcp_adu_len);
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
			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_ERR|EVENT_SRC_TCPBRIDGE, 184, (unsigned) status, client_id, 0, 0);
				update_stat(&Client[0].stat, &tmpstat);
				update_stat(&inputDATA->stat, &tmpstat);

				shutdown(Client[0].csd, SHUT_RDWR);
				close(Client[0].csd);
				Client[0].csd=-1;
				Client[0].status=GW_CLIENT_ERROR;
				if(i!=MAX_QUERY_ENTRIES) query_table[i].status_bit=0;
				continue;
		  	break;
		  default:;
		  };
		
//		}
			 
		for(j=6; j<tcp_adu_len; j++)
			serial_adu[j-6]	= tcp_adu[j];
		serial_adu_len=tcp_adu_len-6+2;

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

		} /* else { //отправляем результат запроса непосредственно клиенту

			serial_adu[RTUADU_ADDRESS]=gate502.modbus_address;
			j=(((serial_adu[RTUADU_START_HI]<<8) | serial_adu[RTUADU_START_LO])&0xffff)+
				query_table[device_id].offset-
				query_table[device_id].start;
			serial_adu[RTUADU_START_HI]=(j>>8)&0xff;
			serial_adu[RTUADU_START_LO]=j&0xff;

			/// определяем тип клиента и соответственно функцию, используемую для отправки ответа
			if(gate502.clients[client_id].p_num<=SERIAL_P8)
			if(	(IfaceRTU[gate502.clients[client_id].p_num].modbus_mode==BRIDGE_PROXY) &&
					(gate502.clients[client_id].status==MB_CONNECTION_ESTABLISHED)) {
						
				if(gate502.show_data_flow==1)
					show_traffic(TRAFFIC_RTU_SEND, gate502.clients[client_id].p_num, client_id, serial_adu, serial_adu_len-2);
				///!!! необходимо сделать мьютексы для синхронизации работы нескольких потоков с одним портом
				status = mb_serial_send_adu(IfaceRTU[gate502.clients[client_id].p_num].serial.fd, &tmpstat, serial_adu, serial_adu_len-2, tcp_adu, &tcp_adu_len);
		
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
						update_stat(&IfaceRTU[gate502.clients[client_id].p_num].stat, &tmpstat);
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
	
			/// при отсутствии записей в очереди переходим к обработке следующей записи из таблицы опроса
			i=device_id;
			} */

//	update_stat(&inputDATA->clients[client_id].stat, &tmpstat);
	update_stat(&inputDATA->stat, &tmpstat);

	gettimeofday(&tv2, &tz);
	inputDATA->stat.request_time=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
		///!!! статистику обновляем централизованно в функции int refresh_shm(void *arg) или подобной
	//gate502.wData4x[gate502.status_info+3*port_id+2]=inputDATA->stat.request_time_average;
	}

	EndRun: ;
//	close(tcsd);
//	inputDATA->current_connections_number--;
	Client[0].rc=1;
	// THREAD STOPPED
 	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|EVENT_SRC_TCPBRIDGE, 43, 0, client_id, 0, 0);
	pthread_exit (0);	
}

///-----------------------------------------------------------------------------------------------------------------
int bridge_reset_tcp(GW_Iface *bridge)
	{
	if (Client[0].csd < 0)
	  Client[0].csd = socket(AF_INET, SOCK_STREAM, 0);
	if (Client[0].csd < 0) {
		perror("bridge socket");
		//bridge->modbus_mode=MODBUS_PORT_ERROR;
    //strcpy(bridge->bridge_status, "ERR");
		return 1;
		} // else printf("\nPORT2 BRIDGE #%d socket OK...\n", client);

	struct sockaddr_in server;
	//struct hostent *h;
	//h = gethostbyname ("server.domain.ru");
	//memcpy ((char *)kserver.sin_addr,h->h_addr,h->h_length);
	// Определяем семейство протоколов
	server.sin_family = AF_INET;
	// определяем IP-адрес сервера
	//server.sin_addr.s_addr = 0x0a0006f0; // 10.0.0.240
	server.sin_addr.s_addr = IfaceTCP[0].ethernet.ip;
  //	server.sin_addr.s_addr = 0xc00000fc; // 192.0.0.252
	// Определяем порт сервера
	server.sin_port =  htons(IfaceTCP[0].ethernet.port);

	struct timeval tvs, tvr;
	int optlen=sizeof(tvs);
	tvs.tv_sec=0; tvs.tv_usec=500000;
	tvr.tv_sec=0; tvr.tv_usec=500000;

	if(
	(setsockopt(Client[0].csd, SOL_SOCKET, SO_SNDTIMEO, &tvs, optlen)!=0)||
	(setsockopt(Client[0].csd, SOL_SOCKET, SO_RCVTIMEO, &tvr, optlen)!=0)) 
	// SOCKET INITIALIZED
 	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|EVENT_SRC_TCPBRIDGE, 65, 2, 0, 0, 0);
//	printf("for client%d send_timeout=%dms, receive_timeout=%dms\n", client, tvs.tv_sec*1000+tvs.tv_usec/1000, tvr.tv_sec*1000+tvr.tv_usec/1000);

	// Вызов функции connect()
	if(connect(Client[0].csd, (struct sockaddr *)&server, sizeof(server))==-1) {
		perror("bridge tcp connection");																			
		// CONNECTION FAILED
	 	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|EVENT_SRC_TCPBRIDGE, 69, IfaceTCP[0].ethernet.ip, 0, 0, 0);
		return 2;
		} else // CONNECTION ESTABLISHED
		 	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|EVENT_SRC_TCPBRIDGE, 68, IfaceTCP[0].ethernet.ip, 0, 0, 0);


	Client[0].status=GW_CLIENT_CLOSED; /// :)
  //bridge->current_connections_number++;
	time(&Client[0].connection_time);
	return 0;
	}
