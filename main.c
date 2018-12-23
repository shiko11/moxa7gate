/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///********* ���������� MOXA7GATE, ������� ������ ��������� ********************

///=== MAIN_H IMPLEMENTATION

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "main.h"

///=== MAIN_H private variables

//struct timeval tv_mem;
//struct timezone tz;
//unsigned int p_errors[MAX_MOXA_PORTS];
struct sockaddr_in	addr;

///=== MAIN_H private functions

///-----------------------------------------------------------------------------------------------------------------
// ���� �������� �� ������� main ������������� ������ ������ ����������:
// 0 - ���������� ����������� ������� ������� (�� ������� �� HMI, ���� �������������)
// 1 - �������� ������ ��� ������ ���������� ������������;
// 2 - �������� ������ ��� �������� ��������� ����������� ���������� ������������;
// 3 - �������� ������ ��� ������������� ��������� ����������, ��������� �������� ��� ���� ����������;
// 4 - �������� ��������� ������ � ���� ������ ����������;
int main(int argc, char *argv[])
{

/***   � � � � � � � � � � � � �   ***/

// ���������� � �������� �������������: ��� ������������� ��������� ������ ������
// ���� �������� ��������� �� ����� LCM, ��������� �� ������� �������� WEB-����������,
// � ��� �� ���� ������� (��� ������� �������� ������������� ��������� Moxa UC-7410).

	int k;

  int i, j;

	struct sembuf operations[1];

	int arg, P, T;
	int ports[MAX_MOXA_PORTS]; // ���� ������ ������ ��� ����������� ������� ������������� ������: ������� IFACE_TCPSERVER, ����� ���������

  int rc;
	pthread_t		moxaTH;

	time_t curtime;

  init_moxagate_h();
  init_interfaces_h();
  init_frwd_queue_h();

  k=init_messages_h();
  if(k!=0) {
    // ��� ��� ������ ��������� �� ���������������, ������� ��������� �����
    printf("FTL HMI\tINI: CONFIGURED MESSAGE TEMPLATE IS TOO LONG");
    exit(1);
    }
									 
  // ���������� ����� ����������������:
  shm_segment_ok=-1;

#ifndef MOXA7GATE_WITHOUT_HMI_KLB
  // ����� ��������� ������ �� ����� LCM
  k=init_hmi_klb_h();
  if(k!=0) exit(3);
#endif

  // �������� ������������ �������� ������,
  // ��������� ������ ��� ������ ���������
  k=init_hmi_web_h();
  if(k!=0) exit(3);

	//-------------------------------------------------------
  init_statistics_h();
  init_clients();

  kltm_port=GATEWAY_NONE;

  ///!!! ����� ��������. �������� �������� ���������� HMI ������������� ����������

/*** ������ ���������� ��������� ������ ***/
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

#ifdef MOXA7GATE_KM400
  if(kltm_port == GATEWAY_NONE) {
  	printf("check kltm_port CLI parameter\n");
    close_shm();
		exit(1);
    }
#endif

  /// �� ����� ������� ��� ��������� � ��������� ������ ����� ��� EVENT_CAT_MONITOR,
  /// �.�. ������������ ������� ��������� �� ��������� ��������� ��� ������ ���������

  ///!!! ����� ��������. �������� �������� ����������� ������������ ���������� HMI

  update_shm(); // �������� ������ � ������������ � ����������� ������� ������

/*** �������� ������������ ����� � �����, ������������� ��������� ���������������� ���������� ***/

  k=check_GatewayTCPPorts();
  if(k!=0) {
    sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, CONFIG_TCPPORT_CONFLICT, k >> 8, k & 0xff, 0, 0);
    close_shm();
    exit(2);
    }

  k=check_GatewayAddressMap();
  if((k&0xff)!=0) { // ���� � ������� ����������� TCP-��������� ��� ������ M7G ����������� ��� ���� �����������
      sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, CONFIG_ADDRMAP, k & 0xff, k >> 8, 0, 0);
      close_shm();
      exit(2);
      }

  k=check_GatewayIfaces_ex();
  if(k!=0) {
    sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, CONFIG_FORWARDING, k & 0xff, 0, 0, 0);
    close_shm();
    exit(2);
    }

  k=check_GatewayConf();
  if(k!=0) {
    sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, CONFIG_GATEWAY, 0, 0, 0, 0);
    close_shm();
    exit(2);
    }

  k=check_IntegrityAddressMap();
  if(k!=0) {
    sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, CONFIG_ADDRMAP_INTEGRITY, k, 0, 0, 0);
    close_shm();
    exit(2);
    }

  k=check_IntegrityVSlaves();
  if(k!=0) {
  	// CONFIG_VSLAVES_INTEGRITY_IFACES, CONFIG_VSLAVES_INTEGRITY_DIAP
    sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, k>>16, (k&0xff)+1, ((k>>8)&0xff)+1, 0, 0);
    close_shm();
    exit(2);
    }

  k=check_IntegrityPQueries();
  if(k!=0) {
		// CONFIG_PQUERIES_INTEGRITY_IFACE, CONFIG_PQUERIES_INTEGRITY_DIAP
    sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, k>>16, (k&0xff)+1, ((k>>8)&0xff)+1, 0, 0);
    close_shm();
    exit(2);
    }

