/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///********* ПРИЛОЖЕНИЕ MOXA7GATE, ГЛАВНЫЙ МОДУЛЬ ПРОГРАММЫ ********************

///=== MAIN_H IMPLEMENTATION

#include "main.h"

///=== MAIN_H private variables

//struct timeval tv_mem;
//struct timezone tz;
//unsigned int p_errors[MAX_MOXA_PORTS];
struct sockaddr_in	addr;

///=== MAIN_H private functions

///-----------------------------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{

/*** ИНИЦИАЛИЗАЦИЯ ПЕРЕМЕННЫХ ПРОГРАММЫ ***/

	int k;

  init_moxagate_h();
  init_interfaces_h();
  init_frwd_queue_h();
  init_messages_h(); // обязательно перед init_hmi_web_h()
  init_hmi_web_h();
  init_statistics_h();
  init_clients();

/*** РАЗБОР ПАРАМЕТРОВ КОМАНДНОЙ СТРОКИ ***/
	k = get_command_line(argc, argv);
	switch(k & 0xff) {

		case COMMAND_LINE_OK:			// CMD LINE: OK
			sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|GATEWAY_SYSTEM, k & 0xff, 0, 0, 0, 0);
			break;

		case COMMAND_LINE_ARGC:
			sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, k & 0xff, 0, 0, 0, 0);
			exit(1);

		case COMMAND_LINE_ERROR:
			sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, k & 0xff, k >> 8, 0, 0, 0);
			exit(1);

		case COMMAND_LINE_INFO: 
			sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|GATEWAY_SYSTEM, k & 0xff, 0, 0, 0, 0);
      exit(1);

		default:;		  // CMD LINE: UNCERTAIN RESULT
			sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, COMMAND_LINE_UNDEFINED, 0, 0, 0, 0);
      exit(1);
		}

/*** ПРОВЕРКА КОНФИГУРАЦИИ ШЛЮЗА В ЦЕЛОМ ***/

  k=check_GatewayTCPPorts();
  if(k!=0) {
    sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, CONFIG_TCPPORT_CONFLICT, k >> 8, k & 0xff, 0, 0);
    exit(1);
    }

  k=check_GatewayAddressMap();
  if((k&0xff)!=0) 
  if( ((k&0xff)==GATEWAY_MOXAGATE) &&
      ((k >> 8)==0) &&
      (AddressMap[MoxaDevice.modbus_address].iface==GATEWAY_NONE)
    ) {
    // присваиваем адрес шлюзу по умолчанию:
    AddressMap[MoxaDevice.modbus_address].iface=GATEWAY_MOXAGATE;
    } else {
      sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, CONFIG_ADDRMAP, k & 0xff, k >> 8, 0, 0);
      exit(1);
      }

  k=check_GatewayIfaces_ex();
  if(k!=0) {
    sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, CONFIG_FORWARDING, k & 0xff, 0, 0, 0);
    exit(1);
    }

  k=check_GatewayConf();
  if(k!=0) {
    sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, CONFIG_GATEWAY, 0, 0, 0, 0);
    exit(1);
    }

  k=check_IntegrityAddressMap();
  if(k!=0) {
    sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, CONFIG_ADDRMAP_INTEGRITY, k, 0, 0, 0);
    exit(1);
    }

  k=check_IntegrityVSlaves();
  if(k!=0) {
    sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, CONFIG_VSLAVES_INTEGRITY, k, 0, 0, 0);
    exit(1);
    }

  k=check_IntegrityPQueries();
  if(k!=0) {
    sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, CONFIG_PQUERIES_INTEGRITY, k, 0, 0, 0);
    exit(1);
    }

/*** ИНИЦИАЛИЗАЦИЯ МАССИВОВ ПАМЯТИ ПОД ТАБЛИЦЫ MODBUS ***/

  int i, j;

