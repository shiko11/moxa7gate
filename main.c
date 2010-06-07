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
	memset(iDATA,0,sizeof(iDATA));
	memset(vslave,0,sizeof(vslave));
	memset(query_table,0,sizeof(query_table));

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
		gate502.start_time=0;

		gate502.show_data_flow=0;
		gate502.show_sys_messages=0;
		gate502.watchdog_timer=0;
		gate502.use_buzzer=0;
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

		gate502.halt=0;
	
		init_shm();

		signal(SIGPIPE, sigpipe_handler);
		signal(SIGIO, sigio_handler);

/*** ������ ���������� ��������� ������ ***/

	res_cl = get_command_line (argc, argv,
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

/*** ������������� �������� ������ ��� ������� MODBUS ***/

/* � ���������� ������� ���������� ��������� ������ �������� ���������� ����������,
   ������� ����� ��������� ��������. �������������� ���������� ��������� ������ ��� ��� */

	unsigned k;

	amount1xStatus=16;
	amount2xStatus=16;
	amount3xRegisters=1;

	amount4xRegisters=0;
	offset4xRegisters=0xffff;
	for(i=0; i<MAX_QUERY_ENTRIES; i++) {																	
		if((query_table[i].offset==0)||(query_table[i].length==0)) continue;
		if(offset4xRegisters>query_table[i].offset) offset4xRegisters=query_table[i].offset;
		if(amount4xRegisters<query_table[i].offset+query_table[i].length)
			amount4xRegisters=query_table[i].offset+query_table[i].length;
		}
	//printf("offset4xRegisters=%d, amount4xRegisters=%d\n", offset4xRegisters, amount4xRegisters);
	if((amount4xRegisters==0)&&(gate502.status_info!=0)) {
		offset4xRegisters=gate502.status_info-1;
		amount4xRegisters=gate502.status_info-1+GATE_STATUS_BLOCK_LENGTH;
		} else if(	/// ���� ��������� ��������� �� �������������
				(gate502.status_info-1+GATE_STATUS_BLOCK_LENGTH <= offset4xRegisters) ||
				(gate502.status_info-1 >= amount4xRegisters)) {
	
			if(gate502.status_info-1 >= amount4xRegisters)
				amount4xRegisters=gate502.status_info-1+GATE_STATUS_BLOCK_LENGTH;
				else offset4xRegisters=gate502.status_info-1;
	
			} else	{
				printf("gate status info registers overlaps with others... so they are disabled\n");
				gate502.status_info=0;
				}

	amount4xRegisters-=offset4xRegisters;

  // ��������� ������ ��� ������� 1x
	if(amount1xStatus>0) {
		k=sizeof(u16)*((amount1xStatus-1)/16+1);
		printf("Memory allocation 1x (%db)...", k);
		wData1x=(u16 *) malloc(k);
		if(wData1x==NULL) {
			printf("error\n");
			exit(1);
			}
		memset(wData1x,0, k);
		printf("OK\n");
		}

  // ��������� ������ ��� ������� 2x
	if(amount2xStatus>0) {
		k=sizeof(u16)*((amount2xStatus-1)/16+1);
		printf("Memory allocation 2x (%db)...", k);
		wData2x=(u16 *) malloc(k);
		if(wData2x==NULL) {
			printf("error\n");
			exit(1);
			}
		memset(wData2x,0, k);
		printf("OK\n");
		}

  // ��������� ������ ��� ������� 3x
	if(amount3xRegisters>0) {
		printf("Memory allocation 3x (%db)...", sizeof(u16)*amount3xRegisters);
		wData3x=(u16 *) malloc(sizeof(u16)*amount3xRegisters);
		if(wData3x==NULL) {
			printf("error\n");
			exit(1);
			}
		memset(wData3x,0,sizeof(u16)*amount3xRegisters);
		printf("OK\n");
		}

  // ��������� ������ ��� ������� 4x
	if(amount4xRegisters>0) {
		printf("Memory allocation 4x (%db)...", sizeof(u16)*amount4xRegisters);
		wData4x=(u16 *) malloc(sizeof(u16)*amount4xRegisters);
		if(wData4x==NULL) {
			printf("error\n");
			exit(1);
			}
		memset(wData4x,0,sizeof(u16)*amount4xRegisters);
		printf("OK\n");
		}

/*** ������������� ��������� �������� � �����������: LCM, KEYPAD, BUZZER (�������, ����������, ������) ***/
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

/// ��������� ����� ��� ��������� ����� � ���������� � ������ �� ������� ��������� ����������
	int			rc;
	pthread_t		tstTH;
	rc = pthread_create(
		&tstTH,
		NULL,
		mx_keypad_lcm,
		NULL);
	//-------------------------------------------------------

//  printf("stopping program...\n"); exit(1);


	struct sockaddr_in	addr;
	int csd, arg, P;

	// ���� ������ ������ ��� ����������� ������� ������������� ������ (������� GATEWAY_SIMPLE, ����� ���������)
	int ports[MAX_MOXA_PORTS]; 
	memset(ports, 0, sizeof(ports));

/* ������������� ���������, �� ������� �������� ������� ������ */

		key_t sem_key=ftok("/tmp/app", 'b');
		
		if((semaphore_id = semget(sem_key, MAX_MOXA_PORTS+1, IPC_CREAT|IPC_EXCL|0666)) == -1) {
			printf("ERROR. Semaphore Set already exists?\n");
			exit(1);
		 	}
		
		union semun sems;
		unsigned short values[1];

		values[0]=0;
		sems.array = values;
		/* �������������� ��� �������� ����� ��������� */
		semctl(semaphore_id, 0, SETALL, sems);
		
		struct sembuf operations[1];
		operations[0].sem_op=1;
		operations[0].sem_flg=0;

/*** ������������� TCP ����� �����, ������������ ���������� �� ���� ������, �� ����������� ������ GATEWAY_SIMPLE ***/
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
		//����������� ����� 
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

/* ������ ��������� �������, ������������� ���������������� ������ */

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
	    
	    if(0) {iDATA[P].modbus_mode=MODBUS_PORT_OFF; strcpy(iDATA[P].bridge_status, "ERR"); continue;} ///!!!
	    sprintf(eventmsg, "Serial port P%d init OK; mode %d", P+1, iDATA[P].modbus_mode);
			sysmsg(P, 0, eventmsg, 1);
			} else continue;

		switch(iDATA[P].modbus_mode) {
			case GATEWAY_ATM:

				strcpy(iDATA[P].bridge_status, "00A");
			  arg=(P<<8)&0xff00;
				  iDATA[P].clients[0].rc = pthread_create(
					  &iDATA[P].clients[0].tid_srvr,
					  NULL,
					  srvr_tcp_child2,
					  (void *) arg);

				operations[0].sem_num=P;
  			semop(semaphore_id, operations, 1);
				break;

			case GATEWAY_RTM:

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
				  iDATA[P].clients[0].rc = pthread_create(
					  &iDATA[P].clients[0].tid_srvr,
					  NULL,
					  srvr_tcp_proxy,
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
				//����������� ����� 
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

				//������ � �������
				listen(iDATA[P].ssd, MAX_TCP_CLIENTS_PER_PORT);
				
				fcntl(iDATA[P].ssd, F_SETFL, fcntl(iDATA[P].ssd, F_GETFL, 0) | O_NONBLOCK);
			  strcpy(iDATA[P].bridge_status, "00G");
		
				break;

			/// ������������� BRIDGE-���������� �� ��� ����������� ����� �������������
			/// �������������� ������� GATEWAY-������
			case BRIDGE_SIMPLE:
				//������������� ������� ���������� ����� � ������ BRIDGE ��������� � ������ �����

				strcpy(iDATA[P].bridge_status, "0BS");

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
					srvr_tcp_bridge_proxy,
					(void *) arg);
				break;

			default: iDATA[P].modbus_mode=MODBUS_PORT_OFF;
			}
		
		if(	(iDATA[P].modbus_mode==GATEWAY_ATM)||
				(iDATA[P].modbus_mode==GATEWAY_RTM)||
				(iDATA[P].modbus_mode==GATEWAY_PROXY)||
				(iDATA[P].modbus_mode==BRIDGE_SIMPLE)
				)
			if (iDATA[P].clients[0].rc){
				sprintf(eventmsg, "pthread_create() ERROR %d", iDATA[P].clients[0].rc);
		    sysmsg(P, 0, eventmsg, 1);
				iDATA[P].modbus_mode=MODBUS_PORT_ERROR;
	      strcpy(iDATA[P].bridge_status, "ERR");
			  }