/*** ������������� �������� ������ ��� ������� MODBUS ***/

	// ���� ��������� ���������� ��������� � ������� 4x, �������� ����� ��� ����
  // (������� check_IntegrityPQueries() ������ ���� ��������� �����)
  if(MoxaDevice.amount4xRegisters==0) { /// ���� 4� ������� ��������� ������������ �� ���������
    MoxaDevice.offset4xRegisters = MoxaDevice.status_info;
    MoxaDevice.amount4xRegisters = GATE_STATUS_BLOCK_LENGTH;
    MoxaDevice.used4xRegisters   = GATE_STATUS_BLOCK_LENGTH;
		} else if(	/// ���� ������� 4� ��������� � ��������� ��������� �� �������������
					(MoxaDevice.status_info + GATE_STATUS_BLOCK_LENGTH <= MoxaDevice.offset4xRegisters) ||
					(MoxaDevice.status_info >= MoxaDevice.offset4xRegisters + MoxaDevice.amount4xRegisters)) {
		
      if(MoxaDevice.status_info >= MoxaDevice.offset4xRegisters + MoxaDevice.amount4xRegisters)
        MoxaDevice.amount4xRegisters =
          MoxaDevice.status_info + GATE_STATUS_BLOCK_LENGTH - MoxaDevice.offset4xRegisters;
        else {
          MoxaDevice.amount4xRegisters += (MoxaDevice.offset4xRegisters - MoxaDevice.status_info);
          MoxaDevice.offset4xRegisters = MoxaDevice.status_info;
          }													

      MoxaDevice.used4xRegisters += GATE_STATUS_BLOCK_LENGTH;
		
      } else { /// ������, ���� ��������� ��������� �������������
        // MOXAGATE_STATINFO_OVERLAPS
        sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, MOXAGATE_STATINFO_OVERLAPS, 0, 0, 0, 0);
        close_shm();
        exit(2);
        }

  // ���������, ��� ������� ������ ����������� ��������� � ����������� ������� ����� �� ������������
  // (��� �������, ��� ��� ����������������)

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
    sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, MOXAGATE_MBTABLES_OVERLAPS, k, 0, 0, 0);
    close_shm();
    exit(2);
    }

  ///!!! ����� ��������. ���� ������� ����������� ���������� ������������

