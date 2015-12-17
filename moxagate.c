/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///*********************** МОДУЛЬ УСТРОЙСТВА MODBUS MOXA UC-7410 ***************
///*** МЕХАНИЗМ ВНУТРЕННЕЙ ПАМЯТИ
///*** СТРУКТУРЫ ДАННЫХ ДЛЯ РЕАЛИЗАЦИИ ФУНКЦИЙ HMI-ВЗАИМОДЕЙСТВИЯ

///=== MOXAGATE_H IMPLEMENTATION

#include <string.h>
#include <stdlib.h>

#include "interfaces.h"
#include "moxagate.h"
#include "modbus.h"
#include "messages.h"

///----------------------------------------------------------------------------
int init_moxagate_h()
  {

	MoxaDevice.start_time=0;

  MoxaDevice.modbus_address=1;
  /// должен быть ноль, т.к. это индекс. в настоящее время декрементируется при инициализации:
  MoxaDevice.status_info=0;

	// MoxaDevice.queue
  clear_stat(&MoxaDevice.stat);

  MoxaDevice.map2Xto4X=0;
  MoxaDevice.map3Xto4X=0;

  // MoxaDevice.moxa_mutex

  // начало блока внутренних регистров Moxa (смещение)
  MoxaDevice.offset1xStatus =
  MoxaDevice.offset2xStatus =
  MoxaDevice.offset3xRegisters =
  MoxaDevice.offset4xRegisters = 0;

  // количество элементов в каждой из таблиц MODBUS
  MoxaDevice.amount1xStatus =
  MoxaDevice.amount2xStatus =
  MoxaDevice.amount3xRegisters =
  MoxaDevice.amount4xRegisters = 0;

  MoxaDevice.used1xStatus =
  MoxaDevice.used2xStatus =
  MoxaDevice.used3xRegisters =
  MoxaDevice.used4xRegisters = 0;

  // указатели на массивы памяти
  MoxaDevice.wData1x=(u8 *) NULL;
  MoxaDevice.wData2x=(u8 *) NULL;
  MoxaDevice.wData3x=(u16 *) NULL;
  MoxaDevice.wData4x=(u16 *) NULL;

  // очередь запросов интерфейса
  init_queue(&MoxaDevice.queue, IFACE_MOXAGATE);

  return 0;
  }

