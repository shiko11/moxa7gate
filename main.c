/*
MOXA7GATE MODBUS GATEWAY SOFTWARE
SEM-ENGINEERING
                    BRYANSK 2009
*/

#include "global.h"

#include "modbus_rtu.h"
#include "modbus_tcp.h"
#include "workers.h"


#define DEBUG_oDATA
#define DEBUG_iDATA

///-----------------------------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
	int				res_cl;
//	u16				tcp_port;
	memset(iDATA,0,sizeof(iDATA)); // must be standart constructor for each data structure
	memset(vslave,0,sizeof(vslave)); // must be standart constructor for each data structure
	memset(query_table,0,sizeof(query_table)); // must be standart constructor for each data structure

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
		iDATA[i].serial.timeout=2000000;
		iDATA[i].tcp_port=1000*i+502;
		
    iDATA[i].current_client=0;

		p_errors[i]=0; // this value for buzzer function
		iDATA[i].start_time=0;

		iDATA[i].queue_start=iDATA[i].queue_len=0;
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
//		gate502.start_time=0;

	res_cl = get_command_line (argc, argv,
								 						 iDATA,
														 vslave,
														 query_table,
														 &_show_data_flow,
														 &_show_sys_messages,
														 &_single_gateway_port_502,
														 &_single_address_space,
														 &_proxy_mode);
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
			printf("Command line parsing error: There are incorrect VSLAVES parameters\n");
			exit(1);
		case CL_ERR_QT_CFG:
			printf("Command line parsing error: There are incorrect QUERY_TABLE entries\n");
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

		if(_proxy_mode==1) {
			printf("Memory allocation (%db)...", sizeof(u16)*PROXY_MODE_REGISTERS);
			oDATA=(u16 *) malloc(sizeof(u16)*PROXY_MODE_REGISTERS);
			if(oDATA==NULL) {
				printf("error\n");
				exit(1);
				}
			printf("OK\n");
			memset(oDATA,0,sizeof(u16)*PROXY_MODE_REGISTERS);	// zdes' ispol'zuem dinamicheskoe vydelenit pamyati
			}