/// ������������� WATCH-DOG �������
if(Security.watchdog_timer==1) {
	MoxaDevice.mxwdt_handle = mxwdg_open(MOXAGATE_WATCHDOG_PERIOD*1000);
	if(MoxaDevice.mxwdt_handle<0) {
    sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, MOXAGATE_WATCHDOG_STARTED, 0, 0, 0, 0);
		exit (1);
	  }
  sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|GATEWAY_SYSTEM, MOXAGATE_WATCHDOG_STARTED, 0, 0, 0, 0);
	}

  // ���������� ��������� ������ ��� �������� ������ ��������
  init_moxagate_memory();
  ///!!! ����� ��������. �������� �������� ������� ���������� ����� ����� �������� ������ ������������ �����������

	//-------------------------------------------------------
#ifdef MOXA7GATE_KM400
/* ������������� ������ ��-400 */
  k=init_kltm_h();
  if(k!=0) {
  	printf("init_kltm_h: error code %d. exit\n", k);
    exit(1);
    }
#endif

/* ������������� ���������, �� ������� �������� ������� ������ */
	if(init_sem_set() != 0) exit(1);

/* ������������� TCP ����� �����, ������������ ���������� �� ���� ������, �� ����������� ������ GATEWAY_SIMPLE */
  if(init_main_socket() !=0 ) exit(1);

/* ������ ��������� �������, ������������� ���������������� ������ */

	operations[0].sem_op=1;
	operations[0].sem_flg=0;

	memset(ports, 0, sizeof(ports));

	for(i=0; i<MAX_MOXA_PORTS; i++) {
		
		P=0xff;

		if(P==0xff) for(j=0; j<MAX_MOXA_PORTS; j++)
		  if((IfaceRTU[j].modbus_mode==IFACE_TCPSERVER)&&(ports[j]==0)) {
		  	ports[j]=1; P=j; break;
		    }

		if(P==0xff) for(j=0; j<MAX_MOXA_PORTS; j++)
		  if((IfaceRTU[j].modbus_mode==IFACE_RTUMASTER)&&(ports[j]==0)) {
		  	ports[j]=1; P=j; break;
		    }

		if(P==0xff) for(j=0; j<MAX_MOXA_PORTS; j++)
		  if((IfaceRTU[j].modbus_mode==IFACE_RTUSLAVE)&&(ports[j]==0)) {
		  	ports[j]=1; P=j; break;
		    }

		if(P==0xff) continue;
		
		// opening serial ports
		if(IfaceRTU[P].modbus_mode!=IFACE_OFF) {
	    IfaceRTU[P].serial.fd = open_comm(IfaceRTU[P].serial.p_name, IfaceRTU[P].serial.p_mode);
	    
			IfaceRTU[P].serial.ch_interval_timeout =
			  set_param_comms(IfaceRTU[P].serial.fd,
			  	              IfaceRTU[P].serial.speed,
			  	              IfaceRTU[P].serial.parity,
			  	              IfaceRTU[P].serial.timeout);
		//	printf("speed %s\n", inputDATA->serial.speed);
		//	printf("parity %s\n", inputDATA->serial.parity);
	    
			// ����
	    //if(P==4) {IfaceRTU[P].modbus_mode=IFACE_ERROR; strcpy(IfaceRTU[P].bridge_status, "ERR"); continue;}
	    if(0) {IfaceRTU[P].modbus_mode=IFACE_OFF; strcpy(IfaceRTU[P].bridge_status, "ERR"); continue;} ///!!!

			// SERIAL PORT INITIALIZED
			sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|P, IFACE_TTYINIT, IfaceRTU[P].modbus_mode, 0, 0, 0);
			} else continue;

		switch(IfaceRTU[P].modbus_mode) {

			case IFACE_RTUMASTER:

#ifdef MOXA7GATE_KM400
        if(P==GATEWAY_P1 && kltm_port!=GATEWAY_P1) continue; // ���� GATEWAY_P1 ��������������
        // ������� ��� ������������� ������� � ������, ���������� �������� ���������� PLC
          else pthread_mutex_init(&IfaceRTU[P].serial_mutex, NULL);
#endif

				strcpy(IfaceRTU[P].bridge_status, "00P");
				init_queue(&IfaceRTU[P].queue, P);
			  arg=(P<<8)&0xff00;
				  IfaceRTU[P].rc = pthread_create(
					  &IfaceRTU[P].tid_srvr,
					  NULL,
					  iface_rtu_master,
					  (void *) arg);

				if (IfaceRTU[P].rc!=0){
					IfaceRTU[P].modbus_mode=IFACE_ERROR;
		      strcpy(IfaceRTU[P].bridge_status, "ERR");
		      // THREAD INITIALIZED
					sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|P, 41, IfaceRTU[P].rc, 0, 0, 0);
				  }

				operations[0].sem_num=P;
  			semop(semaphore_id, operations, 1);
				break;