/* в результате разбора параметров командной строки получаем количество параметров,
   которые нужно сохранять локально. соответственно производим выделение памяти под них */

	MoxaDevice.amount1xStatus=\
	MoxaDevice.amount2xStatus=\
	MoxaDevice.amount3xRegisters=\
	MoxaDevice.amount4xRegisters=0;

	MoxaDevice.offset1xStatus=\
	MoxaDevice.offset2xStatus=\
	MoxaDevice.offset3xRegisters=\
	MoxaDevice.offset4xRegisters=0xffff;

	for(i=0; i<MAX_QUERY_ENTRIES; i++) {																	

		if(	(query_table[i].offset==0)||
				(query_table[i].length==0)) continue;

		switch(query_table[i].mbf) {

			case MBF_READ_COILS:
				if(MoxaDevice.offset1xStatus > query_table[i].offset-1) MoxaDevice.offset1xStatus = query_table[i].offset-1;
				if(MoxaDevice.amount1xStatus < query_table[i].offset-1+query_table[i].length)
					MoxaDevice.amount1xStatus = query_table[i].offset-1+query_table[i].length;
				break;

			case MBF_READ_DECRETE_INPUTS:
				if(MoxaDevice.offset2xStatus > query_table[i].offset-1) MoxaDevice.offset2xStatus = query_table[i].offset-1;
				if(MoxaDevice.amount2xStatus < query_table[i].offset-1+query_table[i].length)
					MoxaDevice.amount2xStatus = query_table[i].offset-1+query_table[i].length;
				break;

			case MBF_READ_HOLDING_REGISTERS:
				if(MoxaDevice.offset4xRegisters > query_table[i].offset-1) MoxaDevice.offset4xRegisters = query_table[i].offset-1;
				if(MoxaDevice.amount4xRegisters < query_table[i].offset-1+query_table[i].length)
					MoxaDevice.amount4xRegisters = query_table[i].offset-1+query_table[i].length;
				break;

			case MBF_READ_INPUT_REGISTERS:
				if(MoxaDevice.offset3xRegisters > query_table[i].offset-1) MoxaDevice.offset3xRegisters = query_table[i].offset-1;
				if(MoxaDevice.amount3xRegisters < query_table[i].offset-1+query_table[i].length)
					MoxaDevice.amount3xRegisters = query_table[i].offset-1+query_table[i].length;
				break;

			default: continue;
			}
		}

	//printf("offset1x=%d, amount1x=%d\n", gate502.offset1xStatus, gate502.amount1xStatus);

	if(MoxaDevice.offset1xStatus==0xffff)		MoxaDevice.offset1xStatus=0;
	if(MoxaDevice.offset2xStatus==0xffff)		MoxaDevice.offset2xStatus=0;
	if(MoxaDevice.offset3xRegisters==0xffff)	MoxaDevice.offset3xRegisters=0;
	if(MoxaDevice.offset4xRegisters==0xffff)	MoxaDevice.offset4xRegisters=0;

  ///!!! алгоритм ниже исправить: следующий оператор не должен выполняться здесь, он выполняется ранее
  MoxaDevice.status_info++;

	// блок статусной информации находится в области 4x, выделяем место для него
	if(MoxaDevice.status_info!=0) { /// если должен быть инициализирован блок статусной информации шлюза
		if(MoxaDevice.amount4xRegisters==0) { /// если 4х область адресного пространства не размечена
			MoxaDevice.offset4xRegisters=MoxaDevice.status_info-1;
			MoxaDevice.amount4xRegisters=MoxaDevice.status_info-1+GATE_STATUS_BLOCK_LENGTH;
			} else if(	/// если область 4х размечена уже и диапазоны регистров не перекрываются
					(MoxaDevice.status_info-1+GATE_STATUS_BLOCK_LENGTH <= MoxaDevice.offset4xRegisters) ||
					(MoxaDevice.status_info-1 >= MoxaDevice.amount4xRegisters)) {
		
				if(MoxaDevice.status_info-1 >= MoxaDevice.amount4xRegisters)
					MoxaDevice.amount4xRegisters = MoxaDevice.status_info-1+GATE_STATUS_BLOCK_LENGTH;
					else MoxaDevice.offset4xRegisters = MoxaDevice.status_info-1;
		
				} else	{ /// ошибка, если диапазоны регистров перекрываются
					// STATUS INFO OVERLAPS
					sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, 39, 0, 0, 0, 0);
					MoxaDevice.status_info=0;
					}
		}

	MoxaDevice.amount1xStatus-=		MoxaDevice.offset1xStatus;
	MoxaDevice.amount2xStatus-=		MoxaDevice.offset2xStatus;
	MoxaDevice.amount3xRegisters-=	MoxaDevice.offset3xRegisters;
	MoxaDevice.amount4xRegisters-=	MoxaDevice.offset4xRegisters;

  // выделение памяти под таблицу 1x
	if(MoxaDevice.amount1xStatus>0) {
		k=sizeof(u8)*((MoxaDevice.amount1xStatus-1)/8+1);
		MoxaDevice.wData1x=(u8 *) malloc(k);
		if(MoxaDevice.wData1x==NULL) {
			sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, 28, 1, k, 0, 0);
			exit(1);
			}
		memset(MoxaDevice.wData1x,0, k);
		sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|GATEWAY_SYSTEM, 28, 1, k, 0, 0);
		}

	// отображаем таблицу дискретных входов на таблицу holding-регистров
  /*/ выделение памяти под таблицу 2x
	if(gate502.amount2xStatus>0) {
		k=sizeof(u8)*((gate502.amount2xStatus-1)/8+1);
		gate502.wData2x=(u8 *) malloc(k);
		if(gate502.wData2x==NULL) {
			sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|EVENT_SRC_SYSTEM, 28, 2, k, 0, 0);
			exit(1);
			}
		memset(gate502.wData2x,0, k);
		sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|EVENT_SRC_SYSTEM, 28, 2, k, 0, 0);
		}*/

  // выделение памяти под таблицу 3x
	if(MoxaDevice.amount3xRegisters>0) {
		k=sizeof(u16)*MoxaDevice.amount3xRegisters;
		MoxaDevice.wData3x=(u16 *) malloc(k);
		if(MoxaDevice.wData3x==NULL) {
			sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, 28, 3, k, 0, 0);
			exit(1);
			}
		memset(MoxaDevice.wData3x, 0, k);
		sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|GATEWAY_SYSTEM, 28, 3, k, 0, 0);
		}

  // выделение памяти под таблицу 4x
	if(MoxaDevice.amount4xRegisters>0) {
		k=sizeof(u16)*MoxaDevice.amount4xRegisters;
		MoxaDevice.wData4x=(u16 *) malloc(k);
		if(MoxaDevice.wData4x==NULL) {
			sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, 28, 4, k, 0, 0);
			exit(1);
			}
		memset(MoxaDevice.wData4x, 0, k);
		sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|GATEWAY_SYSTEM, 28, 4, k, 0, 0);

		// отображаем таблицу дискретных входов на таблицу holding-регистров
		MoxaDevice.offset2xStatus=MoxaDevice.offset4xRegisters*sizeof(u16)*8;
		MoxaDevice.amount2xStatus=MoxaDevice.amount4xRegisters*sizeof(u16)*8;
		MoxaDevice.wData2x=(u8 *) MoxaDevice.wData4x;
		sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|GATEWAY_SYSTEM, 28, 2, k, 0, 0);
		}