//-------------------------------------------------------	
	mxkpd_handle=keypad_open();
	if (mxkpd_handle < 0 ) sysmsg(EVENT_SOURCE_SYSTEM, "Keypad init ERROR", 1);
	  else sysmsg(EVENT_SOURCE_SYSTEM, "Keypad init OK", 1);

	mxlcm_handle = mxlcm_open();
	if (mxlcm_handle < 0 ) sysmsg(EVENT_SOURCE_SYSTEM, "LCM init ERROR", 1);
	  else sysmsg(EVENT_SOURCE_SYSTEM, "LCM init OK", 1);
  mxlcm_control(mxlcm_handle, IOCTL_LCM_AUTO_SCROLL_OFF);

	mxbzr_handle = mxbuzzer_open();
	if (mxbzr_handle < 0 ) sysmsg(EVENT_SOURCE_SYSTEM, "Buzzer init ERROR", 1);
	  else sysmsg(EVENT_SOURCE_SYSTEM, "Buzzer init OK", 1);

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

	init_shm();
	signal(SIGPIPE, sigpipe_handler);
	signal(SIGIO, sigio_handler);

	int			rc;
	pthread_t		tstTH;
	rc = pthread_create(
		&tstTH,
		NULL,
		mx_keypad_lcm,
		NULL);
	//-------------------------------------------------------
	//-------------------------------------------------------
	struct sockaddr_in	addr;
	int csd, arg, P;
	int ports[MAX_MOXA_PORTS]; // этот массив нужен для организации правильного порядка инициализации портов (сначала GATEWAY, затем остальные
	for(j=0; j<MAX_MOXA_PORTS; j++) ports[j]=0;

	if((_single_gateway_port_502==1)||(_single_address_space==1)) {
		
		key_t sem_key=ftok("/tmp/app", 'b');
		
		//if(MAX_MOXA_PORTS > SEMMSL) {
		//printf("Limit exceeded. Maximal semaphore amount reached: %d\n", SEMMSL);
		//exit(1);
		//}
		if((semaphore_id = semget(sem_key, MAX_MOXA_PORTS, IPC_CREAT|IPC_EXCL|0666)) == -1) {
			printf("ERROR. Semaphore Set already exists?\n");
			exit(1);
		 	}
		
//		int c=0; /* счетчик */
		union semun sems;
		unsigned short values[1];
		//	sems.val = MAX_GATEWAY_QUEUE_LENGTH;
		values[0]=0;
		sems.array = values;
		/* Инициализируем все элементы одним значением */
		semctl(semaphore_id, 0, SETALL, sems);
		
  	///-------------------
		sprintf(eventmsg, "Server socket 502 init ");
		gate502.ssd = socket(AF_INET, SOCK_STREAM, 0);
		if (gate502.ssd < 0) {
			perror("csdet");
	    strcat(eventmsg, "ERROR");
	    sysmsg(EVENT_SOURCE_SYSTEM, eventmsg, 1);
			exit(1);
			}
    strcat(eventmsg, "OK");
    sysmsg(EVENT_SOURCE_SYSTEM, eventmsg, 1);
		
		addr.sin_family = AF_INET;
		addr.sin_port = htons(502);
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		//РТЙЧСЪЩЧБЕН УПЛЕФ 
		sprintf(eventmsg, "Bind Socket 502 ");
		if (bind(gate502.ssd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
			perror("bind");
			close(gate502.ssd);
	    strcat(eventmsg, "ERROR");
	    sysmsg(EVENT_SOURCE_SYSTEM, eventmsg, 1);
			exit(1);
			}
    strcat(eventmsg, "OK");
    sysmsg(EVENT_SOURCE_SYSTEM, eventmsg, 1);

		//УФБЧЙН Ч ПЮЕТЕДШ
		listen(gate502.ssd, MAX_TCP_CLIENTS_PER_PORT*MAX_MOXA_PORTS);
		
		fcntl(gate502.ssd, F_SETFL, fcntl(gate502.ssd, F_GETFL, 0) | O_NONBLOCK);

		for(j=0; j<MAX_MOXA_PORTS; j++)
		  if(iDATA[j].modbus_mode==MODBUS_GATEWAY_MODE) {

				iDATA[j].tcp_port=502;

				if(_single_gateway_port_502==1) strcpy(iDATA[j].bridge_status, "00A");
				if(_single_address_space==1) strcpy(iDATA[j].bridge_status, "00R");
				if(_proxy_mode==1) strcpy(iDATA[j].bridge_status, "00P");

			  int arg=(j<<8)&0xff00;
				if(_proxy_mode!=1) {
				  iDATA[j].clients[0].rc = pthread_create(
					  &iDATA[j].clients[0].tid_srvr,
					  NULL,
					  srvr_tcp_child2,
					  (void *) arg);
					} else {
				  iDATA[j].clients[0].rc = pthread_create(
					  &iDATA[j].clients[0].tid_srvr,
					  NULL,
					  srvr_tcp_proxy,
					  (void *) arg);
						}
			
			  if (iDATA[j].clients[0].rc){
				  iDATA[j].modbus_mode=MODBUS_PORT_ERROR;
	        strcpy(iDATA[j].bridge_status, "ERR");
				  sprintf(eventmsg, "pthread_create() ERROR %d", iDATA[j].clients[0].rc);
		      sysmsg(j, eventmsg, 0);
			    }

				time(&iDATA[j].start_time);
				}

		}

	/// semaphore
	struct sembuf operations[1];
	operations[0].sem_op=1;
	operations[0].sem_flg=0;

	for(i=0; i<MAX_MOXA_PORTS; i++) {
		
		P=0xff;
		for(j=0; j<MAX_MOXA_PORTS; j++)
		  if((iDATA[j].modbus_mode==MODBUS_GATEWAY_MODE)&&(ports[j]==0)) {
		  	ports[j]=1; P=j; break;
		    }
		if(P==0xff) for(j=0; j<MAX_MOXA_PORTS; j++)
		  if((iDATA[j].modbus_mode==MODBUS_BRIDGE_MODE)&&(ports[j]==0)) {
		  	ports[j]=1; P=j; break;
		    }
		if(P==0xff) continue;
		
		// opening serial ports
		if(iDATA[P].modbus_mode!=MODBUS_PORT_OFF) {
	    iDATA[P].serial.fd = open_comm(iDATA[P].serial.p_name, iDATA[P].serial.p_mode);
	    
			iDATA[P].serial.ch_interval_timeout = set_param_comms(iDATA[P].serial.fd, iDATA[P].serial.speed, iDATA[P].serial.parity);
		//	printf("speed %s\n", inputDATA->serial.speed);
		//	printf("parity %s\n", inputDATA->serial.parity);
	    
	    if(0) {iDATA[P].modbus_mode=MODBUS_PORT_OFF; strcpy(iDATA[P].bridge_status, "ERR"); continue;} ///!!!
	    sprintf(eventmsg, "Serial port P%d init OK; mode %d", P+1, iDATA[P].modbus_mode);
			sysmsg(P, eventmsg, 1);
			} else continue;

		switch(iDATA[P].modbus_mode) {
			case MODBUS_GATEWAY_MODE:

				pthread_mutex_init(&iDATA[P].serial_mutex, NULL);

				if((_single_gateway_port_502==1)||(_single_address_space==1)) {
				  //strcpy(iDATA[P].bridge_status, "00G");
					operations[0].sem_num=P;
	  			semop(semaphore_id, operations, 1);
					break;
					}
				
	    	sprintf(eventmsg, "Server socket init for P%d ", P+1);
				iDATA[P].ssd = socket(AF_INET, SOCK_STREAM, 0);
				if (iDATA[P].ssd < 0) {
					perror("csdet");
					iDATA[P].modbus_mode=MODBUS_PORT_ERROR;
			    strcpy(iDATA[P].bridge_status, "ERR");
			    strcat(eventmsg, "ERROR");
			    sysmsg(P, eventmsg, 1);
			    break;
					}
		    strcat(eventmsg, "OK");
		    sysmsg(P, eventmsg, 1);
				
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
			    sysmsg(P, eventmsg, 1);
			    break;
					}
		    strcat(eventmsg, "OK");
		    sysmsg(P, eventmsg, 1);

				//УФБЧЙН Ч ПЮЕТЕДШ
				listen(iDATA[P].ssd, MAX_TCP_CLIENTS_PER_PORT);
				
				fcntl(iDATA[P].ssd, F_SETFL, fcntl(iDATA[P].ssd, F_GETFL, 0) | O_NONBLOCK);
			  strcpy(iDATA[P].bridge_status, "00G");
		
				break;

			/// инициализация BRIDGE-соединений по ЛВС выполняется ПОСЛЕ инициализации
			/// прослушивающих сокетов GATEWAY-портов
			case MODBUS_BRIDGE_MODE:

				//инициализацию сетевых соединений порта в режиме BRIDGE проиводим в потоке порта

				arg=(P<<8)|(iDATA[P].current_client&0xff);
				//printf("arg:%d\n", arg);
				iDATA[P].clients[0].rc = pthread_create(
					&iDATA[P].clients[0].tid_srvr,
					NULL,
					srvr_tcp_bridge,
					(void *) arg);
				
				if (iDATA[P].clients[0].rc){
					sprintf(eventmsg, "pthread_create() ERROR %d", iDATA[P].clients[0].rc);
			    sysmsg(P, eventmsg, 1);
					iDATA[P].modbus_mode=MODBUS_PORT_ERROR;
		      strcpy(iDATA[P].bridge_status, "ERR");
				  } else strcpy(iDATA[P].bridge_status, "00B");
			  break;

			case MODBUS_PROXY_MODE: ///!!!
			default: iDATA[P].modbus_mode=MODBUS_PORT_OFF;
			}
		
		time(&iDATA[P].start_time);
		}

