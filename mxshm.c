/*
MOXA7GATE MODBUS GATEWAY SOFTWARE
SEM-ENGINEERING
                    BRYANSK 2009
*/

#include "global.h"

#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <errno.h>

input_cfg *shared_memory;
SHM_Data *shm_data;
key_t access_key;
struct shmid_ds shmbuffer;

/*//---for reference purposes only!-----------------------------------------------------
struct shmid_ds {
	struct	ipc_perm	shm_perm;		//operation permission struct // operation perms /
	int				shm_segsz;			// size of segment in bytes / 	/ size of segment (bytes) /
	time_t			shm_atime;			// time of last shmat() / 	/ last attach time /
	time_t			shm_dtime;			// time of last shmdt() / 	/ last detach time /
	time_t			shm_ctime;			// time of last change by shmctl() /		/ last change time /
	unsigned short	shm_cpid;			// pid of creator / 	/ pid of creator /
	unsigned short	shm_lpid;			// pid of last shmop /		/ pid of last operator /
	short			shm_nattch;		// number of current attaches /	/ no. of current attaches /
}; 

*///--------------------------------------------------------------------------
int init_shm()
  {
	access_key=ftok("/tmp/app", 'a');

  shm_segment_id=shmget(access_key, sizeof(SHM_Data)+sizeof(input_cfg)*MAX_MOXA_PORTS,
  																					IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

	if(shm_segment_id==-1) {

		switch(errno) {
			case ENOENT: printf("shmget error: ENOENT\n"); break;
			case EACCES: printf("shmget error: EACCES\n"); break;
			case EINVAL: printf("shmget error: EINVAL\n"); break;
			case ENOMEM: printf("shmget error: ENOMEM\n"); break;
			default: printf("shmget error: unknown\n");
			}

		return shm_segment_id;
	  }
	printf("shmget OK\n");

  ///--- permission mode-------
	struct shmid_ds mds;
	mds.shm_perm.mode=438;
	shmctl(shm_segment_id, IPC_SET, &mds);

	shm_data=(SHM_Data *) shmat(shm_segment_id, 0, 0);
	shared_memory=(input_cfg *)(shm_data+sizeof(SHM_Data));

  return 0;
  }

///--------------------------------------------------------------------------
int refresh_shm(void *arg)
	{
  input_cfg *iDATA=(input_cfg *) arg;
  
  int i, j, k;
  for(i=0; i<MAX_MOXA_PORTS; i++) {

//		iDATA[i].stat.request_time_average=0;
//    for(j=0; j<MAX_TCP_CLIENTS_PER_PORT; j++)
//  	  if(iDATA[i].clients[j].stat.request_time_average!=0)
//  	    if(iDATA[i].stat.request_time_average<iDATA[i].clients[j].stat.request_time_average)
//  	      iDATA[i].stat.request_time_average=iDATA[i].clients[j].stat.request_time_average;
  	      
  	if(iDATA[i].modbus_mode!=MODBUS_PORT_ERROR && iDATA[i].modbus_mode!=MODBUS_PORT_OFF) {
	    sprintf(iDATA[i].bridge_status, "***");
			switch(iDATA[i].modbus_mode) {
				case MODBUS_GATEWAY_MODE:
					k=iDATA[i].current_connections_number;
			    sprintf(iDATA[i].bridge_status, "%2.2dG", k);
					if(_single_gateway_port_502==1) {
						k=iDATA[i].queue_len*100/MAX_GATEWAY_QUEUE_LENGTH;
						if(k<100) sprintf(iDATA[i].bridge_status, "%2.2dA", k);
							else sprintf(iDATA[i].bridge_status, "ERR");
						}
					if(_single_address_space==1) {
						k=iDATA[i].queue_len*100/MAX_GATEWAY_QUEUE_LENGTH;
						if(k<100) sprintf(iDATA[i].bridge_status, "%2.2dR", k);
							else sprintf(iDATA[i].bridge_status, "ERR");
						}
					break;
				case MODBUS_BRIDGE_MODE:
					k=iDATA[i].current_connections_number;
			    sprintf(iDATA[i].bridge_status, "%2.2dB", k);
					break;
				default:;
				}
			}

    }
    
	gettimeofday(&tv, &tz);
	int sec=(tv.tv_sec-tv_mem.tv_sec)+(tv.tv_usec-tv_mem.tv_usec)/1000000;
	if(sec>=LCM_BUZZER_CONTROL_PERIOD) {
//		printf("buzzer:%d; sec:%d; difference:%d", screen.buzzer_control, sec, iDATA[i].stat.errors-p_errors[i]);
		tv_mem.tv_sec=tv.tv_sec;
		tv_mem.tv_usec=tv.tv_usec;
		for(i=0; i<MAX_MOXA_PORTS; i++) {
		  if(iDATA[i].stat.errors-p_errors[i] >= LCM_BUZZER_CONTROL_ERRORS)
//		    if(screen.buzzer_control==1) mxbuzzer_beep(mxbzr_handle, 400);
		  p_errors[i]=iDATA[i].stat.errors;
		  }
	  }
  
	//shmctl(shm_segment_id, IPC_STAT, &shmbuffer);
	//unsigned segment_size=shmbuffer.shm_segsz;
	//printf("segment size: %d, iDATA size: %d\n", segment_size, sizeof(input_cfg)*MAX_MOXA_PORTS);
	if(shm_segment_id==-1) return 1;

  ///--------------------------------------------------
  for(i=0; i<MAX_MOXA_PORTS; i++) {
	  shared_memory[i].tcp_port=iDATA[i].tcp_port;
	  shared_memory[i].modbus_mode=iDATA[i].modbus_mode;
	  strcpy(shared_memory[i].bridge_status, iDATA[i].bridge_status);

	  shared_memory[i].accepted_connections_number=iDATA[i].accepted_connections_number;
	  shared_memory[i].current_connections_number=iDATA[i].current_connections_number;
	  shared_memory[i].rejected_connections_number=iDATA[i].rejected_connections_number;

		shared_memory[i].start_time=iDATA[i].start_time;

	  shared_memory[i].serial.p_num=iDATA[i].serial.p_num;
	  strcpy(shared_memory[i].serial.p_name, iDATA[i].serial.p_name);
	  strcpy(shared_memory[i].serial.p_mode, iDATA[i].serial.p_mode);
	  strcpy(shared_memory[i].serial.speed, iDATA[i].serial.speed);
	  strcpy(shared_memory[i].serial.parity, iDATA[i].serial.parity);
	  shared_memory[i].serial.timeout=iDATA[i].serial.timeout;

	    for(j=0; j<MAX_TCP_CLIENTS_PER_PORT; j++) {
			  shared_memory[i].clients[j].connection_time=iDATA[i].clients[j].connection_time;
			  shared_memory[i].clients[j].ip=iDATA[i].clients[j].ip;
			  shared_memory[i].clients[j].port=iDATA[i].clients[j].port;
			  shared_memory[i].clients[j].rc=iDATA[i].clients[j].rc;
			  shared_memory[i].clients[j].connection_status=iDATA[i].clients[j].connection_status;
			  shared_memory[i].clients[j].mb_slave=iDATA[i].clients[j].mb_slave;
			  shared_memory[i].clients[j].address_shift=iDATA[i].clients[j].address_shift;
			
			  shared_memory[i].clients[j].stat.accepted=iDATA[i].clients[j].stat.accepted;
			  shared_memory[i].clients[j].stat.errors_input_communication= iDATA[i].clients[j].stat.errors_input_communication;
			  shared_memory[i].clients[j].stat.errors_tcp_adu=iDATA[i].clients[j].stat.errors_tcp_adu;
			  shared_memory[i].clients[j].stat.errors_tcp_pdu=iDATA[i].clients[j].stat.errors_tcp_pdu;
			  shared_memory[i].clients[j].stat.errors_serial_sending=iDATA[i].clients[j].stat.errors_serial_sending;
			  shared_memory[i].clients[j].stat.errors_serial_accepting=iDATA[i].clients[j].stat.errors_serial_accepting;
			  shared_memory[i].clients[j].stat.timeouts=iDATA[i].clients[j].stat.timeouts;
			  shared_memory[i].clients[j].stat.crc_errors=iDATA[i].clients[j].stat.crc_errors;
			  shared_memory[i].clients[j].stat.errors_serial_adu=iDATA[i].clients[j].stat.errors_serial_adu;
			  shared_memory[i].clients[j].stat.errors_serial_pdu=iDATA[i].clients[j].stat.errors_serial_pdu;
			  shared_memory[i].clients[j].stat.errors_tcp_sending=iDATA[i].clients[j].stat.errors_tcp_sending;
			  shared_memory[i].clients[j].stat.errors=iDATA[i].clients[j].stat.errors;
				//if((i==SERIAL_P2)&&(j==0)) printf("shm_seral_p2_sended:%d", iDATA[i].clients[j].stat.sended);
			  shared_memory[i].clients[j].stat.sended=iDATA[i].clients[j].stat.sended;
			  shared_memory[i].clients[j].stat.request_time_min=iDATA[i].clients[j].stat.request_time_min;
			  shared_memory[i].clients[j].stat.request_time_max=iDATA[i].clients[j].stat.request_time_max;
			  shared_memory[i].clients[j].stat.request_time_average=iDATA[i].clients[j].stat.request_time_average;
			  shared_memory[i].clients[j].stat.scan_rate=iDATA[i].clients[j].stat.scan_rate;
				//unsigned int input_messages [MB_FUNCTIONS_IMPLEMENTED*2+1];
				//unsigned int output_messages[MB_FUNCTIONS_IMPLEMENTED*2+1];
		    }

	  shared_memory[i].stat.accepted=iDATA[i].stat.accepted;
	  shared_memory[i].stat.errors_input_communication=iDATA[i].stat.errors_input_communication;
	  shared_memory[i].stat.errors_tcp_adu=iDATA[i].stat.errors_tcp_adu;
	  shared_memory[i].stat.errors_tcp_pdu=iDATA[i].stat.errors_tcp_pdu;
	  shared_memory[i].stat.errors_serial_sending=iDATA[i].stat.errors_serial_sending;
	  shared_memory[i].stat.errors_serial_accepting=iDATA[i].stat.errors_serial_accepting;
	  shared_memory[i].stat.timeouts=iDATA[i].stat.timeouts;
	  shared_memory[i].stat.crc_errors=iDATA[i].stat.crc_errors;
	  shared_memory[i].stat.errors_serial_adu=iDATA[i].stat.errors_serial_adu;
	  shared_memory[i].stat.errors_serial_pdu=iDATA[i].stat.errors_serial_pdu;
	  shared_memory[i].stat.errors_tcp_sending=iDATA[i].stat.errors_tcp_sending;
	  shared_memory[i].stat.errors=iDATA[i].stat.errors;
	  shared_memory[i].stat.sended=iDATA[i].stat.sended;
	  shared_memory[i].stat.request_time_min=iDATA[i].stat.request_time_min;
	  shared_memory[i].stat.request_time_max=iDATA[i].stat.request_time_max;
	  shared_memory[i].stat.request_time_average=iDATA[i].stat.request_time_average;
	  shared_memory[i].stat.scan_rate=iDATA[i].stat.scan_rate;
		//unsigned int input_messages [MB_FUNCTIONS_IMPLEMENTED*2+1];
		//unsigned int output_messages[MB_FUNCTIONS_IMPLEMENTED*2+1];
	  }
		 
	time(&shm_data->timestamp);
	shm_data->ATM=_single_gateway_port_502;
	shm_data->RTM=_single_address_space;
	shm_data->PROXY=_proxy_mode;
  ///--------------------------------------------------

	return 0;
	}

///--------------------------------------------------------------------------
int close_shm()
	{
	shmdt(shared_memory);
	shmctl(shm_segment_id, IPC_RMID, 0);
	printf("shmget CLOSED\n");
	return 0;
	}