///----------------------------------------------------------------------------
int init_moxagate_memory()
  {
  unsigned int k;																	

  // выделение памяти под таблицу 1x
	if(MoxaDevice.amount1xStatus>0) {
		k=sizeof(u8)*((MoxaDevice.amount1xStatus-1)/8+1);
		MoxaDevice.wData1x=(u8 *) malloc(k);
		if(MoxaDevice.wData1x==NULL) {
			sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, MOXAGATE_MBTABLE_ALLOCATED, 1, k, 0, 0);
			return 1;
			}
		memset(MoxaDevice.wData1x,0, k);
		sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|GATEWAY_SYSTEM, MOXAGATE_MBTABLE_ALLOCATED, 1, k, 0, 0);
		}

  // выделение памяти под таблицу 2x
	if((MoxaDevice.amount2xStatus>0) && (MoxaDevice.map2Xto4X==0)) {
		k=sizeof(u8)*((MoxaDevice.amount2xStatus-1)/8+1);
		MoxaDevice.wData2x=(u8 *) malloc(k);
		if(MoxaDevice.wData2x==NULL) {
			sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, MOXAGATE_MBTABLE_ALLOCATED, 2, k, 0, 0);
			return 1;
			}
		memset(MoxaDevice.wData2x,0, k);
		sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|GATEWAY_SYSTEM, MOXAGATE_MBTABLE_ALLOCATED, 2, k, 0, 0);
		}

  // выделение памяти под таблицу 3x
	if((MoxaDevice.amount3xRegisters>0) && (MoxaDevice.map3Xto4X==0)) {
		k=sizeof(u16)*MoxaDevice.amount3xRegisters;
		MoxaDevice.wData3x=(u16 *) malloc(k);
		if(MoxaDevice.wData3x==NULL) {
			sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, MOXAGATE_MBTABLE_ALLOCATED, 3, k, 0, 0);
			return 1;
			}
		memset(MoxaDevice.wData3x, 0, k);
		sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|GATEWAY_SYSTEM, MOXAGATE_MBTABLE_ALLOCATED, 3, k, 0, 0);
		}

  // выделение памяти под таблицу 4x
	if(MoxaDevice.amount4xRegisters>0) {
		k=sizeof(u16)*MoxaDevice.amount4xRegisters;
		MoxaDevice.wData4x=(u16 *) malloc(k);
		if(MoxaDevice.wData4x==NULL) {
			sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, MOXAGATE_MBTABLE_ALLOCATED, 4, k, 0, 0);
			return 1;
			}
		memset(MoxaDevice.wData4x, 0, k);
		sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|GATEWAY_SYSTEM, MOXAGATE_MBTABLE_ALLOCATED, 4, k, 0, 0);

		if(MoxaDevice.map2Xto4X!=0) {
			// отображаем таблицу дискретных входов на таблицу holding-регистров
			MoxaDevice.offset2xStatus=MoxaDevice.offset4xRegisters*sizeof(u16)*8;
			MoxaDevice.amount2xStatus=MoxaDevice.amount4xRegisters*sizeof(u16)*8;
			MoxaDevice.wData2x=(u8 *) MoxaDevice.wData4x;
			sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|GATEWAY_SYSTEM, MOXAGATE_MBTABLE_ALLOCATED, 2, k, 0, 0);
		  }

		if(MoxaDevice.map3Xto4X!=0) {
			// отображаем таблицу input-регистров на таблицу holding-регистров
			MoxaDevice.offset3xRegisters=MoxaDevice.offset4xRegisters;
			MoxaDevice.amount3xRegisters=MoxaDevice.amount4xRegisters;
			MoxaDevice.wData3x=MoxaDevice.wData4x;
		        sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|GATEWAY_SYSTEM, MOXAGATE_MBTABLE_ALLOCATED, 3, k, 0, 0);
		  }
		}

  // мьютекс используется для синхронизации потоков при работе с памятью
	pthread_mutex_init(&MoxaDevice.moxa_mutex, NULL);

	// идентификаторы объектов для вывода детальной статистики в блок диагностики шлюза
  MoxaDevice.wData4x[MoxaDevice.status_info + GWINF_STATDETAILS_1] = 0xffff;
  MoxaDevice.wData4x[MoxaDevice.status_info + GWINF_STATDETAILS_2] = 0xffff;
  MoxaDevice.wData4x[MoxaDevice.status_info + GWINF_STATDETAILS_3] = 0xffff;
  MoxaDevice.wData4x[MoxaDevice.status_info + GWINF_STATDETAILS_4] = 0xffff;

  return 0;
  }