/// для удобства в дальнейшей работе переводим номера регистров и битов в смещения, когда нумерация идет с нуля
	for(i=0; i<MAX_QUERY_ENTRIES; i++) { query_table[i].offset--; query_table[i].start--; }
  ///!!! алгоритм выше исправить: следующий оператор не должен выполняться здесь, он выполняется ранее
  MoxaDevice.status_info--;

  // мьютекс используется для синхронизации потоков при работе с памятью
	pthread_mutex_init(&MoxaDevice.moxa_mutex, NULL);

/*** ИНИЦИАЛИЗАЦИЯ УСТРОЙСТВ КОНТРОЛЯ И МОНИТОРИНГА: LCM, KEYPAD, BUZZER (ДИСПЛЕЙ, КЛАВИАТУРА, ЗУММЕР) ***/
	mxkpd_handle=keypad_open();
	sysmsg_ex(EVENT_CAT_MONITOR|(mxkpd_handle<0?EVENT_TYPE_ERR:EVENT_TYPE_INF)|GATEWAY_SYSTEM,
						26, 0, 0, 0, 0);

	mxlcm_handle = mxlcm_open();
	sysmsg_ex(EVENT_CAT_MONITOR|(mxlcm_handle<0?EVENT_TYPE_ERR:EVENT_TYPE_INF)|GATEWAY_SYSTEM,
						27, 0, 0, 0, 0);
  mxlcm_control(mxlcm_handle, IOCTL_LCM_AUTO_SCROLL_OFF);

	mxbzr_handle = mxbuzzer_open();
	sysmsg_ex(EVENT_CAT_MONITOR|(mxbzr_handle<0?EVENT_TYPE_ERR:EVENT_TYPE_INF)|GATEWAY_SYSTEM,
						25, 0, 0, 0, 0);

	screen.current_screen=LCM_SCREEN_MAIN;

  screen.main_scr_mode=1;
  screen.menu_scr_mode=1;
  screen.secr_scr_mode=1;
  screen.back_light=1;
  screen.max_tcp_clients_per_com=8;
  screen.watch_dog_control=0;
  screen.buzzer_control=1;
  screen.secr_scr_changes_was_made=0;

