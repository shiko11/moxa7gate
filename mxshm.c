#include "global.h"

#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <errno.h>

input_cfg *shared_memory;
time_t *timestamp;
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

  shm_segment_id=shmget(access_key, sizeof(time_t)+sizeof(input_cfg)*MAX_MOXA_PORTS,
  																					IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

	if(shm_segment_id==-1) {
		printf("error %d. segment exists?\n", errno);
		return shm_segment_id;
	  }
	timestamp=(time_t *) shmat(shm_segment_id, 0, 0);
	shared_memory=(input_cfg *)(timestamp+sizeof(time_t));

  return 0;
  }

///--------------------------------------------------------------------------
int refresh_shm(void *arg)
	{
  input_cfg *iDATA=(input_cfg *) arg;
  
  int i, j;
  for(i=0; i<MAX_MOXA_PORTS; i++) {

		iDATA[i].stat.request_time_average=0;
    for(j=0; j<MAX_TCP_CLIENTS_PER_PORT; j++)
  	  if(iDATA[i].clients[j].stat.request_time_average!=0)
  	    if(iDATA[i].stat.request_time_average<iDATA[i].clients[j].stat.request_time_average)
  	      iDATA[i].stat.request_time_average=iDATA[i].clients[j].stat.request_time_average;
  	      
//  	if(strcmp(iDATA[i].bridge_status, "ERR")!=0 && strcmp(iDATA[i].bridge_status, "OFF")!=0)
  	if(iDATA[i].modbus_mode!=MODBUS_PORT_ERROR && iDATA[i].modbus_mode!=MODBUS_PORT_OFF)
	    sprintf(iDATA[i].bridge_status, "%2.2d%c",
	    				iDATA[i].current_connections_number,
	    				iDATA[i].bridge_status[2]);
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
		 
	time(timestamp);
  ///--------------------------------------------------

	return 0;
	}

///--------------------------------------------------------------------------
int close_shm()
	{
	shmdt(shared_memory);
	shmctl(shm_segment_id, IPC_RMID, 0);
	return 0;
	}