///----------------------------------------------------------------------------
void *moxa_device(void *arg) //РТЙЕН - РЕТЕДБЮБ ДБООЩИ РП Modbus TCP
  {
	u8  req_adu[MB_UNIVERSAL_ADU_LEN];
	u16 req_adu_len;
	u8  rsp_adu[MB_UNIVERSAL_ADU_LEN];
	u16 rsp_adu_len;

	int		status;
	int		client_id, device_id;

	struct timeval tv1, tv2;
	struct timezone tz;

///!!!
//	inputDATA->clients[client_id].stat.request_time_min=10000; // 10 seconds, must be "this->serial.timeout"
//	inputDATA->clients[client_id].stat.request_time_max=0;
//	inputDATA->clients[client_id].stat.request_time_average=0;
//	for(i=0; i<MAX_LATENCY_HISTORY_POINTS; i++) inputDATA->clients[client_id].stat.latency_history[i]=1000; //ms
//	inputDATA->clients[client_id].stat.clp=0;

	MoxaDevice.queue.operations[0].sem_op=-1;
	semop(semaphore_id, MoxaDevice.queue.operations, 1);
	
//	int fd=inputDATA->serial.fd;

  // THREAD STARTED
	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|GATEWAY_MOXAGATE, IFACE_THREAD_STARTED, GATEWAY_MOXAGATE, 0, 0, 0);

	while (1) {
		
		// в отличие от прочих потоковых функций, здесь поток блокируется семафором
		status=get_query_from_queue(&MoxaDevice.queue, &client_id, &device_id, req_adu, &req_adu_len);

		if(status==1) { // внутренняя ошибка в программе
			MoxaDevice.stat.accepted++;
			MoxaDevice.stat.errors++;
			MoxaDevice.stat.frwd_queue_out++;
		  }

		if(status!=0) continue;

		MoxaDevice.stat.accepted++;

		status = process_moxamb_request(client_id, req_adu, req_adu_len, rsp_adu, &rsp_adu_len);

		if(status!=0) { // запрос был перенаправлен на другой порт

			if(status!=3) { // учет ошибочных запросов
				MoxaDevice.stat.errors++;
				Client[client_id].stat.errors++;
				func_res_err(req_adu[TCPADU_FUNCTION], &MoxaDevice.stat);
				func_res_err(req_adu[TCPADU_FUNCTION], &Client[client_id].stat);
				stage_to_stat((MBCOM_REQ<<16) | (MBCOM_FRWD_REQ<<8) | FRWD_TYPE_PROXY, &MoxaDevice.stat);
				stage_to_stat((MBCOM_REQ<<16) | (MBCOM_FRWD_REQ<<8) | FRWD_TYPE_PROXY, &Client[client_id].stat);

			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_WRN|GATEWAY_MOXAGATE, FRWD_TRANS_PQUERY, client_id,(unsigned) status,  0, 0);
				continue;
				}

			MoxaDevice.stat.frwd_p++;
			//Client[client_id].stat.frwd_p++; // этот тип перенаправления уже был учтен ранее
			func_res_ok(req_adu[TCPADU_FUNCTION], &MoxaDevice.stat);
			//func_res_ok(req_adu[TCPADU_FUNCTION], &Client[client_id].stat); // запрос пошел дальше
			continue;
			}

//		gettimeofday(&tv1, &tz);
		// считаем статистику, только если явно отправляем ответ клиенту

//------------------------
			rsp_adu[TCPADU_TRANS_HI]=req_adu[TCPADU_TRANS_HI];
			rsp_adu[TCPADU_TRANS_LO]=req_adu[TCPADU_TRANS_LO];
			rsp_adu[TCPADU_PROTO_HI]=req_adu[TCPADU_PROTO_HI];
			rsp_adu[TCPADU_PROTO_LO]=req_adu[TCPADU_PROTO_LO];
			rsp_adu[TCPADU_SIZE_HI ]=(rsp_adu_len-TCPADU_ADDRESS)>>8;
			rsp_adu[TCPADU_SIZE_LO ]=(rsp_adu_len-TCPADU_ADDRESS)&0xff;

			if(Security.show_data_flow==1)
				show_traffic(TRAFFIC_TCP_SEND, GATEWAY_MOXAGATE, client_id, rsp_adu, rsp_adu_len);

			status = mbcom_tcp_send(Client[client_id].csd,
															rsp_adu,
															rsp_adu_len);

		switch(status) {
		  case 0:
				MoxaDevice.stat.sended++;
				Client[client_id].stat.sended++;
				func_res_ok(rsp_adu[TCPADU_FUNCTION], &MoxaDevice.stat);
				func_res_ok(rsp_adu[TCPADU_FUNCTION], &Client[client_id].stat);
		  	break;

		  case TCP_COM_ERR_SEND:

				MoxaDevice.stat.errors++;
				Client[client_id].stat.errors++;
				func_res_err(rsp_adu[TCPADU_FUNCTION], &MoxaDevice.stat);
				func_res_err(rsp_adu[TCPADU_FUNCTION], &Client[client_id].stat);
				stage_to_stat((MBCOM_RSP<<16) | (MBCOM_TCP_SEND<<8) | status, &MoxaDevice.stat);
				stage_to_stat((MBCOM_RSP<<16) | (MBCOM_TCP_SEND<<8) | status, &Client[client_id].stat);

			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_WRN|GATEWAY_MOXAGATE, POLL_TCP_SEND, (unsigned) status, client_id, 0, 0);

		  	break;

		  default:;
		  };

