/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///*********************** ћќƒ”Ћ№ ”—“–ќ…—“¬ј MODBUS MOXA UC-7410 ***************
///*** ћ≈’јЌ»«ћ ¬Ќ”“–≈ЌЌ≈… ѕјћя“»
///*** —“–” “”–џ ƒјЌЌџ’ ƒЋя –≈јЋ»«ј÷»» ‘”Ќ ÷»… HMI-¬«ј»ћќƒ≈…—“¬»я

///=== MOXAGATE_H IMPLEMENTATION

#include "interfaces.h"
#include "moxagate.h"
#include "modbus.h"
#include "messages.h"

///-----------------------------------------------------------------------------------------------------------------
int init_moxagate_h()
  {

	MoxaDevice.start_time=0;

  MoxaDevice.modbus_address=1;
  ///!!! должен быть ноль, т.к. это индекс. в насто€щее врем€ декрементируетс€ при инициализации:
  MoxaDevice.status_info=0;

	// MoxaDevice.queue
  // MoxaDevice.stat

  MoxaDevice.map2Xto4X=0;

  // MoxaDevice.moxa_mutex

	// начало блока внутренних регистров Moxa (смещение)
	//MoxaDevice.offset1xStatus=\
    MoxaDevice.offset2xStatus=\
    MoxaDevice.offset3xRegisters=\
    MoxaDevice.offset4xRegisters=0;

	// количество элементов в каждой из таблиц MODBUS
	//MoxaDevice.amount1xStatus=\
    MoxaDevice.amount2xStatus=\
    MoxaDevice.amount3xRegisters=\
    MoxaDevice.amount4xRegisters=0;

	// указатели на массивы пам€ти
	// MoxaDevice.wData1x
	// MoxaDevice.wData2x
	// MoxaDevice.wData3x
	// MoxaDevice.wData4x

  return 0;
  }

void *moxa_device(void *arg) //–“…≈Ќ - –≈“≈ƒЅёЅ ƒЅќќў» –ѕ Modbus TCP
  {
	u8			tcp_adu[MB_TCP_MAX_ADU_LENGTH];// TCP ADU
	u16			tcp_adu_len;
	u8			memory_adu[MB_SERIAL_MAX_ADU_LEN];
	u16			memory_adu_len;

	int		status;
	int		client_id, device_id;

  int port_id; //=MOXA_MB_DEVICE;
//  int client_id=((unsigned)arg)&0xff;

//	int		tcsd = IfaceRTU[port_id].clients[client_id].csd;
	GW_StaticData tmpstat;
	
//	input_cfg	*inputDATA;
//	inputDATA = &IfaceRTU[port_id];
	
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
	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|GATEWAY_MOXAGATE, 42, MOXA_MB_DEVICE, 0, 0, 0);

	while (1) {
		
		status=get_query_from_queue(&MoxaDevice.queue, &client_id, &device_id, tcp_adu, &tcp_adu_len);
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
					update_stat(&MoxaDevice.stat, &tmpstat);
					}
				continue;
				}

//		gettimeofday(&tv1, &tz);
		// считаем статистику, только если €вно отправл€ем ответ клиенту
		tmpstat.accepted++;

			if(Security.show_data_flow==1)
				show_traffic(TRAFFIC_TCP_SEND, GATEWAY_MOXAGATE, client_id, memory_adu, memory_adu_len-2);

			status = mb_tcp_send_adu(Client[client_id].csd,
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
			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_ERR|GATEWAY_MOXAGATE, 185, (unsigned) status, client_id, 0, 0);
		  	break;
		  default:;
		  };

//	if(exception_on==1) continue;///!!! обработка ошибки д.б., нужна ли здес эта проверка вообще?

	update_stat(&MoxaDevice.stat, &tmpstat);
