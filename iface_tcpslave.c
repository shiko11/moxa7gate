/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///*********************** МОДУЛЬ ИНТЕРФЕЙСОВ ШЛЮЗА ****************************
///*** МЕХАНИЗМ ОПРОСА MODBUS TCP SLAVE

///=== INTERFACES_H MODULE IMPLEMENTATION

#include "messages.h"
#include "interfaces.h"
#include "moxagate.h"
#include "modbus.h"

int process_tcpslave_request(int client_id, u8 *adu, u16 adu_len, u8 *memory_adu, u16 *memory_adu_len);

///----------------------------------------------------------------------------
void *iface_tcp_slave(void *arg)
  {
	u8  req_adu[MB_UNIVERSAL_ADU_LEN];
	u16 req_adu_len;
	u8  rsp_adu[MB_UNIVERSAL_ADU_LEN];
	u16 rsp_adu_len;

	int status;

  int port_id=((long)arg)>>8;
  int client_id=((long)arg)&0xff;

	GW_Iface	*tcp_server;

	struct timeval tv1, tv2;
	struct timezone tz;

	tcp_server = &IfaceTCP[port_id];
	
	//tcp_server->clients[client_id].stat.request_time_min=10000; // 10 seconds, must be "this->serial.timeout"
	//tcp_server->clients[client_id].stat.request_time_max=0;
	//tcp_server->clients[client_id].stat.request_time_average=0;
	//for(i=0; i<MAX_LATENCY_HISTORY_POINTS; i++) tcp_server->clients[client_id].stat.latency_history[i]=1000; //ms
	//tcp_server->clients[client_id].stat.clp=0;

  // THREAD STARTED
	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|GATEWAY_LANTCP, IFACE_THREAD_STARTED, tcp_server->modbus_mode, client_id, 0, 0);

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
					// tcp_server->modbus_mode=MODBUS_PORT_ERROR; /// нельзя так делать
					goto EndRun;
					}
				continue;
		  	break;
		  default:;
		  };
	
		status = process_tcpslave_request(client_id, req_adu, req_adu_len, rsp_adu, &rsp_adu_len);

    if(status!=0) { // учет ошибочных запросов
      tcp_server->stat.errors++;
      Client[client_id].stat.errors++;
      func_res_err(req_adu[TCPADU_FUNCTION], &tcp_server->stat);
      func_res_err(req_adu[TCPADU_FUNCTION], &Client[client_id].stat);
      stage_to_stat((MBCOM_REQ<<16) | (MBCOM_FRWD_REQ<<8) | FRWD_TYPE_PROXY, &tcp_server->stat);
      stage_to_stat((MBCOM_REQ<<16) | (MBCOM_FRWD_REQ<<8) | FRWD_TYPE_PROXY, &Client[client_id].stat);
      sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_WRN|GATEWAY_LANTCP, FRWD_TRANS_PQUERY, client_id, (unsigned) status, 0, 0);
      // подготовка ответа - исключения
			rsp_adu[TCPADU_ADDRESS]  = req_adu[TCPADU_ADDRESS];         //device ID
		  rsp_adu[TCPADU_FUNCTION] = req_adu[TCPADU_FUNCTION] | 0x80; /* MODBUS EXCEPTION RESPONSE CODE */
		  rsp_adu[TCPADU_START_HI] = 0x0b;                            /* GATEWAY TARGET DEVICE FAILED TO RESPOND */
		  rsp_adu_len=3;
      }

    rsp_adu[TCPADU_TRANS_HI]=req_adu[TCPADU_TRANS_HI];
    rsp_adu[TCPADU_TRANS_LO]=req_adu[TCPADU_TRANS_LO];
    rsp_adu[TCPADU_PROTO_HI]=req_adu[TCPADU_PROTO_HI];
    rsp_adu[TCPADU_PROTO_LO]=req_adu[TCPADU_PROTO_LO];
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