/// ���������� ����� ������� �����
		time(&iDATA[P].start_time);
		}

/// ������ ������ ��� ��������� �������� ��������������� � MOXA

	pthread_t		moxaTH;
	rc = pthread_create(
		&moxaTH,
		NULL,
		moxa_mb_thread,
		NULL);
	operations[0].sem_num=MOXA_MB_DEVICE;
	semop(semaphore_id, operations, 1);

/// ���������� ����� ������� �����
time(&gate502.start_time);
/*/----------   � � � �   �   � � � � � �   ------------
if((_single_gateway_port_502==0)&&(_single_address_space==0)) gateway_common_processing();
  else gateway_single_port_processing();*/
gateway_common_processing();
//----------- ���������� ������ ���������� -------------
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

  mxlcm_close(mxlcm_handle);
  keypad_close(mxkpd_handle);
  mxbuzzer_close(mxbzr_handle);

	close_shm();
	semctl(semaphore_id, MAX_MOXA_PORTS, IPC_RMID, NULL);

	free(wData1x);
	free(wData2x);
	free(wData3x);
	free(wData4x);

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
		//
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
		/// ��������� �������� ���������� �� ���� 502
		gateway_single_port_processing();
		/// �������� ���������� ��������
		query_translating();
	  
		// ������� ��������� �� ������� �������
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

			///### ���� ��������� �������� TCP-����������
			/// ���� ��������� ���� ��� ������ ����������
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
			 iDATA[0].rejected_connections_number++;
			 
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
			
		  	iDATA[0].accepted_connections_number++;
		  	iDATA[0].current_connections_number++;
				}
		  }


	  //usleep(500000); // proverit' zaderzhku na obrabotku vseh klientov v rezhime MASTER

	return 0;
	}
