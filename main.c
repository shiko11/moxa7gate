/*
MOXA7GATE MODBUS GATEWAY SOFTWARE
SEM-ENGINEERING
                    BRYANSK 2009
*/

#include "global.h"

#include "modbus_tcp.h"

#define DEBUG_oDATA
#define DEBUG_iDATA

///-----------------------------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
/*** ИНИЦИАЛИЗАЦИЯ ПЕРЕМЕННЫХ ПРОГРАММЫ ***/
	memset(iDATA,0,sizeof(iDATA));

	memset(vslave,0,sizeof(vslave));
	memset(query_table,0,sizeof(query_table));
	memset(tcp_servers,0,sizeof(tcp_servers));

	int			i, j;
	for(i=0; i<MAX_MOXA_PORTS; i++) {

		for(j=0; j<MAX_TCP_CLIENTS_PER_PORT; j++) {
			iDATA[i].clients[j].rc=1;
			iDATA[i].clients[j].csd=-1;
			iDATA[i].clients[j].connection_status=MB_CONNECTION_CLOSED;
			iDATA[i].clients[j].mb_slave=MB_SLAVE_NOT_DEFINED;
			iDATA[i].clients[j].address_shift=MB_ADDRESS_NO_SHIFT;
		  }

	  iDATA[i].accepted_connections_number=0;
	  iDATA[i].current_connections_number=0;
	  iDATA[i].rejected_connections_number=0;

		iDATA[i].ssd=-1;
		iDATA[i].modbus_mode=MODBUS_PORT_OFF;
		strcpy(iDATA[i].bridge_status, "OFF");
		sprintf(iDATA[i].serial.p_name, "/dev/ttyM%d", i);

		iDATA[i].serial.p_num=i+1;
		strcpy(iDATA[i].serial.p_mode, "RS485_2w");
		strcpy(iDATA[i].serial.speed, "9600");
		strcpy(iDATA[i].serial.parity, "none");
		iDATA[i].serial.timeout=1000000;
		iDATA[i].tcp_port=1000*i+502;
		
    iDATA[i].current_client=0;

		p_errors[i]=0; // this value for buzzer function
		iDATA[i].start_time=0;

//		iDATA[i].queue_start=iDATA[i].queue_len=0; /// obsolete

		iDATA[i].queue.port_id=i;
		//queue.queue_adu[MAX_GATEWAY_QUEUE_LENGTH][MB_TCP_MAX_ADU_LENGTH];
		memset(iDATA[i].queue.queue_adu_len, 0, sizeof(iDATA[i].queue.queue_adu_len));
		//queue.queue_clients[MAX_GATEWAY_QUEUE_LENGTH];
		//queue.queue_slaves[MAX_GATEWAY_QUEUE_LENGTH];
		iDATA[i].queue.queue_start = iDATA[i].queue.queue_len = 0;
		pthread_mutex_init(&iDATA[i].queue.queue_mutex, NULL);
		iDATA[i].queue.operations[0].sem_flg=0;
		iDATA[i].queue.operations[0].sem_num=i;
		}

		for(j=0; j<MAX_MOXA_PORTS*MAX_TCP_CLIENTS_PER_PORT; j++) {
			gate502.clients[j].rc=1;
			gate502.clients[j].csd=-1;
			gate502.clients[j].connection_status=MB_CONNECTION_CLOSED;
			gate502.clients[j].mb_slave=MB_SLAVE_NOT_DEFINED;
			gate502.clients[j].address_shift=MB_ADDRESS_NO_SHIFT;
		  }

//	  gate502.accepted_connections_number=0;
//	  gate502.current_connections_number=0;
//	  gate502.rejected_connections_number=0;

		gate502.ssd=-1;
		gate502.tcp_port=502;
    gate502.current_client=0;
		gate502.start_time=0;

		gate502.show_data_flow=0;
		gate502.show_sys_messages=0;
		gate502.watchdog_timer=0;
		gate502.use_buzzer=0;
		gate502.back_light=0;
		gate502.tcp_port=502;
		gate502.modbus_address=0;
		gate502.status_info=0;

		gate502.object[0]=0;
		gate502.location[0]=0;
		gate502.version[0]=0;
		gate502.networkName[0]=0;
		gate502.IPAddress=0;

		gate502.app_log_current_entry=gate502.app_log_entries_total=0;
		app_log=NULL;

		gate502.queue.port_id=MOXA_MB_DEVICE;
		//queue.queue_adu[MAX_GATEWAY_QUEUE_LENGTH][MB_TCP_MAX_ADU_LENGTH];
		memset(gate502.queue.queue_adu_len, 0, sizeof(gate502.queue.queue_adu_len));
		//queue.queue_clients[MAX_GATEWAY_QUEUE_LENGTH];
		//queue.queue_slaves[MAX_GATEWAY_QUEUE_LENGTH];
		gate502.queue.queue_start = gate502.queue.queue_len = 0;
		pthread_mutex_init(&gate502.queue.queue_mutex, NULL);
		gate502.queue.operations[0].sem_flg=0;
		gate502.queue.operations[0].sem_num=MOXA_MB_DEVICE;

		gate502.halt=0;
	
		init_shm();

//		signal(SIGPIPE, sigpipe_handler);
//		signal(SIGIO, sigio_handler);

