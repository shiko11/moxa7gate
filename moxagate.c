/*
MOXA7GATE MODBUS GATEWAY SOFTWARE
SEM-ENGINEERING
                    BRYANSK 2010
*/

#include "global.h"

#include "modbus_tcp.h"

///-----------------------------------------------------------------------------------------------------------------
void *moxa_mb_thread(void *arg) //РТЙЕН - РЕТЕДБЮБ ДБООЩИ РП Modbus TCP
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

			if(status!=0) { // запрос был перенаправлен на другой порт
				if(status!=3) { // учет ошибочных запросов
					tmpstat.accepted++;
					func_res_err(tcp_adu[TCPADU_FUNCTION], &tmpstat);
					update_stat(&gate502.stat, &tmpstat);
					}
				continue;
				}

//		gettimeofday(&tv1, &tz);
		// считаем статистику, только если явно отправляем ответ клиенту
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

//	if(exception_on==1) continue;///!!! обработка ошибки д.б., нужна ли здес эта проверка вообще?

	update_stat(&gate502.stat, &tmpstat);
//	gettimeofday(&tv2, &tz);
// время не считаем, так как вся обработка происходит локально без передачи запроса далее
//	gate502.stat.request_time_average=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
	}

	EndRun: ;
	// THREAD STOPPED
 	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|port_id, 43, 0, 0, 0, 0);
	pthread_exit (0);	
}
///-----------------------------------------------------------------------------------------------------------------