time(&start_time);
//------------------------------------------------------
if((_single_gateway_port_502==0)&&(_single_address_space==0)) gateway_common_processing();
  else gateway_single_port_processing();
//------------------------------------------------------
	for(i=0; i<MAX_MOXA_PORTS; i++)
	  if(iDATA[i].ssd>=0) {
			shutdown(iDATA[i].ssd, SHUT_RDWR);
	  	close(iDATA[i].ssd);
	  	}

  mxlcm_close(mxlcm_handle);
  keypad_close(mxkpd_handle);
  mxbuzzer_close(mxbzr_handle);

	close_shm();
	semctl(semaphore_id, MAX_MOXA_PORTS, IPC_RMID, NULL);

	free(oDATA);

	sysmsg(EVENT_SOURCE_SYSTEM, "Program stopped", 1);

	return (0);
}
///-----------------------------------------------------------------------------------------------------------------
int gateway_common_processing()
  {									
	struct sockaddr_in	addr;
  int i, j, csd, rc;

	while (1) {
		//
	  for(i=0; i<MAX_MOXA_PORTS; i++) {
	  	
		  if(iDATA[i].modbus_mode!=MODBUS_GATEWAY_MODE) continue;
	  	
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

	     sysmsg(i, eventmsg, 0);
			 
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
		    sysmsg(i, eventmsg, 0);
			  }
		  iDATA[i].accepted_connections_number++;
		  iDATA[i].current_connections_number++;
	    }
	  usleep(750000); // proverit' zaderzhku na obrabotku vseh klientov v rezhime MASTER
	  }

	return 0;
  }