//	gettimeofday(&tv2, &tz);
// время не считаем, так как вся обработка происходит локально без передачи запроса далее
//	gate502.stat.request_time_average=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
	}

	EndRun: ;
	// THREAD STOPPED
 	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|GATEWAY_MOXAGATE, 43, 0, 0, 0, 0);
	pthread_exit (0);	
}
///-----------------------------------------------------------------------------------------------------------------
// обработка запроса к внутренней памяти шлюза, - формируем ответ для клиента или перенаправляем дальше
int process_moxamb_request(int client_id, u8 *adu, u16 adu_len, u8 *memory_adu, u16 *memory_adu_len)
  {
	static u16	*mem_start;
	static u8		*m_start;
	static u8		mask_src, mask_dst;

	static int	status, k, n, i, j;
	static int device_id, port_id;
	static GW_Iface *iface;

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
			*memory_adu_len+=TCPADU_ADDRESS;
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

			memory_adu[TCPADU_ADDRESS]=adu[TCPADU_ADDRESS]; //device ID
			memory_adu[TCPADU_FUNCTION]=adu[TCPADU_FUNCTION]; //ModBus Function Code
			memory_adu[TCPADU_BYTES]=2*k; 				//bytes total

			for(n=0; n<k; n++) {
        memory_adu[9+2*n]		=(mem_start[j+n]>>8)&0xff;
        memory_adu[9+2*n+1]	=	mem_start[j+n]&0xff;
				}
			*memory_adu_len=2*k+3;
			*memory_adu_len+=TCPADU_ADDRESS;
    	break;

		case MBF_WRITE_MULTIPLE_COILS:
		case MBF_WRITE_MULTIPLE_REGISTERS:
			n = ((adu[TCPADU_LEN_HI]<<8)|adu[TCPADU_LEN_LO])&0xffff;
		case MBF_WRITE_SINGLE_COIL:
		case MBF_WRITE_SINGLE_REGISTER:
			if(	(adu[TCPADU_FUNCTION]==MBF_WRITE_SINGLE_REGISTER) ||
					(adu[TCPADU_FUNCTION]==MBF_WRITE_SINGLE_COIL)) n=1;

			j=(adu[TCPADU_START_HI]<<8)|adu[TCPADU_START_LO];

			// если записываются предопределенные регистры внутри блока диагностики шлюза
			if((adu[TCPADU_FUNCTION]==MBF_WRITE_SINGLE_REGISTER)&&( (j==GWINF_STATDETAILS_1) ||
			                                                        (j==GWINF_STATDETAILS_2) ||
			                                                        (j==GWINF_STATDETAILS_3) ||
			                                                        (j==GWINF_STATDETAILS_4) )) {

				MoxaDevice.wData4x[j] = (adu[TCPADU_LEN_HI]<<8)|adu[TCPADU_LEN_LO]; // записываемое значение

				// формируем ответ
				memory_adu[TCPADU_ADDRESS] =adu[TCPADU_ADDRESS ];
				memory_adu[TCPADU_FUNCTION]=adu[TCPADU_FUNCTION];
				memory_adu[TCPADU_START_HI]=adu[TCPADU_START_HI];
				memory_adu[TCPADU_START_LO]=adu[TCPADU_START_LO];
				memory_adu[TCPADU_LEN_HI]  =adu[TCPADU_LEN_HI  ];
				memory_adu[TCPADU_LEN_LO]  =adu[TCPADU_LEN_LO  ];
		
				*memory_adu_len=6+TCPADU_ADDRESS;

				break;
			  }

			status=translateProxyDevice(j, n, &port_id, &device_id);

		  if(status!=FRWD_TYPE_PROXY) {
		 		// FRWD: PROXY TRANSLATION
		 		sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|GATEWAY_MOXAGATE, 130, client_id, ((adu[TCPADU_START_HI]<<8)|adu[TCPADU_START_LO])&0xffff, n, 0);
				return 1;
				}
														
			/// ставим запрос в очередь MASTER-интерфейса
			i = port_id-GATEWAY_T01;
			iface= port_id<=GATEWAY_P8? &IfaceRTU[port_id]: &IfaceTCP[i];

			///!!! отправка исключения клиенту в случае запрета на запись данной области памяти внутри шлюза

			status=enqueue_query_ex(&iface->queue, client_id, (FRWD_TYPE_PROXY<<8)|(device_id&0xff), adu, adu_len);
			return 3;

			break;

		default: //!!! добавить код счетчика статистики. уже не первая проверка по пути пакета
		 		// POLLING: FUNC NOT SUPPORTED
		 		sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|GATEWAY_MOXAGATE, 180, adu[TCPADU_FUNCTION], 0, 0, 0);
				return 2;
		}

	return 0;
	}