//gettimeofday(&tv_mem, &tz);
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
	if(init_queue() == 1) exit(1);

/*** ИНИЦИАЛИЗАЦИЯ TCP ПОРТА ШЛЮЗА, ПРИНИМАЮЩЕГО СОЕДИНЕНИЯ КО ВСЕМ ПОРТАМ, ЗА ИСКЛЮЧЕНИЕМ ПОРТОВ GATEWAY_SIMPLE ***/
 if(init_main_socket() !=0 ) exit(1);

/* ЗАПУСК ПОТОКОВЫХ ФУНКЦИЙ, ИНИЦИАЛИЗАЦИЯ ПОСЛЕДОВАТЕЛЬНЫХ ПОРТОВ */

	int arg, P;

	struct sembuf operations[1]; /// obsolete
	operations[0].sem_op=1; /// obsolete
	operations[0].sem_flg=0; /// obsolete

	// этот массив служит для организации порядка инициализации портов (сначала GATEWAY_SIMPLE, затем остальные)
	int ports[MAX_MOXA_PORTS]; 
	memset(ports, 0, sizeof(ports));

	for(i=0; i<MAX_MOXA_PORTS; i++) {
		
		P=0xff;

		if(P==0xff) for(j=0; j<MAX_MOXA_PORTS; j++)
		  if(((IfaceRTU[j].modbus_mode==GATEWAY_SIMPLE)||(IfaceRTU[j].modbus_mode==GATEWAY_PROXY))&&(ports[j]==0)) {
		  	ports[j]=1; P=j; break;
		    }

		if(P==0xff) for(j=0; j<MAX_MOXA_PORTS; j++)
		  if(((IfaceRTU[j].modbus_mode==GATEWAY_ATM)||(IfaceRTU[j].modbus_mode==GATEWAY_RTM))&&(ports[j]==0)) {
		  	ports[j]=1; P=j; break;
		    }

		if(P==0xff) for(j=0; j<MAX_MOXA_PORTS; j++)
		  if((IfaceRTU[j].modbus_mode==BRIDGE_PROXY)&&(ports[j]==0)) {
		  	ports[j]=1; P=j; break;
		    }

		if(P==0xff) continue;
		
		// opening serial ports
		if(IfaceRTU[P].modbus_mode!=MODBUS_PORT_OFF) {
	    IfaceRTU[P].serial.fd = open_comm(IfaceRTU[P].serial.p_name, IfaceRTU[P].serial.p_mode);
	    
			IfaceRTU[P].serial.ch_interval_timeout = set_param_comms(IfaceRTU[P].serial.fd, IfaceRTU[P].serial.speed, IfaceRTU[P].serial.parity);
		//	printf("speed %s\n", inputDATA->serial.speed);
		//	printf("parity %s\n", inputDATA->serial.parity);
	    
			// тест
	    //if(P==4) {IfaceRTU[P].modbus_mode=MODBUS_PORT_ERROR; strcpy(IfaceRTU[P].bridge_status, "ERR"); continue;}
	    if(0) {IfaceRTU[P].modbus_mode=MODBUS_PORT_OFF; strcpy(IfaceRTU[P].bridge_status, "ERR"); continue;} ///!!!

			// SERIAL PORT INITIALIZED
			sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|P, 40, IfaceRTU[P].modbus_mode, 0, 0, 0);
			} else continue;

		switch(IfaceRTU[P].modbus_mode) {

			case GATEWAY_ATM:
				strcpy(IfaceRTU[P].bridge_status, "00A");
			case GATEWAY_RTM:

				if(IfaceRTU[P].modbus_mode==GATEWAY_RTM)
					strcpy(IfaceRTU[P].bridge_status, "00R");

			  arg=(P<<8)&0xff00;
			  IfaceRTU[P].rc = pthread_create(
				  &IfaceRTU[P].tid_srvr,
				  NULL,
				  srvr_tcp_child2,
				  (void *) arg);

				operations[0].sem_num=P;
  			semop(semaphore_id, operations, 1);
				break;

			case GATEWAY_PROXY:
				strcpy(IfaceRTU[P].bridge_status, "00P");
			  arg=(P<<8)&0xff00;
				  IfaceRTU[P].rc = pthread_create(
					  &IfaceRTU[P].tid_srvr,
					  NULL,
					  iface_rtu_master,
					  (void *) arg);

				operations[0].sem_num=P;
  			semop(semaphore_id, operations, 1);
				break;

			case GATEWAY_SIMPLE:

				pthread_mutex_init(&IfaceRTU[P].serial_mutex, NULL);

	    	// SOCKET INITIALIZED
				IfaceRTU[P].ssd = socket(AF_INET, SOCK_STREAM, 0);
				if (IfaceRTU[P].ssd < 0) {
					perror("csdet");
					IfaceRTU[P].modbus_mode=MODBUS_PORT_ERROR;
			    ///!!! перенести формирование этой строки в mxshm.c:99[refresh_shm()]
			    strcpy(IfaceRTU[P].bridge_status, "ERR"); 
					sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|P, 65, 1, 0, 0, 0);
			    break;
					}
				
				addr.sin_family = AF_INET;
				addr.sin_port = htons(IfaceRTU[P].Security.tcp_port);
				addr.sin_addr.s_addr = htonl(INADDR_ANY);
				//РТЙЧСЪЩЧБЕН УПЛЕФ 
				if (bind(IfaceRTU[P].ssd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
					perror("bind");
					//status
					//exit(1);
					close(IfaceRTU[P].ssd);
					IfaceRTU[P].ssd=-1;
					IfaceRTU[P].modbus_mode=MODBUS_PORT_ERROR;
			    ///!!! перенести формирование этой строки в mxshm.c:99[refresh_shm()]
			    strcpy(IfaceRTU[P].bridge_status, "ERR");
					sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|P, 65, 2, 0, 0, 0);
			    break;
					}

				//УФБЧЙН Ч ПЮЕТЕДШ
				listen(IfaceRTU[P].ssd, MAX_TCP_CLIENTS_PER_PORT);
				
				fcntl(IfaceRTU[P].ssd, F_SETFL, fcntl(IfaceRTU[P].ssd, F_GETFL, 0) | O_NONBLOCK);

				sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|P, 65, 3, 0, 0, 0);
			  strcpy(IfaceRTU[P].bridge_status, "00G");
		
				break;

			case BRIDGE_PROXY: ///!!!
				strcpy(IfaceRTU[P].bridge_status, "BPR");

				/// ищем свободный слот для modbus-rtu клиента
			  j=get_current_client();

			  Client[j].status=GW_CLIENT_RTU_SLV;
			  Client[j].iface=P;
				time(&Client[j].connection_time);
				Client[j].disconnection_time=0;
				time(&Client[j].last_activity_time);
			  //clear_stat(&Сlient[j].stat);
				sprintf(Client[j].device_name, "RTU SLAVE P%d", j+1);

				arg=(P<<8)|(j&0xff);
				IfaceRTU[P].rc = pthread_create(
					&IfaceRTU[P].tid_srvr,
					NULL,
					iface_rtu_slave,
					(void *) arg);
				break;

			default: IfaceRTU[P].modbus_mode=MODBUS_PORT_OFF;
			}
		
		if(	(IfaceRTU[P].modbus_mode==GATEWAY_ATM)||
				(IfaceRTU[P].modbus_mode==GATEWAY_RTM)||
				(IfaceRTU[P].modbus_mode==GATEWAY_PROXY)
				)
			if (IfaceRTU[P].rc!=0){
				IfaceRTU[P].modbus_mode=MODBUS_PORT_ERROR;
	      strcpy(IfaceRTU[P].bridge_status, "ERR");
	      // THREAD INITIALIZED
				sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_ERR|P, 41, IfaceRTU[P].rc, 0, 0, 0);
			  }