#ifndef MOXA7GATE_WITHOUT_IFACE_RTUSLAVE
			case IFACE_RTUSLAVE:
				strcpy(IfaceRTU[P].bridge_status, "SLV");

				/// ���� ��������� ���� ��� modbus-rtu �������
			  j=get_current_client();

			  Client[j].status=GW_CLIENT_RTU_SLV;
			  Client[j].iface=P;
				time(&Client[j].connection_time);
				Client[j].disconnection_time=0;
				time(&Client[j].last_activity_time);
			  //clear_stat(&�lient[j].stat);
				sprintf(Client[j].device_name, "RTU SLAVE P%d", P+1);

				arg=(P<<8)|(j&0xff);
				IfaceRTU[P].rc = pthread_create(
					&IfaceRTU[P].tid_srvr,
					NULL,
					iface_rtu_slave,
					(void *) arg);
				break;
#endif

#ifndef MOXA7GATE_WITHOUT_IFACE_TCPSERVER
			case IFACE_TCPSERVER:

        // ������� ��� ������������� ������� � ����������������� ���������� �� ������� ���������� �������
				pthread_mutex_init(&IfaceRTU[P].serial_mutex, NULL);

	    	// SOCKET INITIALIZED
				IfaceRTU[P].ssd = socket(AF_INET, SOCK_STREAM, 0);
				if (IfaceRTU[P].ssd < 0) {
					perror("csdet");
					IfaceRTU[P].modbus_mode=IFACE_ERROR;
			    ///!!! ��������� ������������ ���� ������ � mxshm.c:99[refresh_shm()]
			    strcpy(IfaceRTU[P].bridge_status, "ERR"); 
					sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|P, TCPCON_INITIALIZED, 1, 0, 0, 0);
			    break;
					}
				
				addr.sin_family = AF_INET;
				addr.sin_port = htons(IfaceRTU[P].Security.tcp_port);
				addr.sin_addr.s_addr = htonl(INADDR_ANY);
				//����������� ����� 
				if (bind(IfaceRTU[P].ssd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
					perror("bind");
					//status
					//exit(1);
					close(IfaceRTU[P].ssd);
					IfaceRTU[P].ssd=-1;
					IfaceRTU[P].modbus_mode=IFACE_ERROR;
			    ///!!! ��������� ������������ ���� ������ � mxshm.c:99[refresh_shm()]
			    strcpy(IfaceRTU[P].bridge_status, "ERR");
					sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|P, TCPCON_INITIALIZED, 2, 0, 0, 0);
			    break;
					}

				//������ � �������
				listen(IfaceRTU[P].ssd, MAX_TCP_CLIENTS_PER_PORT);
				
				fcntl(IfaceRTU[P].ssd, F_SETFL, fcntl(IfaceRTU[P].ssd, F_GETFL, 0) | O_NONBLOCK);

				sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|P, TCPCON_INITIALIZED, 3, 0, 0, 0);
			    ///!!! ��������� ������������ ���� ������ � mxshm.c:99[refresh_shm()]
			  strcpy(IfaceRTU[P].bridge_status, "00G");
		
				break;
#endif

			default: IfaceRTU[P].modbus_mode=IFACE_OFF;
			}
		
