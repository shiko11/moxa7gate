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

/***   И Н И Ц И А Л И З А Ц И Я   ***/

// требование к процессу инициализации: при возникновении фатальных ошибок должно
// быть выведено сообщение на экран LCM, сообщение на главной странице WEB-интерфейса,
// а так же звук зуммера (при условии успешной инициализации устройств Moxa UC-7410).

	int k;

  init_moxagate_h();
  init_interfaces_h();
  init_frwd_queue_h();
  init_messages_h();

  // вывод возможных ошибок на экран LCM
  k=init_hmi_klb_h();
  if(k!=0) exit(1);

  // создание разделяемого сегмента памяти,
  // выделение памяти под журнал сообщений
  k=init_hmi_web_h();
  if(k!=0) exit(1);

  ///!!! точка останова. возможен контроль средствами HMI инициализации переменных

/*** РАЗБОР ПАРАМЕТРОВ КОМАНДНОЙ СТРОКИ ***/
	k = get_command_line(argc, argv);
	switch(k & 0xff) {

		case COMMAND_LINE_OK:			// CMD LINE: OK
			sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|GATEWAY_SYSTEM, k & 0xff, 0, 0, 0, 0);
			break;

		case COMMAND_LINE_ARGC:
			sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, k & 0xff, 0, 0, 0, 0);
			break;

		case COMMAND_LINE_ERROR:
			sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, k & 0xff, k >> 8, 0, 0, 0);
			break;

		case COMMAND_LINE_INFO: 
			sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|GATEWAY_SYSTEM, k & 0xff, 0, 0, 0, 0);
			break;

		default:;		  // CMD LINE: UNCERTAIN RESULT
			sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, COMMAND_LINE_UNDEFINED, 0, 0, 0, 0);
		}

  if(k!=COMMAND_LINE_OK) {
    close_shm();
		exit(1);
    }

  /// До этого момента все сообщения в программе должны иметь тип EVENT_CAT_MONITOR,
  /// т.к. конфигурация системы сообщений по умолчанию отсеивает все прочие сообщения

  ///!!! точка останова. возможен контроль прочитанной конфигурации средствами HMI
  do {usleep(1000000);} while (Security.halt!=1);
  close_shm();
  printf("stopping program...\n"); exit(1);

/*** ПРОВЕРКА КОНФИГУРАЦИИ ШЛЮЗА В ЦЕЛОМ, ИНИЦИАЛИЗАЦИЯ ВТОРИЧНЫХ КОНФИГУРАЦИОННЫХ ПАРАМЕТРОВ ***/

  k=check_GatewayTCPPorts();
  if(k!=0) {
    sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, CONFIG_TCPPORT_CONFLICT, k >> 8, k & 0xff, 0, 0);
    close_shm();
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
      close_shm();
      exit(1);
      }

  k=check_GatewayIfaces_ex();
  if(k!=0) {
    sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, CONFIG_FORWARDING, k & 0xff, 0, 0, 0);
    close_shm();
    exit(1);
    }

  k=check_GatewayConf();
  if(k!=0) {
    sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, CONFIG_GATEWAY, 0, 0, 0, 0);
    close_shm();
    exit(1);
    }

  k=check_IntegrityAddressMap();
  if(k!=0) {
    sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, CONFIG_ADDRMAP_INTEGRITY, k, 0, 0, 0);
    close_shm();
    exit(1);
    }

  k=check_IntegrityVSlaves();
  if(k!=0) {
    sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, CONFIG_VSLAVES_INTEGRITY, k, 0, 0, 0);
    close_shm();
    exit(1);
    }

  k=check_IntegrityPQueries();
  if(k!=0) {
    sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, CONFIG_PQUERIES_INTEGRITY, k, 0, 0, 0);
    close_shm();
    exit(1);
    }