///-----------------------------------------------------------------------------------------------------------------
// обработка запроса к внутренней памяти шлюза, - формируем ответ для клиента
int process_tcpslave_request(int client_id, u8 *adu, u16 adu_len, u8 *memory_adu, u16 *memory_adu_len)
  {
	u16	*mem_start;
	u8	*m_start;
	u8	 mask_src, mask_dst;

	int	status, k, n, i, j;
	GW_Iface *iface;

	if(Client[client_id].status!=GW_CLIENT_TCP_SLV) return 1;
	iface= &IfaceTCP[Client[client_id].iface];

  // проверка на корректность адресов регистров указанных в запросе
  // (клиент может читать/писать только предопределённые регистры),
	// определение целевой записи в таблице опроса
	status = MAX_QUERY_ENTRIES;
	for(i=0; i<iface->PQueryIndex[MAX_QUERY_ENTRIES]; i++) {
    j = iface->PQueryIndex[i];
		if((PQuery[j].mbf == MBF_READ_HOLDING_REGISTERS) ||
			 (PQuery[j].mbf == MBF_WRITE_MULTIPLE_REGISTERS) ) {
		  n=(adu[TCPADU_START_HI]<<8)|adu[TCPADU_START_LO];
		  k=(adu[TCPADU_LEN_HI]<<8)|adu[TCPADU_LEN_LO];
		  if(((adu[TCPADU_FUNCTION] == MBF_READ_HOLDING_REGISTERS) ||
			    (adu[TCPADU_FUNCTION] == MBF_WRITE_MULTIPLE_REGISTERS) )
				 && (PQuery[j].start <= n)
				 && ((PQuery[j].start+PQuery[j].length) >= (n+k))
				 ) {
			  status = j; // номер целевой записи в таблице опроса
				break;
			  }
		  }
	  }
	if(status == MAX_QUERY_ENTRIES) return 3; // текущий запрос не корректен

	switch(adu[TCPADU_FUNCTION]) {

		case MBF_READ_COILS:
			if(MoxaDevice.amount1xStatus==0) return 4;
			m_start=MoxaDevice.wData1x;

		case MBF_READ_DECRETE_INPUTS:
			if(adu[TCPADU_FUNCTION]==MBF_READ_DECRETE_INPUTS) {
				if(MoxaDevice.amount2xStatus==0) return 5;
				m_start=MoxaDevice.wData2x;												 
				}

			j=(adu[TCPADU_START_HI]<<8)|adu[TCPADU_START_LO];
			k=(adu[TCPADU_LEN_HI]<<8)|adu[TCPADU_LEN_LO];

			memory_adu[TCPADU_ADDRESS]=adu[TCPADU_ADDRESS]; //device ID
			memory_adu[TCPADU_FUNCTION]=adu[TCPADU_FUNCTION]; //ModBus Function Code
			memory_adu[TCPADU_BYTES]=(k-1)/8+1; 				//bytes total

			for(n=0; n<memory_adu[TCPADU_BYTES]; n++)
				for(i=0; i<8; i++) {

					mask_dst = 0x01 << i;									// битовая маска в байте назначения
					mask_src = 0x01 << (j + i + n*8) % 8; // битовая маска в исходном байте
					status = (j + i + n*8) / 8;						// индекс исходного байта в массиве

				// в связи с отображением таблицы 2х на таблицу 4х требуется иной порядок формирования ответа.
				// надо реализовать перемещение по байтам в памяти, учитывая что данные хранятся в блоках по
				// 16 бит, т.е. нужно отправлять сначала младшие 8 бит, затем старшие 8 бит. добавлена 1 строка:
					if(MoxaDevice.map2Xto4X==1) status=status%2?status-1:status+1;

	        memory_adu[9+n] = m_start[status] & mask_src ?
						memory_adu[9+n] | mask_dst :
						memory_adu[9+n] & (~mask_dst);

					}

			*memory_adu_len=memory_adu[TCPADU_BYTES]+3;
			//*memory_adu_len+=TCPADU_ADDRESS;
    	break;

		case MBF_READ_HOLDING_REGISTERS:
			if(MoxaDevice.amount4xRegisters==0) return 7;
			mem_start=MoxaDevice.wData4x;

		case MBF_READ_INPUT_REGISTERS:
			if(adu[TCPADU_FUNCTION]==MBF_READ_INPUT_REGISTERS) {
				if(MoxaDevice.amount3xRegisters==0) return 6;
				mem_start=MoxaDevice.wData3x;												
				}

			j=(adu[TCPADU_START_HI]<<8)|adu[TCPADU_START_LO];
			k=(adu[TCPADU_LEN_HI]<<8)|adu[TCPADU_LEN_LO];

      j = j - PQuery[status].start + PQuery[status].offset;

			memory_adu[TCPADU_ADDRESS]=adu[TCPADU_ADDRESS];   // device ID
			memory_adu[TCPADU_FUNCTION]=adu[TCPADU_FUNCTION]; // ModBus Function Code
			memory_adu[TCPADU_BYTES]=2*k; 				            // bytes total

	    pthread_mutex_lock(&iface->serial_mutex);
			for(n=0; n<k; n++) {
        memory_adu[9+2*n]		=(mem_start[j+n]>>8)&0xff;
        memory_adu[9+2*n+1]	=	mem_start[j+n]&0xff;
				}
	    pthread_mutex_unlock(&iface->serial_mutex);

			*memory_adu_len=2*k+3;
			//*memory_adu_len+=TCPADU_ADDRESS;
    	break;

//  case MBF_WRITE_MULTIPLE_COILS:
		case MBF_WRITE_MULTIPLE_REGISTERS:
			if(MoxaDevice.amount4xRegisters==0) return 9;
			mem_start=MoxaDevice.wData4x;
			k = ((adu[TCPADU_LEN_HI]<<8)|adu[TCPADU_LEN_LO])&0xffff;
			if(adu[TCPADU_BYTECOUNT] != 2*k) return 10;
			if(adu_len != TCPADU_ADDRESS + 7 + 2*k) return 11;
//  case MBF_WRITE_SINGLE_COIL:
//  case MBF_WRITE_SINGLE_REGISTER:
//  	if(	(adu[TCPADU_FUNCTION]==MBF_WRITE_SINGLE_REGISTER) ||
//  			(adu[TCPADU_FUNCTION]==MBF_WRITE_SINGLE_COIL)) k=1;

			j=(adu[TCPADU_START_HI]<<8)|adu[TCPADU_START_LO];
      j = j - PQuery[status].start + PQuery[status].offset;

      pthread_mutex_lock(&iface->serial_mutex);
      for(n=0; n<k; n++) mem_start[j+n] = ((adu[13+2*n]<<8)|adu[13+2*n+1])&0xffff;
      pthread_mutex_unlock(&iface->serial_mutex);

			// формируем ответ
			memory_adu[TCPADU_ADDRESS] =adu[TCPADU_ADDRESS ];
			memory_adu[TCPADU_FUNCTION]=adu[TCPADU_FUNCTION];
			memory_adu[TCPADU_START_HI]=adu[TCPADU_START_HI];
			memory_adu[TCPADU_START_LO]=adu[TCPADU_START_LO];
			memory_adu[TCPADU_LEN_HI]  =adu[TCPADU_LEN_HI  ];
			memory_adu[TCPADU_LEN_LO]  =adu[TCPADU_LEN_LO  ];
	
			*memory_adu_len=6;
    	break;

		default: //!!! добавить код счетчика статистики. уже не первая проверка по пути пакета
		 		// POLLING: FUNC NOT SUPPORTED
		 		sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|GATEWAY_MOXAGATE, 180, adu[TCPADU_FUNCTION], 0, 0, 0);
				return 2;
		}

	return 0;
	}