/*** РАЗБОР ПАРАМЕТРОВ КОМАНДНОЙ СТРОКИ ***/
	int res_cl = get_command_line (argc, argv,
								 						 iDATA,
														 &gate502,
														 vslave,
														 query_table,
														 tcp_servers
														 );
	switch(res_cl) {
		case CL_ERR_NONE_PARAM:
			printf("Command line parsing error: No Configuration Parameters\n");
			exit(1);
		case CL_INFO:
			exit(1);
		case CL_ERR_PORT_WORD:
			printf("Command line parsing error: Keyword PORT not found\n");
			exit(1);
		case CL_ERR_IN_STRUCT:
			printf("Command line parsing error: Invalid parameters amount for some PORTs\n");
			exit(1);
		case CL_ERR_IN_PORT_SETT:
			printf("Command line parsing error: Wrong Serial Port Configuration Parameter(s)\n");
			exit(1);
		case CL_ERR_GATEWAY_MODE:
			printf("Command line parsing error: Wrong GATEWAY mode keyword\n");
			exit(1);
		case CL_ERR_IN_MAP:
			printf("Command line parsing error: Wrong MAP data\n");
			exit(1);
		case CL_ERR_MIN_PARAM:
			printf("Command line parsing error: Too low Serial Port Configuration Parameters\n");
			exit(1);
		case CL_ERR_MUTEX_PARAM:
			printf("Command line parsing error: There are two mutualy ex(lusive) Parameters\n");
			exit(1);
		case CL_ERR_VSLAVES_CFG:
			printf("Command line parsing error: There are incorrect RTM_TABLE entries\n");
			exit(1);
		case CL_ERR_QT_CFG:
			printf("Command line parsing error: There are incorrect PROXY_TABLE entries\n");
			exit(1);
		case CL_ERR_TCPSRV_CFG:
			printf("Command line parsing error: There are incorrect TCP_SERVERS entries\n");
			exit(1);
		case CL_ERR_NOT_ALLOWED:
			printf("Command line parsing error: Parameter is not allowed\n");
			exit(1);
		case CL_OK:
			printf("Command line parsed successfully\n"); ///!!! add as system message: sysmsg();
		default:;
		}

//printf("\n\nCOMMAND LINE TESTING\n");
//exit (1);

/*		res_cl=read_conf();
		if(res_cl) {
			printf("Configuration read error %d: check settings\n", res_cl);
			exit(1);
			}
		printf("Configuration read OK\n");
*/

/*** ИНИЦИАЛИЗАЦИЯ МАССИВОВ ПАМЯТИ ПОД ТАБЛИЦЫ MODBUS ***/

/* в результате разбора параметров командной строки получаем количество параметров,
   которые нужно сохранять локально. соответственно производим выделение памяти под них */

	unsigned k;

	gate502.amount1xStatus=\
	gate502.amount2xStatus=\
	gate502.amount3xRegisters=\
	gate502.amount4xRegisters=0;

	gate502.offset1xStatus=\
	gate502.offset2xStatus=\
	gate502.offset3xRegisters=\
	gate502.offset4xRegisters=0xffff;

	for(i=0; i<MAX_QUERY_ENTRIES; i++) {																	

		if(	(query_table[i].offset==0)||
				(query_table[i].length==0)) continue;

		switch(query_table[i].mbf) {

			case MBF_READ_COILS:
				if(gate502.offset1xStatus > query_table[i].offset-1) gate502.offset1xStatus = query_table[i].offset-1;
				if(gate502.amount1xStatus < query_table[i].offset-1+query_table[i].length)
					gate502.amount1xStatus = query_table[i].offset-1+query_table[i].length;
				break;

			case MBF_READ_DECRETE_INPUTS:
				if(gate502.offset2xStatus > query_table[i].offset-1) gate502.offset2xStatus = query_table[i].offset-1;
				if(gate502.amount2xStatus < query_table[i].offset-1+query_table[i].length)
					gate502.amount2xStatus = query_table[i].offset-1+query_table[i].length;
				break;

			case MBF_READ_HOLDING_REGISTERS:
				if(gate502.offset4xRegisters > query_table[i].offset-1) gate502.offset4xRegisters = query_table[i].offset-1;
				if(gate502.amount4xRegisters < query_table[i].offset-1+query_table[i].length)
					gate502.amount4xRegisters = query_table[i].offset-1+query_table[i].length;
				break;

			case MBF_READ_INPUT_REGISTERS:
				if(gate502.offset3xRegisters > query_table[i].offset-1) gate502.offset3xRegisters = query_table[i].offset-1;
				if(gate502.amount3xRegisters < query_table[i].offset-1+query_table[i].length)
					gate502.amount3xRegisters = query_table[i].offset-1+query_table[i].length;
				break;

			default: continue;
			}
		}

	//printf("offset1x=%d, amount1x=%d\n", gate502.offset1xStatus, gate502.amount1xStatus);

	if(gate502.offset1xStatus==0xffff)		gate502.offset1xStatus=0;
	if(gate502.offset2xStatus==0xffff)		gate502.offset2xStatus=0;
	if(gate502.offset3xRegisters==0xffff)	gate502.offset3xRegisters=0;
	if(gate502.offset4xRegisters==0xffff)	gate502.offset4xRegisters=0;

	// блок статусной информации находится в области 4x, выделяем место для него
	if(gate502.status_info!=0) { /// если должен быть инициализирован блок статусной информации шлюза
		if(gate502.amount4xRegisters==0) { /// если 4х область адресного пространства не размечена
			gate502.offset4xRegisters=gate502.status_info-1;
			gate502.amount4xRegisters=gate502.status_info-1+GATE_STATUS_BLOCK_LENGTH;
			} else if(	/// если область 4х размечена уже и диапазоны регистров не перекрываются
					(gate502.status_info-1+GATE_STATUS_BLOCK_LENGTH <= gate502.offset4xRegisters) ||
					(gate502.status_info-1 >= gate502.amount4xRegisters)) {
		
				if(gate502.status_info-1 >= gate502.amount4xRegisters)
					gate502.amount4xRegisters = gate502.status_info-1+GATE_STATUS_BLOCK_LENGTH;
					else gate502.offset4xRegisters = gate502.status_info-1;
		
				} else	{ /// ошибка, если диапазоны регистров перекрываются
					printf("gate status info registers overlaps with others... so they are disabled\n");
					gate502.status_info=0;
					}
		}

	gate502.amount1xStatus-=		gate502.offset1xStatus;
	gate502.amount2xStatus-=		gate502.offset2xStatus;
	gate502.amount3xRegisters-=	gate502.offset3xRegisters;
	gate502.amount4xRegisters-=	gate502.offset4xRegisters;

  // выделение памяти под таблицу 1x
	if(gate502.amount1xStatus>0) {
		k=sizeof(u8)*((gate502.amount1xStatus-1)/8+1);
		printf("Memory allocation 1x (%db)...", k);
		gate502.wData1x=(u8 *) malloc(k);
		if(gate502.wData1x==NULL) {
			printf("error\n");
			exit(1);
			}
		memset(gate502.wData1x,0, k);
		printf("OK\n");
		}

  // выделение памяти под таблицу 2x
	if(gate502.amount2xStatus>0) {
		k=sizeof(u8)*((gate502.amount2xStatus-1)/8+1);
		printf("Memory allocation 2x (%db)...", k);
		gate502.wData2x=(u8 *) malloc(k);
		if(gate502.wData2x==NULL) {
			printf("error\n");
			exit(1);
			}
		memset(gate502.wData2x,0, k);
		printf("OK\n");
		}

  // выделение памяти под таблицу 3x
	if(gate502.amount3xRegisters>0) {
		printf("Memory allocation 3x (%db)...", sizeof(u16)*gate502.amount3xRegisters);
		gate502.wData3x=(u16 *) malloc(sizeof(u16)*gate502.amount3xRegisters);
		if(gate502.wData3x==NULL) {
			printf("error\n");
			exit(1);
			}
		memset(gate502.wData3x,0,sizeof(u16)*gate502.amount3xRegisters);
		printf("OK\n");
		}

  // выделение памяти под таблицу 4x
	if(gate502.amount4xRegisters>0) {
		printf("Memory allocation 4x (%db)...", sizeof(u16)*gate502.amount4xRegisters);
		gate502.wData4x=(u16 *) malloc(sizeof(u16)*gate502.amount4xRegisters);
		if(gate502.wData4x==NULL) {
			printf("error\n");
			exit(1);
			}
		memset(gate502.wData4x,0,sizeof(u16)*gate502.amount4xRegisters);
		printf("OK\n");
		}