/// ЗАПОМИНАЕМ ВРЕМЯ ЗАПУСКА ПОРТА
		time(&IfaceRTU[P].Security.start_time);
		}

/// ЗАПУСК ПОТОКА ДЛЯ ОБРАБОТКИ ЗАПРОСОВ НЕПОСРЕДСТВЕННО К MOXA

	pthread_t		moxaTH;
	rc = pthread_create(
		&moxaTH,
		NULL,
		moxa_device,
		NULL);
	operations[0].sem_num=MOXA_MB_DEVICE;
	semop(semaphore_id, operations, 1);

/// ЗАПУСК ПОТОКОВЫХ ФУНКЦИЙ ИНТЕРФЕЙСОВ В РЕЖИМЕ BRIDGE_TCP

	/// инициализация BRIDGE-соединений по ЛВС выполняется ПОСЛЕ инициализации
	/// прослушивающих сокетов GATEWAY-портов
/*	for(i=0; i<MAX_TCP_SERVERS; i++) {
		
    if(tcp_servers[i].mb_slave==0) continue;

		// case BRIDGE_SIMPLE:
		//инициализацию сетевых соединений порта в режиме BRIDGE производим в потоке порта

		//strcpy(IfaceRTU[P].bridge_status, "00B");
		IfaceTCP[i].modbus_mode=BRIDGE_TCP;
		//IfaceTCP[i].current_client=i; ///!!! надо исключить массив tcp_servers
		//arg=i&0xff; ///!!! нужно сделать отдельно такую переменную для каждого интерфейса, т.к.
								/// в потоковую функцию передается не значение, а ссылка на нее.
		//printf("arg:%d\n", arg);
		IfaceTCP[i].rc = pthread_create(
			&IfaceTCP[i].tid_srvr,
			NULL,
			iface_tcp_master,
			&IfaceTCP[i]);

		operations[0].sem_num=MAX_MOXA_PORTS*2+i;
		semop(semaphore_id, operations, 1);
		}*/