///-----------------------------------------------------------------------------------------------------------------
	//pthread_join(csd, NULL);
int gateway_single_port_processing()
	{
	struct sockaddr_in	addr;
  int i, j, k, csd, rc;

    fd_set watchset;
		fd_set inset;

		FD_ZERO(&watchset);

	struct timeval stv;
	stv.tv_sec=0; stv.tv_usec=0;

	while (1) {

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
  	    sysmsg(EVENT_SOURCE_SYSTEM, "accept error", 1);
			} else {

	   if(gate502.current_client==MAX_MOXA_PORTS*MAX_TCP_CLIENTS_PER_PORT) {
			 iDATA[0].rejected_connections_number++;
			 
			 sprintf(eventmsg, "Conn rejected %d.%d.%d.%d",
			 															 addr.sin_addr.s_addr >> 24,
																		 (addr.sin_addr.s_addr >> 16) & 0xff,
																		 (addr.sin_addr.s_addr >> 8) & 0xff,
																		 addr.sin_addr.s_addr & 0xff);

	     sysmsg(i, eventmsg, 0);
			 
			 close(csd);
	     } else {
	   		 gate502.clients[gate502.current_client].csd=csd;

				 FD_SET(gate502.clients[gate502.current_client].csd, &watchset);
	  	
			 printf("Slot %d connection for TCP502: %d.%d.%d.%d\n", gate502.current_client+1,
			 															 addr.sin_addr.s_addr >> 24,
																		 (addr.sin_addr.s_addr >> 16) & 0xff,
																		 (addr.sin_addr.s_addr >> 8) & 0xff,
																		 addr.sin_addr.s_addr & 0xff);
	     //sysmsg(i, eventmsg, 0);
			
				 time(&gate502.clients[gate502.current_client].connection_time);
				 time(&gate502.clients[gate502.current_client].last_activity_time);
			   gate502.clients[gate502.current_client].ip=addr.sin_addr.s_addr;
			   gate502.clients[gate502.current_client].port=addr.sin_port;
			   gate502.clients[gate502.current_client].connection_status=MB_CONNECTION_ESTABLISHED;
			   clear_stat(&gate502.clients[gate502.current_client].stat);
			//printf("ip%X\n", addr.sin_addr.s_addr);
			
		  	iDATA[0].accepted_connections_number++;
		  	iDATA[0].current_connections_number++;
				}
		  }

			//printf("input data processing...\n");

		///### блок обработки входящих ModbusTCP запросов
		u8			tcp_adu[MB_TCP_MAX_ADU_LENGTH];// TCP ADU
		u16			tcp_adu_len;
		u8			memory_adu[MB_SERIAL_MAX_ADU_LEN];
		u16			memory_adu_len;
		GW_StaticData tmpstat;
		int port_id, device_id;
		int status;

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
						iDATA[0].current_connections_number--;
			    sprintf(eventmsg, "Slot %d disc(tmt): %d.%d.%d.%d\n", i+1,
			 															 addr.sin_addr.s_addr >> 24,
																		 (addr.sin_addr.s_addr >> 16) & 0xff,
																		 (addr.sin_addr.s_addr >> 8) & 0xff,
																		 addr.sin_addr.s_addr & 0xff);
		  			sysmsg(EVENT_SOURCE_GATE502|(i<<8), eventmsg, 0);
						};
      }

	    if(maxfd<gate502.clients[i].csd) maxfd=gate502.clients[i].csd;
	    }

			clear_stat(&tmpstat);
			
			inset=watchset;

		int temp;
			if((temp=select(maxfd+1, &inset, NULL, NULL, &stv))<0) {
				perror("select");
				continue;
				}
		if(temp==0) continue;
																												 
	  for(i=0; i<MAX_MOXA_PORTS*MAX_TCP_CLIENTS_PER_PORT; i++) 
			if (gate502.clients[i].connection_status==MB_CONNECTION_ESTABLISHED)
			if (FD_ISSET(gate502.clients[i].csd, &inset)) {
	  	
			if(_show_data_flow) printf("TCP%4.4d  IN: ", gate502.clients[i].port);
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
	  			sysmsg(EVENT_SOURCE_GATE502|(i<<8), eventmsg, 0);
					//update_stat(&inputDATA->clients[client_id].stat, &tmpstat);
					//update_stat(&iDATA[port_id].stat, &tmpstat);
				  if(status==TCP_COM_ERR_NULL) {
					  FD_CLR(gate502.clients[i].csd, &watchset);
					  gate502.clients[i].connection_status=MB_CONNECTION_CLOSED;
					  shutdown(gate502.clients[i].csd, SHUT_RDWR);
					  close(gate502.clients[i].csd);
					  gate502.clients[i].csd=-1;
						iDATA[0].current_connections_number--;
			    sprintf(eventmsg, "Slot %d disc(lnkoff): %d.%d.%d.%d\n", i+1,
			 															 addr.sin_addr.s_addr >> 24,
																		 (addr.sin_addr.s_addr >> 16) & 0xff,
																		 (addr.sin_addr.s_addr >> 8) & 0xff,
																		 addr.sin_addr.s_addr & 0xff);
		  			sysmsg(EVENT_SOURCE_GATE502|(i<<8), eventmsg, 0);
						}
					continue;
			  	break;
			  default:; ///!!! continue;
			  };

			time(&gate502.clients[i].last_activity_time);

			/// отображение адреса modbus-slave и адресного пространства
			if(_proxy_mode!=1) {
			if(_single_address_space==1) {

				switch(tcp_adu[7]) { ///!!!
					case MBF_WRITE_SINGLE_REGISTER: temp=1; break;
					default: temp=(tcp_adu[10]<<8)|tcp_adu[11];
					}

				status=translateRegisters(((tcp_adu[8]<<8)|tcp_adu[9])&0xffff, temp, &port_id, &device_id);
			  if(status) {
					sprintf(eventmsg, "Reg translation error %d P%d CL%d", status, port_id+1, i);
	  			sysmsg(EVENT_SOURCE_GATE502|(i<<8), eventmsg, 0);
					continue;
					}

				//tcp_adu[8]=j&0xff;
				//tcp_adu[9]=(j>>8)&0xff;

				} else {

					translateAddress(tcp_adu[6], &port_id, &device_id);
					tcp_adu[6]=device_id;

				  if(iDATA[port_id].modbus_mode!=MODBUS_GATEWAY_MODE) {
						sprintf(eventmsg, "invalid Unit ID P%d CL%d", port_id+1, i);
		  			sysmsg(EVENT_SOURCE_GATE502|(i<<8), eventmsg, 0);
						continue;
						}
					}
					}

			/// диагностические данные обновляются в потоке вывода на LCM

			if(_proxy_mode!=1) {/// если нет режима PROXY, ставим запрос в очередь последовательного порта

				if((status=enqueue_query(port_id, i, device_id, tcp_adu, tcp_adu_len))!=0) continue;

				} else { // в режиме PROXY выдаем данные из локального буфера

				switch(tcp_adu[7]) {

					case MBF_READ_HOLDING_REGISTERS:
				j=((tcp_adu[8]<<8)|tcp_adu[9])&0xffff;
				k=((tcp_adu[10]<<8)|tcp_adu[11])&0xffff;
			  if((j+k)>=PROXY_MODE_REGISTERS) {
					sprintf(eventmsg, "Proxy mode error: query addressing\n");
	  			sysmsg(EVENT_SOURCE_GATE502|(i<<8), eventmsg, 0);
					continue;
					}

			memory_adu[0]=tcp_adu[6]; //device ID
			memory_adu[1]=tcp_adu[7]; //ModBus Function Code
			memory_adu[2]=2*k; 				//bytes total
			int n;
//			for(n=0; n<(2*k); n++)
//        memory_adu[3+n]=oDATA[2*j+n];
			for(n=0; n<k; n++) {
        memory_adu[3+2*n]=(oDATA[j+n]>>8)&0xff;
        memory_adu[3+2*n+1]=oDATA[j+n]&0xff;
				}
			memory_adu_len=2*k+3+2;
      	break;

			case MBF_WRITE_SINGLE_REGISTER:
				
				status=translateProxyDevice(((tcp_adu[8]<<8)|tcp_adu[9])&0xffff, 1, &port_id, &device_id);
			  if(status) {
					sprintf(eventmsg, "Proxy translation error %d P%d CL%d", status, port_id+1, i);
	  			sysmsg(EVENT_SOURCE_GATE502|(i<<8), eventmsg, 0);
					continue;
					}

				if((status=enqueue_query(port_id, i, device_id, tcp_adu, tcp_adu_len))!=0) continue;

				continue;
				break;

			default:;
					sprintf(eventmsg, "Proxy mode error: unsupported mbf\n");
	  			sysmsg(EVENT_SOURCE_GATE502|(i<<8), eventmsg, 0);
					continue;
			}

			if(_show_data_flow) printf("TCP%4.4d OUT: ", gate502.clients[i].port);
			status = mb_tcp_send_adu(gate502.clients[i].csd,
																&tmpstat, memory_adu, memory_adu_len-2, tcp_adu, &tcp_adu_len);


		switch(status) {
		  case 0:
		  	tmpstat.sended++; ///!!!
		  	break;
		  case TCP_COM_ERR_SEND:
		  	tmpstat.errors++;
				sprintf(eventmsg, "tcp send error: %d", status);
  			sysmsg(port_id, eventmsg, 0);
		  	break;
		  default:;
		  };


					}
	    }

	  //usleep(500000); // proverit' zaderzhku na obrabotku vseh klientov v rezhime MASTER
	  }

	return 0;
	}