/// для удобства в дальнейшей работе переводим номера регистров и битов в смещения, когда нумерация идет с нуля
	for(i=0; i<MAX_QUERY_ENTRIES; i++) { query_table[i].offset--; query_table[i].start--; }

/*** ИНИЦИАЛИЗАЦИЯ УСТРОЙСТВ КОНТРОЛЯ И МОНИТОРИНГА: LCM, KEYPAD, BUZZER (ДИСПЛЕЙ, КЛАВИАТУРА, ЗУММЕР) ***/
	mxkpd_handle=keypad_open();
	if (mxkpd_handle < 0 ) sysmsg(EVENT_SOURCE_SYSTEM, 0, "Keypad init ERROR", 1);
	  else sysmsg(EVENT_SOURCE_SYSTEM, 0, "Keypad init OK", 1);

	mxlcm_handle = mxlcm_open();
	if (mxlcm_handle < 0 ) sysmsg(EVENT_SOURCE_SYSTEM, 0, "LCM init ERROR", 1);
	  else sysmsg(EVENT_SOURCE_SYSTEM, 0, "LCM init OK", 1);
  mxlcm_control(mxlcm_handle, IOCTL_LCM_AUTO_SCROLL_OFF);

	mxbzr_handle = mxbuzzer_open();
	if (mxbzr_handle < 0 ) sysmsg(EVENT_SOURCE_SYSTEM, 0, "Buzzer init ERROR", 1);
	  else sysmsg(EVENT_SOURCE_SYSTEM, 0, "Buzzer init OK", 1);

	screen.current_screen=LCM_SCREEN_MAIN;

  screen.main_scr_mode=1;
  screen.menu_scr_mode=1;
  screen.secr_scr_mode=1;
  screen.back_light=1;
  screen.max_tcp_clients_per_com=8;
  screen.watch_dog_control=0;
  screen.buzzer_control=1;
  screen.secr_scr_changes_was_made=0;

	gettimeofday(&tv_mem, &tz);
//	printf("tv_mem %d\n", tv_mem.tv_sec);

/// запускаем поток для обработки ввода с клавиатуры и вывода на дисплей статусной информации
	int			rc;
	pthread_t		tstTH;
	rc = pthread_create(
		&tstTH,
		NULL,
		mx_keypad_lcm,
		NULL);
	//-------------------------------------------------------

//  printf("stopping program...\n"); exit(1);