/*** ИНИЦИАЛИЗАЦИЯ МАССИВОВ ПАМЯТИ ПОД ТАБЛИЦЫ MODBUS ***/

	// блок статусной информации находится в области 4x, выделяем место для него
  // (функция check_IntegrityVSlaves_ex() должна быть выполнена ранее)
  if(MoxaDevice.amount4xRegisters==0) { /// если 4х область адресного пространства не размечена
    MoxaDevice.offset4xRegisters = MoxaDevice.status_info;
    MoxaDevice.amount4xRegisters = GATE_STATUS_BLOCK_LENGTH;
    MoxaDevice.used4xRegisters   = GATE_STATUS_BLOCK_LENGTH;
		} else if(	/// если область 4х размечена и диапазоны регистров не перекрываются
					(MoxaDevice.status_info + GATE_STATUS_BLOCK_LENGTH <= MoxaDevice.offset4xRegisters) ||
					(MoxaDevice.status_info >= MoxaDevice.offset4xRegisters + MoxaDevice.amount4xRegisters)) {
		
      if(MoxaDevice.status_info >= MoxaDevice.offset4xRegisters + MoxaDevice.amount4xRegisters)
        MoxaDevice.amount4xRegisters=\
          MoxaDevice.status_info + GATE_STATUS_BLOCK_LENGTH - MoxaDevice.offset4xRegisters;
        else {
          MoxaDevice.amount4xRegisters += (MoxaDevice.offset4xRegisters - MoxaDevice.status_info);
          MoxaDevice.offset4xRegisters = MoxaDevice.status_info;
          }													

      MoxaDevice.used4xRegisters += GATE_STATUS_BLOCK_LENGTH;
		
      } else { /// ошибка, если диапазоны регистров перекрываются
        // STATUS INFO OVERLAPS
        sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, 39, 0, 0, 0, 0);
        close_shm();
        exit(1);
        }

  // проверяем, что области памяти виртуальных устройств и собственных адресов шлюза не пересекаются
  // (при условии, что они инициализированы)

  k=0;

  if((MoxaDevice.amount1xStatus!=0) && (vsmem_amount1xStatus!=0))
  if(!(
        (vsmem_offset1xStatus + vsmem_amount1xStatus <= MoxaDevice.offset1xStatus) ||
        (vsmem_offset1xStatus >= MoxaDevice.offset1xStatus + MoxaDevice.amount1xStatus)
    )) k=COIL_STATUS_TABLE;

  if((MoxaDevice.amount2xStatus!=0) && (vsmem_amount2xStatus!=0))
  if(!(
        (vsmem_offset2xStatus + vsmem_amount2xStatus <= MoxaDevice.offset2xStatus) ||
        (vsmem_offset2xStatus >= MoxaDevice.offset2xStatus + MoxaDevice.amount2xStatus)
    )) k=INPUT_STATUS_TABLE;

  if((MoxaDevice.amount3xRegisters!=0) && (vsmem_amount3xRegisters!=0))
  if(!(
        (vsmem_offset3xRegisters + vsmem_amount3xRegisters <= MoxaDevice.offset3xRegisters) ||
        (vsmem_offset3xRegisters >= MoxaDevice.offset3xRegisters + MoxaDevice.amount3xRegisters)
    )) k=INPUT_REGISTER_TABLE;

  if((MoxaDevice.amount4xRegisters!=0) && (vsmem_amount4xRegisters!=0))
  if(!(
        (vsmem_offset4xRegisters + vsmem_amount4xRegisters <= MoxaDevice.offset4xRegisters) ||
        (vsmem_offset4xRegisters >= MoxaDevice.offset4xRegisters + MoxaDevice.amount4xRegisters)
    )) k=HOLDING_REGISTER_TABLE;


  if(k!=0) {
    sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, CONFIG_PQUERIES_INTEGRITY, k, 0, 0, 0);
    close_shm();
    exit(1);
    }

  // производим выделение памяти для хранения данных локально
  init_moxagate_memory();
  ///!!! точка останова. возможен контроль рабочих параметров шлюза перед запуском циклов сканирования интерфейсов

	//-------------------------------------------------------
  init_statistics_h();
  init_clients();
	//-------------------------------------------------------
/* ИНИЦИАЛИЗАЦИЯ СЕМАФОРОВ, НА КОТОРЫХ РАБОТАЮТ ОЧЕРЕДИ ПОРТОВ */
	if(init_queue() == 1) exit(1);

/*** ИНИЦИАЛИЗАЦИЯ TCP ПОРТА ШЛЮЗА, ПРИНИМАЮЩЕГО СОЕДИНЕНИЯ КО ВСЕМ ПОРТАМ, ЗА ИСКЛЮЧЕНИЕМ ПОРТОВ GATEWAY_SIMPLE ***/
 if(init_main_socket() !=0 ) exit(1);

/* ЗАПУСК ПОТОКОВЫХ ФУНКЦИЙ, ИНИЦИАЛИЗАЦИЯ ПОСЛЕДОВАТЕЛЬНЫХ ПОРТОВ */
  int i, j;

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

  int rc;
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

  clear_hmi_keypad_lcm_h(); // услвно деструктор модуля HMI_KEYPAD_LCM_H

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