///-----------------------------------------------------------------------------------------------------------------
int enqueue_query(int port_id, int client_id, int device_id, u8 *tcp_adu, u16 tcp_adu_len)
  {

			  if(iDATA[port_id].queue_len==MAX_GATEWAY_QUEUE_LENGTH) { ///!!! modbus exception response
					sprintf(eventmsg, "queue overloaded P%d CL%d", port_id+1, client_id);
	  			sysmsg(EVENT_SOURCE_GATE502|(client_id<<8), eventmsg, 0);
					return 1;
					}
	
		  	pthread_mutex_lock(&iDATA[port_id].serial_mutex);
	
				int j, queue_current=(iDATA[port_id].queue_start+iDATA[port_id].queue_len)%MAX_GATEWAY_QUEUE_LENGTH;
	
				//printf("query_queued ?%dP%d\n", queue_current, port_id+1);

				for(j=0; j<tcp_adu_len; j++)
					iDATA[port_id].queue_adu[queue_current][j]=tcp_adu[j];
				iDATA[port_id].queue_adu_len[queue_current]=tcp_adu_len;
				iDATA[port_id].queue_clients[queue_current]=client_id;
				iDATA[port_id].queue_slaves[queue_current]=device_id; ///!!! ATM & PROXY uses this field
	
				iDATA[port_id].queue_len++;
	
	///$$$	struct timeval tv1, tv2;
	///$$$	struct timezone tz;
	//(tv2.tv_sec%10)*1000+tv2.tv_usec/1000
	///$$$		gettimeofday(&tv2, &tz);
	///$$$			printf("\nP%d TCP%4.4d time%d query %d begin[%d]", port_id+1, gate502.clients[i].port, (tv2.tv_sec%10)*10+tv2.tv_usec/100000, iDATA[port_id].queue_len, queue_current);
	
		  	pthread_mutex_unlock(&iDATA[port_id].serial_mutex);
				//printf("mutex passed\n");
	
	/// semaphore
	struct sembuf operations[1];
	operations[0].sem_op=1;
	operations[0].sem_flg=0;

				/// semaphore
				operations[0].sem_num=port_id;
		  	semop(semaphore_id, operations, 1);
				//printf("semaphore passed\n");

  return 0;
  }
///-----------------------------------------------------------------------------------------------------------------