///----------------------------------------------------------------------------------------------------------------
// формирование запроса на основе записи из таблицы опроса
void create_proxy_request(int index, u8 *tcp_adu, u16 *tcp_adu_len)
  {
	u16 *mem_start;
	int j;

        tcp_adu[TCPADU_ADDRESS]=	 PQuery[index].device;
	tcp_adu[TCPADU_FUNCTION]=	 PQuery[index].mbf;

	tcp_adu[TCPADU_START_HI]=(PQuery[index].start >>8)&0xff;
	tcp_adu[TCPADU_START_LO]= PQuery[index].start     &0xff;
	tcp_adu[TCPADU_LEN_HI]=  (PQuery[index].length>>8)&0xff;
	tcp_adu[TCPADU_LEN_LO]=   PQuery[index].length    &0xff;

	*tcp_adu_len=12;

    if(PQuery[index].access == QT_ACCESS_WRITEONLY) { // MBF_WRITE_MULTIPLE_REGISTERS 

      tcp_adu[TCPADU_BYTECOUNT] = 2 * PQuery[index].length;
      mem_start=MoxaDevice.wData4x;
      for(j=0; j<tcp_adu[TCPADU_BYTECOUNT]; j++)
        tcp_adu[TCPADU_BYTECOUNT+1+j] = j%2==0 ?
	  (mem_start[PQuery[index].offset+j/2] >> 8)& 0xff :
           mem_start[PQuery[index].offset+j/2]      & 0xff ;

      *tcp_adu_len += 2 * PQuery[index].length + 1;

      }

  return;
  }
///-----------------------------------------------------------------------------------------------------------------
// обработка ответа на запрос из таблицы опроса
void process_proxy_response(int index, u8 *tcp_adu, u16 tcp_adu_len)
  {
	u16 *mem_start;
	u8  *m_start;
	u8 mask_src, mask_dst;
	int j, status, n;

	if((tcp_adu[TCPADU_FUNCTION]&0x80)==0) {
	switch(tcp_adu[TCPADU_FUNCTION]) {

		case MBF_READ_HOLDING_REGISTERS:
			mem_start=MoxaDevice.wData4x;

		case MBF_READ_INPUT_REGISTERS:
			if(tcp_adu[TCPADU_FUNCTION]==MBF_READ_INPUT_REGISTERS)
				mem_start=MoxaDevice.wData3x;

			for(j=9; j<tcp_adu_len; j+=2)
				mem_start[PQuery[index].offset+(j-9)/2]=
					(tcp_adu[j] << 8) | tcp_adu[j+1];

			break;

		case MBF_READ_COILS:
			m_start=MoxaDevice.wData1x;

		case MBF_READ_DECRETE_INPUTS:
			if(tcp_adu[TCPADU_FUNCTION]==MBF_READ_DECRETE_INPUTS)
				m_start=MoxaDevice.wData2x;

			for(j=9; j<tcp_adu_len; j++)
				for(n=0; n<8; n++) {

					mask_src = 0x01 << n; 								// битовая маска в исходном байте
					mask_dst = 0x01 << (PQuery[index].offset + n + (j-9)*8) % 8;	// битовая маска в байте назначения
					status = (PQuery[index].offset + n + (j-9)*8) / 8;						// индекс байта назначения в массиве

					m_start[status]= tcp_adu[j] & mask_src ?
						m_start[status] | mask_dst :
						m_start[status] & (~mask_dst);

					}

			break;

		case MBF_WRITE_MULTIPLE_REGISTERS:

	          if(((tcp_adu[TCPADU_START_HI] << 8) | tcp_adu[TCPADU_START_LO]) != PQuery[index].start ||
	             ((tcp_adu[TCPADU_LEN_HI  ] << 8) | tcp_adu[TCPADU_LEN_LO  ]) != PQuery[index].length   ) {

	            PQuery[index].err_counter++;
	            if(PQuery[index].err_counter >= PQuery[index].critical) PQuery[index].status_bit=0;
	            }
           

			break;

		default:;
		}

	PQuery[index].status_bit=1;
	PQuery[index].err_counter=0;

	} else { // получено исключение
		
		PQuery[index].err_counter++;
		if(PQuery[index].err_counter >= PQuery[index].critical)
			PQuery[index].status_bit=0;

	  }

	return;
	}