/// ЗАПОМИНАЕМ ВРЕМЯ ЗАПУСКА ШЛЮЗА
time(&MoxaDevice.start_time);
//----------   Ш Л Ю З   В   Р А Б О Т Е   ------------

gateway_common_processing();

//----------------- ЗАВЕРШЕНИЕ РАБОТЫ -----------------

	for(i=0; i<MAX_MOXA_PORTS; i++)
	  if(IfaceRTU[i].ssd>=0) {
			shutdown(IfaceRTU[i].ssd, SHUT_RDWR);
	  	close(IfaceRTU[i].ssd);
	  	}

  close_clients(); // условно деструктор модуля CLIETNS_H

  mxlcm_close(mxlcm_handle);
  keypad_close(mxkpd_handle);
  mxbuzzer_close(mxbzr_handle);

	close_shm();
	semctl(semaphore_id, MAX_MOXA_PORTS, IPC_RMID, NULL);

	free(MoxaDevice.wData1x);
	free(MoxaDevice.wData2x);
	free(MoxaDevice.wData3x);
	free(MoxaDevice.wData4x);

	// PROGRAM TERMINATED
	time_t curtime;
	time(&curtime);
	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|GATEWAY_SYSTEM, 44, curtime-MoxaDevice.start_time, 0, 0, 0);

	return (0);
}
///-----------------------------------------------------------------------------------------------------------------