/// ���������� ����� ������� �����
		time(&IfaceRTU[P].Security.start_time);
		}

/// ������ ��������� ������� ����������� � ������ IFACE_TCPMASTER

	/// ������������� ���������� �� Ethernet ����������� ����� �������������
	/// �������������� ������� TCP SERVER - ������
  for(i=0; i<Security.TCPIndex[MAX_TCP_SERVERS]; i++) {

	  T=Security.TCPIndex[i];
	  if(IfaceTCP[T].modbus_mode!=IFACE_TCPMASTER) continue;

#ifdef MOXA7GATE_KM400
        if((T+GATEWAY_T01)==GATEWAY_T01 && kltm_port!=GATEWAY_T01) continue; // ���� GATEWAY_T01 ��������������
        // ������� ��� ������������� ������� � ������, ���������� �������� ���������� PLC
          else pthread_mutex_init(&IfaceTCP[T].serial_mutex, NULL);
#endif

		strcpy(IfaceTCP[T].bridge_status, "INI");
		init_queue(&IfaceTCP[T].queue, GATEWAY_T01+T);
		
	  arg=((GATEWAY_T01+T)<<8)&0xff00;

		IfaceTCP[T].rc = pthread_create(
			&IfaceTCP[T].tid_srvr,
			NULL,
			iface_tcp_master,
			(void *) arg);

		operations[0].sem_num=GATEWAY_T01+T;
		semop(semaphore_id, operations, 1);
		}

/// ������ ������ ��� ��������� �������� ��������������� � MOXA

	rc = pthread_create(
		&moxaTH,
		NULL,
		moxa_device,
		NULL);
	operations[0].sem_num=IFACE_MOXAGATE;
	semop(semaphore_id, operations, 1);

#ifdef MOXA7GATE_KM400
/// ������ ������ ��� ��������� �������� �� ���� (M340) � ������� ������������ (���)

  /// ���� ��������� ���� ��� ������� KM-400
  kltm_client=get_current_client();

  Client[kltm_client].status=GW_CLIENT_KM400;
  time(&Client[kltm_client].connection_time);
  Client[kltm_client].disconnection_time=0;
  time(&Client[kltm_client].last_activity_time);
  sprintf(Client[j].device_name, "KM-400 MB P%d", 1); //!!! Modbus-master ��������� ������ ������������ �� ������ ������������� ����� ��������� ��������� ������

	Client[kltm_client].rc = pthread_create(
		&Client[kltm_client].tid_srvr,
		NULL,
		kltm_main,
		NULL);

#endif

/// ���������� ����� ������� �����
time(&MoxaDevice.start_time);

//----------   � � � �   �   � � � � � �   ------------

gateway_common_processing();

//----------------- ���������� ������ -----------------

	for(i=0; i<MAX_MOXA_PORTS; i++)
	  if(IfaceRTU[i].ssd>=0) {
			shutdown(IfaceRTU[i].ssd, SHUT_RDWR);
	  	close(IfaceRTU[i].ssd);
	  	}

  close_clients(); // ������� ���������� ������ CLIETNS_H

#ifndef MOXA7GATE_WITHOUT_HMI_KLB
  clear_hmi_klb_h(); // ������ ���������� ������ HMI_KEYPAD_LCM_H
#endif

	close_shm();
	semctl(semaphore_id, MAX_MOXA_PORTS, IPC_RMID, NULL);

	free(MoxaDevice.wData1x);
	free(MoxaDevice.wData2x);
	free(MoxaDevice.wData3x);
	free(MoxaDevice.wData4x);

  if(Security.watchdog_timer==1) mxwdg_close(MoxaDevice.mxwdt_handle);

	// PROGRAM TERMINATED
	time(&curtime);
	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|GATEWAY_SYSTEM, PROGRAM_TERMINATED, curtime-MoxaDevice.start_time, 0, 0, 0);

	return (0);
}
///-----------------------------------------------------------------------------------------------------------------