///-----------------------------------------------------------------------------------------------------------------
int enqueue_query(int port_id, int client_id, int device_id, u8 *tcp_adu, u16 tcp_adu_len)
  {

		if(port_id!=MOXA_MB_DEVICE) {

			  if(iDATA[port_id].queue_len==MAX_GATEWAY_QUEUE_LENGTH) { ///!!! modbus exception response
					sprintf(eventmsg, "queue overloaded P%d CL%d", port_id+1, client_id);
	  			sysmsg(EVENT_SOURCE_GATE502|(client_id<<8), 0, eventmsg, 0);
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

			} else { /// ������ � �������� MOXA MODBUS DEVICE

			  if(gate502.queue_len==MAX_GATEWAY_QUEUE_LENGTH) { ///!!! modbus exception response
					sprintf(eventmsg, "queue overloaded moxa_mb_device CL%d", client_id);
	  			sysmsg(EVENT_SOURCE_GATE502|(client_id<<8), 0, eventmsg, 0);
					return 1;
					}
	
		  	pthread_mutex_lock(&gate502.moxa_mutex);
	
				int j, queue_current=(gate502.queue_start+gate502.queue_len)%MAX_GATEWAY_QUEUE_LENGTH;
	
				//printf("query_queued ?%dP%d\n", queue_current, port_id+1);

				for(j=0; j<tcp_adu_len; j++)
					gate502.queue_adu[queue_current][j]=tcp_adu[j];
				gate502.queue_adu_len[queue_current]=tcp_adu_len;
				gate502.queue_clients[queue_current]=client_id;
//				gate502.queue_slaves[queue_current]=device_id;
	
				gate502.queue_len++;
	
	///$$$	struct timeval tv1, tv2;
	///$$$	struct timezone tz;
	//(tv2.tv_sec%10)*1000+tv2.tv_usec/1000
	///$$$		gettimeofday(&tv2, &tz);
	///$$$			printf("\nP%d TCP%4.4d time%d query %d begin[%d]", port_id+1, gate502.clients[i].port, (tv2.tv_sec%10)*10+tv2.tv_usec/100000, iDATA[port_id].queue_len, queue_current);
	
		  	pthread_mutex_unlock(&gate502.moxa_mutex);
				//printf("mutex passed\n");
	
	/// semaphore
	struct sembuf operations[1];
	operations[0].sem_op=1;
	operations[0].sem_flg=0;

				/// semaphore
				operations[0].sem_num=MOXA_MB_DEVICE;
		  	semop(semaphore_id, operations, 1);
				//printf("semaphore passed\n");
				}

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

		///### ���� ��������� �������� ModbusTCP ��������
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

	  for(i=0; i<MAX_MOXA_PORTS*MAX_TCP_CLIENTS_PER_PORT; i++) { // ��������� ������ �� ��������

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
	  			sysmsg(EVENT_SOURCE_GATE502|(i<<8), 0, eventmsg, 0);
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
		  			sysmsg(EVENT_SOURCE_GATE502|(i<<8), 0, eventmsg, 0);
						}
					return 0;
			  	break;
			  default:; ///!!! continue;
			  };

			time(&gate502.clients[i].last_activity_time);


/* ������ ��������� ������� ���������� � ��������� ������:
	- ��������� modbus �����, ��������� � �������;
	- ��������� �������� ���������, ��������� � �������;
	- ���� � ����� � �������� ����������� ���������� Moxa, ������ ������ � ������� ���������� ��������;
	- ���� ������ ����� ����������� ���������� Moxa, ������ ������ � ������� ������ �� ������ ������ RTM;
	- ���� ����� �� ����������� ���������� Moxa, ������ ������ � ������� ������ �� ������ ������ ATM.
 */

			k=tcp_adu[TCPADU_ADDRESS]==gate502.modbus_address?GATEWAY_SIMPLE:GATEWAY_ATM;

			if(k!=GATEWAY_ATM) {
				switch(
							checkDiapason(	tcp_adu[TCPADU_FUNCTION],
															(tcp_adu[TCPADU_START_HI]<<8)|tcp_adu[TCPADU_START_LO],
															(tcp_adu[TCPADU_LEN_HI]<<8)|tcp_adu[TCPADU_LEN_LO])
					) {
	
					case 	MOXA_DIAPASON_INSIDE:
						k=GATEWAY_PROXY;
						/// ������ ������ � ������� MOXA MODBUS DEVICE
						if((status=enqueue_query(MOXA_MB_DEVICE, i, device_id, tcp_adu, tcp_adu_len))!=0) continue;
//						printf("GATE502 GATEWAY_PROXY\n");
						break;
					
					case MOXA_DIAPASON_OUTSIDE:
						k=GATEWAY_RTM;
	
						switch(tcp_adu[TCPADU_FUNCTION]) { ///!!!
							case MBF_WRITE_SINGLE_REGISTER: temp=1; break;
							default: temp=(tcp_adu[TCPADU_LEN_HI]<<8)|tcp_adu[TCPADU_LEN_LO];
							}
		
						status=translateRegisters(
							(tcp_adu[TCPADU_START_HI]<<8)|tcp_adu[TCPADU_START_LO],
							temp, &port_id, &device_id);

					  if(status) {
							sprintf(eventmsg, "Reg translation error %d P%d CL%d", status, port_id+1, i);
			  			sysmsg(EVENT_SOURCE_GATE502|(i<<8), 0, eventmsg, 0);
							return 0;
							}

						/// ������ ������ � ������� ����������������� �����
						if((status=enqueue_query(port_id, i, device_id, tcp_adu, tcp_adu_len))!=0) continue;
//						printf("GATE502 GATEWAY_RTM\n");
						break;
					
					case MOXA_DIAPASON_OVERLAPPED:
						printf("GATE502 MOXA_DIAPASON_OVERLAPPED\n");
						break;
	
					default:;
					}
				} else { /// ��������� ������� � ������ ATM

					translateAddress(tcp_adu[TCPADU_ADDRESS], &port_id, &device_id);
					tcp_adu[TCPADU_ADDRESS]=device_id;

				  if((iDATA[port_id].modbus_mode!=GATEWAY_ATM)&&(iDATA[port_id].modbus_mode!=GATEWAY_RTM)) {
						sprintf(eventmsg, "invalid Unit ID P%d CL%d", port_id+1, i);
		  			sysmsg(EVENT_SOURCE_GATE502|(i<<8), 0, eventmsg, 0);
						return 0;
						}

					/// ������ ������ � ������� ����������������� �����
//printf("enqueue_query port_id=%d, client_id=%d, device_id=%d\n", port_id, i, device_id);
					if((status=enqueue_query(port_id, i, device_id, tcp_adu, tcp_adu_len))!=0) continue;
					}

	    }

  return 0;
  }
///-----------------------------------------------------------------------------------------------------------------