///-----------------------------------------------------------------------------------------------------------------
// подготовка новой TCP ADU для отправки на TCP сервер
void make_tcp_adu(u8 *tcp_adu, int length)
  {
	static unsigned int transaction=1;

	tcp_adu[TCPADU_TRANS_HI]=(transaction>>8)&0xff;
	tcp_adu[TCPADU_TRANS_LO]=transaction&0xff;
	tcp_adu[TCPADU_PROTO_HI]=0;
	tcp_adu[TCPADU_PROTO_LO]=0;
	tcp_adu[TCPADU_SIZE_HI ]=(length>>8)&0xff;
	tcp_adu[TCPADU_SIZE_LO ]=length&0xff;

	transaction = transaction<0xffff ? transaction+1 : 1;

	return;
	}
///-----------------------------------------------------------------------------------------------------------------
int refresh_status_info()
	{
	int i, j, k;
	GW_StaticData *stat;

	if(MoxaDevice.wData4x==NULL) return 1;

	// состояние последовательных интерфейсов P1 - P8
  for(i=0; i<MAX_MOXA_PORTS; i++)
  	MoxaDevice.wData4x[MoxaDevice.status_info + GWINF_STATE_IFACERTU + i] = IfaceRTU[i].modbus_mode;
  // состояние Ethernet-интерфейсов T01 - T32
  for(i=0; i<MAX_TCP_SERVERS; i++)
  	MoxaDevice.wData4x[MoxaDevice.status_info + GWINF_STATE_IFACETCP + i] = IfaceTCP[i].modbus_mode;
  // состояние клиентских соединений 1 - 32
  for(i=0; i<MOXAGATE_CLIENTS_NUMBER; i++)
  	MoxaDevice.wData4x[MoxaDevice.status_info + GWINF_STATE_CLIENTS + i] = Client[i].status;

	// флаги статуса связи блоков таблицы опроса
#ifndef MOXA7GATE_KM400
  for(i=0; i<MAX_QUERY_ENTRIES; i++) {
		j = 0x01 << (i % 16);
    if(PQuery[i].status_bit==0)
			MoxaDevice.wData4x[MoxaDevice.status_info + GWINF_PROXY_STATUS + (i/16)]&=~j;
			else
			MoxaDevice.wData4x[MoxaDevice.status_info + GWINF_PROXY_STATUS + (i/16)]|= j;
    }
#else
	j = 0x01 << (0 % 16);
  if(PQuery[0].status_bit==0)
		MoxaDevice.wData4x[MoxaDevice.status_info + GWINF_PROXY_STATUS]&=~j;
		else
		MoxaDevice.wData4x[MoxaDevice.status_info + GWINF_PROXY_STATUS]|= j;

	j = 0x01 << (1 % 16);
  if(VSlave[0].status_bit==0)
		MoxaDevice.wData4x[MoxaDevice.status_info + GWINF_PROXY_STATUS]&=~j;
		else
		MoxaDevice.wData4x[MoxaDevice.status_info + GWINF_PROXY_STATUS]|= j;

	j = 0x01 << (4 % 16);
  if(PQuery[1].status_bit==0)
		MoxaDevice.wData4x[MoxaDevice.status_info + GWINF_PROXY_STATUS]&=~j;
		else
		MoxaDevice.wData4x[MoxaDevice.status_info + GWINF_PROXY_STATUS]|= j;

	j = 0x01 << (5 % 16);
  if(VSlave[1].status_bit==0)
		MoxaDevice.wData4x[MoxaDevice.status_info + GWINF_PROXY_STATUS]&=~j;
		else
		MoxaDevice.wData4x[MoxaDevice.status_info + GWINF_PROXY_STATUS]|= j;

#endif

  // регистр-счетчик циклов сканирования
  MoxaDevice.wData4x[MoxaDevice.status_info + GWINF_SCAN_COUNTER] = Security.scan_counter/1000;
	
  // статистика TCP-соединений
  MoxaDevice.wData4x[MoxaDevice.status_info + GWINF_CONSTAT_M7G + 0] = Security.accepted_connections_number;
  MoxaDevice.wData4x[MoxaDevice.status_info + GWINF_CONSTAT_M7G + 1] = Security.current_connections_number;
  MoxaDevice.wData4x[MoxaDevice.status_info + GWINF_CONSTAT_M7G + 2] = Security.rejected_connections_number;
  for(i=0; i<MAX_MOXA_PORTS; i++) {
	  MoxaDevice.wData4x[MoxaDevice.status_info + GWINF_CONSTAT_RTU + 3*i] = IfaceRTU[i].Security.accepted_connections_number;
	  MoxaDevice.wData4x[MoxaDevice.status_info + GWINF_CONSTAT_RTU + 3*i] = IfaceRTU[i].Security.current_connections_number;
	  MoxaDevice.wData4x[MoxaDevice.status_info + GWINF_CONSTAT_RTU + 3*i] = IfaceRTU[i].Security.rejected_connections_number;
    }
  
  // детальная статистика
  for(i=0; i<4; i++) {
  	
		stat=NULL;

  	switch(i) {
  		case 0: j=GWINF_STATDETAILS_1; break;
  		case 1: j=GWINF_STATDETAILS_2; break;
  		case 2: j=GWINF_STATDETAILS_3; break;
  		case 3: j=GWINF_STATDETAILS_4; break;
  		default:;
  	  }

  	if(MoxaDevice.wData4x[MoxaDevice.status_info + j]<=GATEWAY_P8)
  	  stat=&IfaceRTU[MoxaDevice.wData4x[MoxaDevice.status_info + j]].stat;

  	if(MoxaDevice.wData4x[MoxaDevice.status_info + j]==GATEWAY_MOXAGATE)
  	  stat=&Security.stat;

  	if( (MoxaDevice.wData4x[MoxaDevice.status_info + j]>=GATEWAY_T01) &&
  	    (MoxaDevice.wData4x[MoxaDevice.status_info + j]<=GATEWAY_T32)
  	  ) stat=&IfaceTCP[MoxaDevice.wData4x[MoxaDevice.status_info + j] - GATEWAY_T01].stat;

  	if( (MoxaDevice.wData4x[MoxaDevice.status_info + j]>=100) &&
  	    (MoxaDevice.wData4x[MoxaDevice.status_info + j]<=131)
  	  ) stat=&Client[MoxaDevice.wData4x[MoxaDevice.status_info + j] - 100].stat;

		if(stat==NULL) continue;
		j+=1;

	  MoxaDevice.wData4x[MoxaDevice.status_info + j + 0] = stat->tcp_req_recv;
	  MoxaDevice.wData4x[MoxaDevice.status_info + j + 1] = stat->tcp_req_adu;
	  MoxaDevice.wData4x[MoxaDevice.status_info + j + 2] = stat->tcp_req_pdu;
	  MoxaDevice.wData4x[MoxaDevice.status_info + j + 3] = stat->rtu_req_recv;
	  MoxaDevice.wData4x[MoxaDevice.status_info + j + 4] = stat->rtu_req_crc;
	  MoxaDevice.wData4x[MoxaDevice.status_info + j + 5] = stat->rtu_req_adu;
	  MoxaDevice.wData4x[MoxaDevice.status_info + j + 6] = stat->rtu_req_pdu;
	
	  MoxaDevice.wData4x[MoxaDevice.status_info + j + 7] = stat->frwd_proxy;
	  MoxaDevice.wData4x[MoxaDevice.status_info + j + 8] = stat->frwd_atm;
	  MoxaDevice.wData4x[MoxaDevice.status_info + j + 9] = stat->frwd_rtm;
	  MoxaDevice.wData4x[MoxaDevice.status_info + j +10] = stat->frwd_queue_in;
	  MoxaDevice.wData4x[MoxaDevice.status_info + j +11] = stat->frwd_queue_out;
	
	  MoxaDevice.wData4x[MoxaDevice.status_info + j +12] = stat->rtu_req_send;
	  MoxaDevice.wData4x[MoxaDevice.status_info + j +13] = stat->rtu_rsp_recv;
	  MoxaDevice.wData4x[MoxaDevice.status_info + j +14] = stat->rtu_rsp_timeout;
	  MoxaDevice.wData4x[MoxaDevice.status_info + j +15] = stat->rtu_rsp_crc;
	  MoxaDevice.wData4x[MoxaDevice.status_info + j +16] = stat->rtu_rsp_adu;
	  MoxaDevice.wData4x[MoxaDevice.status_info + j +17] = stat->rtu_rsp_pdu;
	
	  MoxaDevice.wData4x[MoxaDevice.status_info + j +18] = stat->tcp_req_send;
	  MoxaDevice.wData4x[MoxaDevice.status_info + j +19] = stat->tcp_rsp_recv;
	  MoxaDevice.wData4x[MoxaDevice.status_info + j +20] = stat->tcp_rsp_timeout;
	  MoxaDevice.wData4x[MoxaDevice.status_info + j +21] = stat->tcp_rsp_adu;
	  MoxaDevice.wData4x[MoxaDevice.status_info + j +22] = stat->tcp_rsp_pdu;
	
	  MoxaDevice.wData4x[MoxaDevice.status_info + j +23] = stat->frwd_rsp;
	  MoxaDevice.wData4x[MoxaDevice.status_info + j +24] = stat->tcp_rsp_send;
	  MoxaDevice.wData4x[MoxaDevice.status_info + j +25] = stat->rtu_rsp_send;

		j-=1;
	  
		for(k=0; k<STAT_FUNC_AMOUNT; k++) {
			MoxaDevice.wData4x[MoxaDevice.status_info + j + 3*k +32] = stat->func[k][STAT_RES_OK];
			MoxaDevice.wData4x[MoxaDevice.status_info + j + 3*k +33] = stat->func[k][STAT_RES_ERR];
			MoxaDevice.wData4x[MoxaDevice.status_info + j + 3*k +34] = stat->func[k][STAT_RES_EXP];
			}
	  
    }

	// общая статистика последовательных интерфейсов P1 - P8
  for(i=0; i<MAX_MOXA_PORTS; i++)
  	common_stat_to_gw4x(GWINF_STAT_RTU + 12*i, &IfaceRTU[i].stat);

  // общая статистика Ethernet-интерфейсов T01 - T32
  for(i=0; i<MAX_TCP_SERVERS; i++)
  	common_stat_to_gw4x(GWINF_STAT_TCP + 12*i, &IfaceTCP[i].stat);

  // общая статистика клиентских соединений 1 - 32
  for(i=0; i<MOXAGATE_CLIENTS_NUMBER; i++)
  	common_stat_to_gw4x(GWINF_STAT_CLIENTS + 12*i, &Client[i].stat);

  // общая статистика по шлюзу
 	common_stat_to_gw4x(GWINF_STAT_MOXAGATE, &Security.stat);

	return 0;
	}