//	gettimeofday(&tv2, &tz);
// врем€ не считаем, так как вс€ обработка происходит локально без передачи запроса далее
//	gate502.stat.request_time_average=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
	}

	EndRun: ;
	// THREAD STOPPED
 	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|GATEWAY_MOXAGATE, 43, 0, 0, 0, 0);
	pthread_exit (0);	
}
///-----------------------------------------------------------------------------------------------------------------
int process_moxamb_request(int client_id, u8 *adu, u16 adu_len, u8 *memory_adu, u16 *memory_adu_len)
  {
	static u16	*mem_start;
	static u8		*m_start;
	static u8		mask_src, mask_dst;

	static int	status, k, n, i, j;
	static int device_id, port_id;


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

			memory_adu[RTUADU_ADDRESS]=adu[TCPADU_ADDRESS]; //device ID
			memory_adu[RTUADU_FUNCTION]=adu[TCPADU_FUNCTION]; //ModBus Function Code
			memory_adu[RTUADU_BYTES]=(k-1)/8+1; 				//bytes total

//			for(n=0; n<(2*k); n++)
//        memory_adu[3+n]=oDATA[2*j+n];
			for(n=0; n<memory_adu[RTUADU_BYTES]; n++)
				for(i=0; i<8; i++) {

					mask_dst = 0x01 << i;									// битова€ маска в байте назначени€
					mask_src = 0x01 << (j + i + n*8) % 8; // битова€ маска в исходном байте
					status = (j + i + n*8) / 8;						// индекс исходного байта в массиве

				// в св€зи с отображением таблицы 2х на таблицу 4х требуетс€ иной пор€док формировани€ ответа.
				// надо реализовать перемещение по байтам в пам€ти, учитыва€ что данные хран€тс€ в блоках по
				// 16 бит, т.е. нужно отправл€ть сначала младшие 8 бит, затем старшие 8 бит. добавлена 1 строка:
					status=status%2?status-1:status+1;

	        memory_adu[3+n] = m_start[status] & mask_src ?\
						memory_adu[3+n] | mask_dst:\
						memory_adu[3+n] & (~mask_dst);

					}

			*memory_adu_len=memory_adu[RTUADU_BYTES]+3+2;
//			printf("status processed start=%d, len=%d\n", j, k);
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

			memory_adu[RTUADU_ADDRESS]=adu[TCPADU_ADDRESS]; //device ID
			memory_adu[RTUADU_FUNCTION]=adu[TCPADU_FUNCTION]; //ModBus Function Code
			memory_adu[RTUADU_BYTES]=2*k; 				//bytes total

//			for(n=0; n<(2*k); n++)
//        memory_adu[3+n]=oDATA[2*j+n];
			for(n=0; n<k; n++) {
        memory_adu[3+2*n]		=(mem_start[j+n]>>8)&0xff;
        memory_adu[3+2*n+1]	=	mem_start[j+n]&0xff;
				}
			*memory_adu_len=2*k+3+2;
//			printf("register processed start=%d, len=%d\n", j, k);
    	break;

		case MBF_WRITE_MULTIPLE_COILS:
		case MBF_WRITE_MULTIPLE_REGISTERS:
			n = ((adu[TCPADU_LEN_HI]<<8)|adu[TCPADU_LEN_LO])&0xffff;
		case MBF_WRITE_SINGLE_COIL:
		case MBF_WRITE_SINGLE_REGISTER:
			if(	(adu[TCPADU_FUNCTION]==MBF_WRITE_SINGLE_REGISTER) ||
					(adu[TCPADU_FUNCTION]==MBF_WRITE_SINGLE_COIL)) n=1;

			status=translateProxyDevice(
				((adu[TCPADU_START_HI]<<8)|adu[TCPADU_START_LO])&0xffff,
				n, &port_id, &device_id);

		  if(status) {
		 		// FRWD: PROXY TRANSLATION
		 		sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|GATEWAY_MOXAGATE, 130, client_id, ((adu[TCPADU_START_HI]<<8)|adu[TCPADU_START_LO])&0xffff, n, 0);
				return 1;
				}
														
//			printf("\n");
//			for(i=0; i<adu_len; i++) printf("(%2.2X)", adu[i]);
//			printf("\n");

			status=enqueue_query_ex(&IfaceRTU[port_id].queue, client_id, device_id, adu, adu_len);
//				printf("enqueue_query_ex %d P%d\n", status, port_id+1);
//				status=enqueue_query(port_id, i, device_id, tcp_adu, tcp_adu_len);
//				if(status!=0) continue;
			return 3;
//				if(status!=0) return 0;
//				return 0;
			break;

		default: //!!! добавить код счетчика статистики. уже не перва€ проверка по пути пакета
		 		// POLLING: FUNC NOT SUPPORTED
		 		sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|GATEWAY_MOXAGATE, 180, adu[TCPADU_FUNCTION], 0, 0, 0);
				return 2;
		}

	return 0;
	}
///----------------------------------------------------------------------------------------------------------------