/* ИНИЦИАЛИЗАЦИЯ СЕМАФОРОВ, НА КОТОРЫХ РАБОТАЮТ ОЧЕРЕДИ ПОРТОВ */

		key_t sem_key=ftok("/tmp/app", 'b');
		
		if((semaphore_id = semget(sem_key, MAX_MOXA_PORTS+1, IPC_CREAT|IPC_EXCL|0666)) == -1) {
			printf("ERROR. Semaphore Set already exists?\n");
			exit(1);
		 	}
		
		union semun sems;
		unsigned short values[1];

		values[0]=0;
		sems.array = values;
		/* Инициализируем все элементы одним значением */
		semctl(semaphore_id, 0, SETALL, sems);
		
		struct sembuf operations[1]; /// obsolete
		operations[0].sem_op=1; /// obsolete
		operations[0].sem_flg=0; /// obsolete

/*** ИНИЦИАЛИЗАЦИЯ TCP ПОРТА ШЛЮЗА, ПРИНИМАЮЩЕГО СОЕДИНЕНИЯ КО ВСЕМ ПОРТАМ, ЗА ИСКЛЮЧЕНИЕМ ПОРТОВ GATEWAY_SIMPLE ***/
	struct sockaddr_in	addr;
	int csd;

		sprintf(eventmsg, "Server socket 502 init ");
		gate502.ssd = socket(AF_INET, SOCK_STREAM, 0);
		if (gate502.ssd < 0) {
			perror("csdet");
	    strcat(eventmsg, "ERROR");
	    sysmsg(EVENT_SOURCE_SYSTEM, 0, eventmsg, 1);
			exit(1);
			}
    strcat(eventmsg, "OK");
    sysmsg(EVENT_SOURCE_SYSTEM, 0, eventmsg, 1);
		
		addr.sin_family = AF_INET;
		addr.sin_port = htons(gate502.tcp_port);
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		//РТЙЧСЪЩЧБЕН УПЛЕФ 
		sprintf(eventmsg, "Bind Socket 502 ");
		if (bind(gate502.ssd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
			perror("bind");
			close(gate502.ssd);
	    strcat(eventmsg, "ERROR");
	    sysmsg(EVENT_SOURCE_SYSTEM, 0, eventmsg, 1);
			exit(1);
			}
    strcat(eventmsg, "OK");
    sysmsg(EVENT_SOURCE_SYSTEM, 0, eventmsg, 1);

		listen(gate502.ssd, MAX_TCP_CLIENTS_PER_PORT*MAX_MOXA_PORTS);
		
		fcntl(gate502.ssd, F_SETFL, fcntl(gate502.ssd, F_GETFL, 0) | O_NONBLOCK);

		pthread_mutex_init(&gate502.moxa_mutex, NULL);

/* ЗАПУСК ПОТОКОВЫХ ФУНКЦИЙ, ИНИЦИАЛИЗАЦИЯ ПОСЛЕДОВАТЕЛЬНЫХ ПОРТОВ */

	int arg, P;

	// этот массив служит для организации порядка инициализации портов (сначала GATEWAY_SIMPLE, затем остальные)
	int ports[MAX_MOXA_PORTS]; 
	memset(ports, 0, sizeof(ports));

	for(i=0; i<MAX_MOXA_PORTS; i++) {
		
		P=0xff;

		if(P==0xff) for(j=0; j<MAX_MOXA_PORTS; j++)
		  if(((iDATA[j].modbus_mode==GATEWAY_SIMPLE)||(iDATA[j].modbus_mode==GATEWAY_PROXY))&&(ports[j]==0)) {
		  	ports[j]=1; P=j; break;
		    }

		if(P==0xff) for(j=0; j<MAX_MOXA_PORTS; j++)
		  if(((iDATA[j].modbus_mode==GATEWAY_ATM)||(iDATA[j].modbus_mode==GATEWAY_RTM))&&(ports[j]==0)) {
		  	ports[j]=1; P=j; break;
		    }

		if(P==0xff) for(j=0; j<MAX_MOXA_PORTS; j++)
		  if(((iDATA[j].modbus_mode==BRIDGE_PROXY)||(iDATA[j].modbus_mode==BRIDGE_SIMPLE))&&(ports[j]==0)) {
		  	ports[j]=1; P=j; break;
		    }

		if(P==0xff) continue;
		
		// opening serial ports
		if(iDATA[P].modbus_mode!=MODBUS_PORT_OFF) {
	    iDATA[P].serial.fd = open_comm(iDATA[P].serial.p_name, iDATA[P].serial.p_mode);
	    
			iDATA[P].serial.ch_interval_timeout = set_param_comms(iDATA[P].serial.fd, iDATA[P].serial.speed, iDATA[P].serial.parity);
		//	printf("speed %s\n", inputDATA->serial.speed);
		//	printf("parity %s\n", inputDATA->serial.parity);
	    
			// тест
	    //if(P==4) {iDATA[P].modbus_mode=MODBUS_PORT_ERROR; strcpy(iDATA[P].bridge_status, "ERR"); continue;}
	    if(0) {iDATA[P].modbus_mode=MODBUS_PORT_OFF; strcpy(iDATA[P].bridge_status, "ERR"); continue;} ///!!!

	    sprintf(eventmsg, "Serial port P%d init OK; mode %d", P+1, iDATA[P].modbus_mode);
			sysmsg(P, 0, eventmsg, 1);
			} else continue;

		switch(iDATA[P].modbus_mode) {

			case GATEWAY_ATM:
				strcpy(iDATA[P].bridge_status, "00A");
			case GATEWAY_RTM:

				if(iDATA[P].modbus_mode==GATEWAY_RTM)
					strcpy(iDATA[P].bridge_status, "00R");

			  arg=(P<<8)&0xff00;
			  iDATA[P].clients[0].rc = pthread_create(
				  &iDATA[P].clients[0].tid_srvr,
				  NULL,
				  srvr_tcp_child2,
				  (void *) arg);

				operations[0].sem_num=P;
  			semop(semaphore_id, operations, 1);
				break;

			case GATEWAY_PROXY:
				strcpy(iDATA[P].bridge_status, "00P");
			  arg=(P<<8)&0xff00;
				  iDATA[P].clients[DEFAULT_CLIENT].rc = pthread_create(
					  &iDATA[P].clients[DEFAULT_CLIENT].tid_srvr,
					  NULL,
					  gateway_proxy_thread,
					  (void *) arg);

				operations[0].sem_num=P;
  			semop(semaphore_id, operations, 1);
				break;

			case GATEWAY_SIMPLE:

				pthread_mutex_init(&iDATA[P].serial_mutex, NULL);

	    	sprintf(eventmsg, "Server socket init for P%d ", P+1);
				iDATA[P].ssd = socket(AF_INET, SOCK_STREAM, 0);
				if (iDATA[P].ssd < 0) {
					perror("csdet");
					iDATA[P].modbus_mode=MODBUS_PORT_ERROR;
			    strcpy(iDATA[P].bridge_status, "ERR");
			    strcat(eventmsg, "ERROR");
			    sysmsg(P, 0, eventmsg, 1);
			    break;
					}
		    strcat(eventmsg, "OK");
		    sysmsg(P, 0, eventmsg, 1);
				
				addr.sin_family = AF_INET;
				addr.sin_port = htons(iDATA[P].tcp_port);
				addr.sin_addr.s_addr = htonl(INADDR_ANY);
				//РТЙЧСЪЩЧБЕН УПЛЕФ 
				sprintf(eventmsg, "Bind Socket for P%d ", P+1);
				if (bind(iDATA[P].ssd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
					perror("bind");
					//status
					//exit(1);
					close(iDATA[P].ssd);
					iDATA[P].ssd=-1;
					iDATA[P].modbus_mode=MODBUS_PORT_ERROR;
			    strcpy(iDATA[P].bridge_status, "ERR");
			    strcat(eventmsg, "ERROR");
			    sysmsg(P, 0, eventmsg, 1);
			    break;
					}
		    strcat(eventmsg, "OK");
		    sysmsg(P, 0, eventmsg, 1);

				//УФБЧЙН Ч ПЮЕТЕДШ
				listen(iDATA[P].ssd, MAX_TCP_CLIENTS_PER_PORT);
				
				fcntl(iDATA[P].ssd, F_SETFL, fcntl(iDATA[P].ssd, F_GETFL, 0) | O_NONBLOCK);
			  strcpy(iDATA[P].bridge_status, "00G");
		
				break;

			/// инициализация BRIDGE-соединений по ЛВС выполняется ПОСЛЕ инициализации
			/// прослушивающих сокетов GATEWAY-портов
			case BRIDGE_SIMPLE:
				//инициализацию сетевых соединений порта в режиме BRIDGE проиводим в потоке порта

				strcpy(iDATA[P].bridge_status, "00B");

				arg=(P<<8)|(iDATA[P].current_client&0xff);
				//printf("arg:%d\n", arg);
				iDATA[P].clients[0].rc = pthread_create(
					&iDATA[P].clients[0].tid_srvr,
					NULL,
					srvr_tcp_bridge,
					(void *) arg);
			  break;

			case BRIDGE_PROXY: ///!!!
				strcpy(iDATA[P].bridge_status, "BPR");

				arg=(P<<8)|(iDATA[P].current_client&0xff);
				iDATA[P].clients[0].rc = pthread_create(
					&iDATA[P].clients[0].tid_srvr,
					NULL,
					bridge_proxy_thread,
					(void *) arg);
				break;

			default: iDATA[P].modbus_mode=MODBUS_PORT_OFF;
			}
		
		if(	(iDATA[P].modbus_mode==GATEWAY_ATM)||
				(iDATA[P].modbus_mode==GATEWAY_RTM)||
				(iDATA[P].modbus_mode==GATEWAY_PROXY)||
				(iDATA[P].modbus_mode==BRIDGE_SIMPLE)
				)
			if (iDATA[P].clients[DEFAULT_CLIENT].rc){
				sprintf(eventmsg, "pthread_create() ERROR %d", iDATA[P].clients[DEFAULT_CLIENT].rc);
		    sysmsg(P, 0, eventmsg, 1);
				iDATA[P].modbus_mode=MODBUS_PORT_ERROR;
	      strcpy(iDATA[P].bridge_status, "ERR");
			  }

/// ЗАПОМИНАЕМ ВРЕМЯ ЗАПУСКА ПОРТА
		time(&iDATA[P].start_time);
		}

/// ЗАПУСК ПОТОКА ДЛЯ ОБРАБОТКИ ЗАПРОСОВ НЕПОСРЕДСТВЕННО К MOXA

	pthread_t		moxaTH;
	rc = pthread_create(
		&moxaTH,
		NULL,
		moxa_mb_thread,
		NULL);
	operations[0].sem_num=MOXA_MB_DEVICE;
	semop(semaphore_id, operations, 1);

/// ЗАПОМИНАЕМ ВРЕМЯ ЗАПУСКА ШЛЮЗА
time(&gate502.start_time);
//----------   Ш Л Ю З   В   Р А Б О Т Е   ------------

gateway_common_processing();

//----------- ЗАВЕРШЕНИЕ РАБОТЫ ПРИЛОЖЕНИЯ -------------

	for(i=0; i<MAX_MOXA_PORTS; i++) {
	  if(iDATA[i].ssd>=0) {
			shutdown(iDATA[i].ssd, SHUT_RDWR);
	  	close(iDATA[i].ssd);
	  	}
		for(j=0; j<MAX_TCP_CLIENTS_PER_PORT; j++)
	  if(iDATA[i].clients[j].csd>=0) {
			shutdown(iDATA[i].clients[j].csd, SHUT_RDWR);
	  	close(iDATA[i].clients[j].csd);
	  	}
	  }

	shutdown(gate502.ssd, SHUT_RDWR);
	close(gate502.ssd);

  mxlcm_close(mxlcm_handle);
  keypad_close(mxkpd_handle);
  mxbuzzer_close(mxbzr_handle);

	close_shm();
	semctl(semaphore_id, MAX_MOXA_PORTS, IPC_RMID, NULL);

	free(gate502.wData1x);
	free(gate502.wData2x);
	free(gate502.wData3x);
	free(gate502.wData4x);

	sysmsg(EVENT_SOURCE_SYSTEM, 0, "Program stopped", 1);

	return (0);
}
///-----------------------------------------------------------------------------------------------------------------
int gateway_common_processing()
  {									
	struct sockaddr_in	addr;
  int i, j, csd, rc;

	FD_ZERO(&watchset);

	while (1) {
// в этом цикле происходит прием входящих соединений к портам в режиме GATEWAY_SIMPLE и создание клиентских потоков:
	  for(i=0; i<MAX_MOXA_PORTS; i++) {
	  	
		  if(iDATA[i].modbus_mode!=GATEWAY_SIMPLE) continue;
	  	
		  for(j=0; j<MAX_TCP_CLIENTS_PER_PORT; j++)
		    if(iDATA[i].clients[j].rc) break;
		  iDATA[i].current_client=j;

		 rc=sizeof(addr);
		 if(iDATA[i].ssd>=0) csd = accept(iDATA[i].ssd, (struct sockaddr *)&addr, &rc);
		   else continue;
			
			if((csd<0)&&(errno==EAGAIN)) {
				//printf("noop\n");
				continue;
			  }
			  
			if (csd < 0) {
				perror("accept");
				close(iDATA[i].ssd);
				iDATA[i].ssd=-1;
				iDATA[i].modbus_mode=MODBUS_PORT_ERROR;
	      strcpy(iDATA[i].bridge_status, "ERR");
			}

	   if(iDATA[i].current_client==MAX_TCP_CLIENTS_PER_PORT) {
			 iDATA[i].rejected_connections_number++;
			 
			 sprintf(eventmsg, "Conn rejected %d.%d.%d.%d",
			 															 addr.sin_addr.s_addr >> 24,
																		 (addr.sin_addr.s_addr >> 16) & 0xff,
																		 (addr.sin_addr.s_addr >> 8) & 0xff,
																		 addr.sin_addr.s_addr & 0xff);

	     sysmsg(i, 0, eventmsg, 0);
			 
			 close(csd);
	   	 continue;
	     }
	   iDATA[i].clients[iDATA[i].current_client].csd=csd;
	  	
			//printf("accepted %d connection for port %d\n", iDATA[i].current_client+1, i+1);
			
			int arg=(i<<8)|(iDATA[i].current_client&0xff);
			iDATA[i].clients[iDATA[i].current_client].rc = pthread_create(
				&iDATA[i].clients[iDATA[i].current_client].tid_srvr,
				NULL,
				srvr_tcp_child,
				(void *) arg);
			
			time(&iDATA[i].clients[iDATA[i].current_client].connection_time);
			iDATA[i].clients[iDATA[i].current_client].ip=addr.sin_addr.s_addr;
			iDATA[i].clients[iDATA[i].current_client].port=addr.sin_port;
			iDATA[i].clients[iDATA[i].current_client].connection_status=MB_CONNECTION_ESTABLISHED;
			clear_stat(&iDATA[i].clients[iDATA[i].current_client].stat);
			//printf("ip%X\n", addr.sin_addr.s_addr);
			
			if (iDATA[i].clients[iDATA[i].current_client].rc){
				close(iDATA[i].ssd);
				iDATA[i].ssd=-1;
				iDATA[i].modbus_mode=MODBUS_PORT_ERROR;
	      strcpy(iDATA[i].bridge_status, "ERR");
				sprintf(eventmsg, "pthread_create() ERROR %d", iDATA[i].clients[iDATA[i].current_client].rc);
		    sysmsg(i, 0, eventmsg, 0);
			  }
		  iDATA[i].accepted_connections_number++;
		  iDATA[i].current_connections_number++;
	    }

	  ///usleep(750000); ///!!!
		/// обработка входящих соединений на порт 502
		gateway_single_port_processing();
		/// механизм трансляции запросов
		query_translating();
	  
		// останов программы по внешней команде
		if(gate502.halt==1) break;
		}

	return 0;
  }
///-----------------------------------------------------------------------------------------------------------------
	//pthread_join(csd, NULL);
int gateway_single_port_processing()
	{
	struct sockaddr_in	addr;
  int i, j, k, csd, rc;

			///### блок обработки входящих TCP-соединений
			/// ищем свободный слот для нового соединения
		  for(j=0; j<MAX_MOXA_PORTS*MAX_TCP_CLIENTS_PER_PORT; j++)
		    if(gate502.clients[j].csd==-1) break;
		  gate502.current_client=j;

		 rc=sizeof(addr);
		 csd=-1;
		 if(gate502.ssd>=0) csd=accept(gate502.ssd, (struct sockaddr *)&addr, &rc);
			
			if(!((csd<0)&&(errno==EAGAIN)))			  
			if (csd < 0) {
				perror("accept");
  	    sysmsg(EVENT_SOURCE_SYSTEM, 0, "accept error", 1);
			} else {

	   if(gate502.current_client==MAX_MOXA_PORTS*MAX_TCP_CLIENTS_PER_PORT) {
			 gate502.rejected_connections_number++;
			 
			 sprintf(eventmsg, "Conn rejected %d.%d.%d.%d",
			 															 addr.sin_addr.s_addr >> 24,
																		 (addr.sin_addr.s_addr >> 16) & 0xff,
																		 (addr.sin_addr.s_addr >> 8) & 0xff,
																		 addr.sin_addr.s_addr & 0xff);

	     sysmsg(i, 0, eventmsg, 0);
			 
			 close(csd);
	     } else {
	   		 gate502.clients[gate502.current_client].csd=csd;

				 FD_SET(gate502.clients[gate502.current_client].csd, &watchset);
	  	
			 	 sprintf(eventmsg, "Slot %d connection for TCP502: %d.%d.%d.%d\n", gate502.current_client+1,
			 															 addr.sin_addr.s_addr >> 24,
																		 (addr.sin_addr.s_addr >> 16) & 0xff,
																		 (addr.sin_addr.s_addr >> 8) & 0xff,
																		 addr.sin_addr.s_addr & 0xff);
    		 sysmsg(EVENT_SOURCE_GATE502|(i<<8), 0, eventmsg, 1);
			
				 time(&gate502.clients[gate502.current_client].connection_time);
				 time(&gate502.clients[gate502.current_client].last_activity_time);
			   gate502.clients[gate502.current_client].ip=addr.sin_addr.s_addr;
			   gate502.clients[gate502.current_client].port=addr.sin_port;
			   gate502.clients[gate502.current_client].connection_status=MB_CONNECTION_ESTABLISHED;
			   clear_stat(&gate502.clients[gate502.current_client].stat);
			//printf("ip%X\n", addr.sin_addr.s_addr);
			
		  	gate502.accepted_connections_number++;
		  	gate502.current_connections_number++;
				}
		  }


	  //usleep(500000); // proverit' zaderzhku na obrabotku vseh klientov v rezhime MASTER

	return 0;
	}
///-----------------------------------------------------------------------------------------------------------------
int query_translating()
  {									
  int i, j, k;
	struct sockaddr_in	addr;

	struct timeval stv;
	stv.tv_sec=0; stv.tv_usec=0;

		//printf("input data processing...\n");

		///### блок обработки входящих ModbusTCP запросов
		u8			tcp_adu[MB_TCP_MAX_ADU_LENGTH];// TCP ADU
		u16			tcp_adu_len;
		GW_StaticData tmpstat;
		int port_id, device_id;
		int status;

		unsigned temp;
		int maxfd;
		maxfd=0;

    int diff;
	  time_t moment;
    time(&moment);

	  for(i=0; i<MAX_MOXA_PORTS*MAX_TCP_CLIENTS_PER_PORT; i++) { // принимаем данные от клиентов

			if(gate502.clients[i].csd!=-1)
			  if(FD_ISSET(gate502.clients[i].csd, &watchset)) {
					diff=difftime(moment, gate502.clients[i].last_activity_time);
	    		if(diff>=MAX_CLIENT_ACTIVITY_TIMEOUT) {
					  FD_CLR(gate502.clients[i].csd, &watchset);
					  gate502.clients[i].connection_status=MB_CONNECTION_CLOSED;
					  shutdown(gate502.clients[i].csd, SHUT_RDWR);
					  close(gate502.clients[i].csd);
					  gate502.clients[i].csd=-1;
						gate502.current_connections_number--;
			    sprintf(eventmsg, "Slot %d disc(tmt): %d.%d.%d.%d\n", i+1,
			 															 addr.sin_addr.s_addr >> 24,
																		 (addr.sin_addr.s_addr >> 16) & 0xff,
																		 (addr.sin_addr.s_addr >> 8) & 0xff,
																		 addr.sin_addr.s_addr & 0xff);
		  			sysmsg(EVENT_SOURCE_GATE502|(i<<8), 0, eventmsg, 0);
						};
      }

	    if(maxfd<gate502.clients[i].csd) maxfd=gate502.clients[i].csd;
	    }

			clear_stat(&tmpstat);
			
			inset=watchset;

			if((temp=select(maxfd+1, &inset, NULL, NULL, &stv))<0) {
				perror("select");
				return 0;
				}
		if(temp==0) return 0;
																												 
	  for(i=0; i<MAX_MOXA_PORTS*MAX_TCP_CLIENTS_PER_PORT; i++) 
			if (gate502.clients[i].connection_status==MB_CONNECTION_ESTABLISHED)
			if (FD_ISSET(gate502.clients[i].csd, &inset)) {
	  	
			if(_show_data_flow)
				printf("TCP%4.4d  IN: ", gate502.clients[i].port);
			status = mb_tcp_receive_adu(gate502.clients[i].csd, &tmpstat, tcp_adu, &tcp_adu_len);
	
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
					sprintf(eventmsg, "tcp receive error: %d", status);
	  			sysmsg(EVENT_SOURCE_GATE502|(i<<8), 0, eventmsg, 0);
					//update_stat(&inputDATA->clients[client_id].stat, &tmpstat);
					//update_stat(&iDATA[port_id].stat, &tmpstat);
				  if(status==TCP_COM_ERR_NULL) {
					  FD_CLR(gate502.clients[i].csd, &watchset);
					  gate502.clients[i].connection_status=MB_CONNECTION_CLOSED;
					  shutdown(gate502.clients[i].csd, SHUT_RDWR);
					  close(gate502.clients[i].csd);
					  gate502.clients[i].csd=-1;
						gate502.current_connections_number--;
			    sprintf(eventmsg, "Slot %d disc(lnkoff): %d.%d.%d.%d\n", i+1,
			 															 addr.sin_addr.s_addr >> 24,
																		 (addr.sin_addr.s_addr >> 16) & 0xff,
																		 (addr.sin_addr.s_addr >> 8) & 0xff,
																		 addr.sin_addr.s_addr & 0xff);
		  			sysmsg(EVENT_SOURCE_GATE502|(i<<8), 0, eventmsg, 0);
						}
					return 0;
			  	break;
			  default:; ///!!! continue;
			  };

			time(&gate502.clients[i].last_activity_time);


/* разбор входящего запроса происходит в несколько этапов:
	- проверяем modbus адрес, указанный в запросе;
	- проверяем диапазон регистров, указанный в запросе;
	- если и адрес и диапазон принадлежат устройству Moxa, ставим запрос в очередь внутренних запросов;
	- если только адрес принадлежит устройству Moxa, ставим запрос в очередь одного из портов режима RTM;
	- если адрес не принадлежит устройству Moxa, ставим запрос в очередь одного из портов режима ATM.
 */

			k=tcp_adu[TCPADU_ADDRESS]==gate502.modbus_address?GATEWAY_SIMPLE:GATEWAY_ATM;

			switch(tcp_adu[TCPADU_FUNCTION]) { 	// определяем длину диапазона регистров, требуемых в запросе,
																					// а также фильтруем входящие запросы по функциям ModBus
				case MBF_READ_COILS:
				case MBF_READ_DECRETE_INPUTS:
				case MBF_READ_HOLDING_REGISTERS:
				case MBF_READ_INPUT_REGISTERS:
		
				case MBF_WRITE_MULTIPLE_COILS:
				case MBF_WRITE_MULTIPLE_REGISTERS:
		
					j=(tcp_adu[TCPADU_LEN_HI]<<8)|tcp_adu[TCPADU_LEN_LO];
					break;

				case MBF_WRITE_SINGLE_COIL:
				case MBF_WRITE_SINGLE_REGISTER:
					j=1;
					break;
		
				default: ///!!! функция не поддерживается шлюзом
					printf("modbus function not supported\n");
					continue;
				}
	
			if(k!=GATEWAY_ATM) {
				switch(
							checkDiapason(	tcp_adu[TCPADU_FUNCTION],
															(tcp_adu[TCPADU_START_HI]<<8)|tcp_adu[TCPADU_START_LO],
															j)) {
	
					case 	MOXA_DIAPASON_INSIDE:
						k=GATEWAY_PROXY;
						/// ставим запрос в очередь MOXA MODBUS DEVICE
						if((status=enqueue_query_ex(&gate502.queue, i, device_id, tcp_adu, tcp_adu_len))!=0) continue;
//						printf("GATE502 GATEWAY_PROXY\n");
						break;
					
					case MOXA_DIAPASON_OUTSIDE:
						k=GATEWAY_RTM;
	
						status=translateRegisters(
							(tcp_adu[TCPADU_START_HI]<<8)|tcp_adu[TCPADU_START_LO],
							j, &port_id, &device_id);

					  if(status) {
							sprintf(eventmsg, "Reg translation error %d P%d CL%d", status, port_id+1, i);
			  			sysmsg(EVENT_SOURCE_GATE502|(i<<8), 0, eventmsg, 0);
							continue;
							}

						/// ставим запрос в очередь последовательного порта
//						if((status=enqueue_query(port_id, i, device_id, tcp_adu, tcp_adu_len))!=0) continue;
						if((status=enqueue_query_ex(&iDATA[port_id].queue, i, device_id, tcp_adu, tcp_adu_len))!=0) continue;
//						printf("GATE502 GATEWAY_RTM\n");
						break;
					
					case MOXA_DIAPASON_OVERLAPPED:
						printf("GATE502 MOXA_DIAPASON_OVERLAPPED\n");
						break;
	
					default:;
					}
				} else { /// обработка запроса в режиме ATM

					status=translateAddress(tcp_adu[TCPADU_ADDRESS], &port_id, &device_id);

				  if(status) {
						sprintf(eventmsg, "invalid Unit ID P%d CL%d", port_id+1, i);
		  			sysmsg(EVENT_SOURCE_GATE502|(i<<8), 0, eventmsg, 0);
						continue;
						}

					tcp_adu[TCPADU_ADDRESS]=device_id;

					/// ставим запрос в очередь последовательного порта
//printf("enqueue_query port_id=%d, client_id=%d, device_id=%d\n", port_id, i, device_id);
//					if((status=enqueue_query(port_id, i, device_id, tcp_adu, tcp_adu_len))!=0) continue;
						if((status=enqueue_query_ex(&iDATA[port_id].queue, i, device_id, tcp_adu, tcp_adu_len))!=0) continue;
					}

	    }

  return 0;
  }
///-----------------------------------------------------------------------------------------------------------------