///-----------------------------------------------------------------------------------------------------------------
int common_stat_to_gw4x(int index, GW_StaticData *src)
  {
	MoxaDevice.wData4x[MoxaDevice.status_info + index + 0] = src->accepted;
	MoxaDevice.wData4x[MoxaDevice.status_info + index + 1] = src->frwd_p;
	MoxaDevice.wData4x[MoxaDevice.status_info + index + 2] = src->frwd_a;
	MoxaDevice.wData4x[MoxaDevice.status_info + index + 3] = src->frwd_r;
	MoxaDevice.wData4x[MoxaDevice.status_info + index + 4] = src->errors;
	MoxaDevice.wData4x[MoxaDevice.status_info + index + 5] = src->sended;

	MoxaDevice.wData4x[MoxaDevice.status_info + index + 6] = src->proc_time;
	MoxaDevice.wData4x[MoxaDevice.status_info + index + 7] = src->proc_time_min;
	MoxaDevice.wData4x[MoxaDevice.status_info + index + 8] = src->proc_time_max;

	MoxaDevice.wData4x[MoxaDevice.status_info + index + 9] = src->poll_time;
	MoxaDevice.wData4x[MoxaDevice.status_info + index +10] = src->poll_time_min;
	MoxaDevice.wData4x[MoxaDevice.status_info + index +11] = src->poll_time_max;

	return 0;
  }
///-----------------------------------------------------------------------------------------------------------------
