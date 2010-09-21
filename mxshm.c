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

char *pointer;

input_cfg *shared_memory;
input_cfg_502 *gate;

RT_Table_Entry *t_rtm; //[MAX_VIRTUAL_SLAVES];
Query_Table_Entry *t_proxy; //[MAX_QUERY_ENTRIES];
GW_TCP_Server *t_tcpsrv; //[MAX_TCP_SERVERS];

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
	struct shmid_ds shmbuffer;

	access_key=ftok("/tmp/app", 'a');

	unsigned mem_size_ttl =
		sizeof(input_cfg_502)+
		sizeof(input_cfg)*MAX_MOXA_PORTS+
		sizeof(GW_EventLog)*EVENT_LOG_LENGTH+
		sizeof(RT_Table_Entry)*MAX_VIRTUAL_SLAVES+
		sizeof(Query_Table_Entry)*MAX_QUERY_ENTRIES+
		sizeof(GW_TCP_Server)*MAX_TCP_SERVERS;

  shm_segment_id=shmget(access_key, mem_size_ttl, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

	if(shm_segment_id==-1) {

		switch(errno) {
			case ENOENT: printf("shmget error: ENOENT\n"); break;
			case EACCES: printf("shmget error: EACCES\n"); break;
			case EINVAL: printf("shmget error: EINVAL\n"); break;
			case ENOMEM: printf("shmget error: ENOMEM\n"); break;
			case EEXIST: printf("shmget error: EEXIST\n"); break;
			default: printf("shmget error: unknown %d\n", errno);
			}

		return shm_segment_id;
	  }
	printf("shmget OK.");

  ///--- permission mode-------
	struct shmid_ds mds;
	mds.shm_perm.mode=438;
	shmctl(shm_segment_id, IPC_SET, &mds);

	shmctl(shm_segment_id, IPC_STAT, &shmbuffer);
	printf(" segment size: %d\n", shmbuffer.shm_segsz);

	pointer=shmat(shm_segment_id, 0, 0);

	gate=(input_cfg_502 *) pointer;
	shared_memory=(input_cfg *)(pointer+sizeof(input_cfg_502));
	app_log=(GW_EventLog *)(pointer+sizeof(input_cfg_502)+sizeof(input_cfg)*MAX_MOXA_PORTS);

	t_rtm=(RT_Table_Entry *)(pointer+sizeof(input_cfg_502)+sizeof(input_cfg)*MAX_MOXA_PORTS+
													 sizeof(GW_EventLog)*EVENT_LOG_LENGTH);
	t_proxy=(Query_Table_Entry *)(pointer+sizeof(input_cfg_502)+sizeof(input_cfg)*MAX_MOXA_PORTS+
													 sizeof(GW_EventLog)*EVENT_LOG_LENGTH+sizeof(RT_Table_Entry)*MAX_VIRTUAL_SLAVES);
	t_tcpsrv=(GW_TCP_Server *)(pointer+sizeof(input_cfg_502)+sizeof(input_cfg)*MAX_MOXA_PORTS+
														sizeof(GW_EventLog)*EVENT_LOG_LENGTH+sizeof(RT_Table_Entry)*MAX_VIRTUAL_SLAVES+
														sizeof(Query_Table_Entry)*MAX_QUERY_ENTRIES);

///  printf("%d %d %d\n", shm_data, shared_memory, app_log);
///  printf("%p %p %p\n", shm_data, shared_memory, app_log);

  return 0;
  }

///--------------------------------------------------------------------------
int refresh_shm(void *arg)
	{
  input_cfg *iDATA=(input_cfg *) arg;
  
  int i, j, k;
  int stat1, stat2;

  for(i=0; i<MAX_MOXA_PORTS; i++) {

//		iDATA[i].stat.request_time_average=0;
//    for(j=0; j<MAX_TCP_CLIENTS_PER_PORT; j++)
//  	  if(iDATA[i].clients[j].stat.request_time_average!=0)
//  	    if(iDATA[i].stat.request_time_average<iDATA[i].clients[j].stat.request_time_average)
//  	      iDATA[i].stat.request_time_average=iDATA[i].clients[j].stat.request_time_average;
  	      
  	if(iDATA[i].modbus_mode!=MODBUS_PORT_ERROR && iDATA[i].modbus_mode!=MODBUS_PORT_OFF) {
	    sprintf(iDATA[i].bridge_status, "***");
			switch(iDATA[i].modbus_mode) {

				case GATEWAY_SIMPLE:
					k=iDATA[i].current_connections_number;
			    sprintf(iDATA[i].bridge_status, "%2.2dG", k);
					break;

				case GATEWAY_ATM:
					k=iDATA[i].queue.queue_len*100/MAX_GATEWAY_QUEUE_LENGTH;
					if(k<100) sprintf(iDATA[i].bridge_status, "%2.2dA", k);
						else sprintf(iDATA[i].bridge_status, "OVR"); /// отладочный код
					break;

				case GATEWAY_RTM:
					k=iDATA[i].queue.queue_len*100/MAX_GATEWAY_QUEUE_LENGTH;
					if(k<100) sprintf(iDATA[i].bridge_status, "%2.2dR", k);
						else sprintf(iDATA[i].bridge_status, "OVR"); /// отладочный код
					break;

				case GATEWAY_PROXY:
					/// количество успешно выполняемых запросов из таблицы QUERY_TABLE для этого порта (00-99%)
				  stat1=stat2=0;
				  for(j=0; j<MAX_QUERY_ENTRIES; j++)
						if(query_table[j].port==i) {
							stat1++;
							if(query_table[j].status_bit==1) stat2++;
							}
					k=stat1==stat2?99:stat2*100/stat1;
			    sprintf(iDATA[i].bridge_status, "%2.2dP", k);
					break;

				case BRIDGE_PROXY:
					//k=iDATA[i].current_connections_number;
			    sprintf(iDATA[i].bridge_status, "BPR");
					break;

				case BRIDGE_SIMPLE:
					k=iDATA[i].current_connections_number;
					j=iDATA[i].accepted_connections_number;
			    sprintf(iDATA[i].bridge_status, "%1.1d%1.1dB", k, j);
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

	  strcpy(shared_memory[i].description, iDATA[i].description);

	    for(j=0; j<MAX_TCP_CLIENTS_PER_PORT; j++) {
			  shared_memory[i].clients[j].connection_time=iDATA[i].clients[j].connection_time;
			  shared_memory[i].clients[j].ip=iDATA[i].clients[j].ip;
			  shared_memory[i].clients[j].port=iDATA[i].clients[j].port;
			  shared_memory[i].clients[j].rc=iDATA[i].clients[j].rc;
			  shared_memory[i].clients[j].connection_status=iDATA[i].clients[j].connection_status;
			  shared_memory[i].clients[j].mb_slave=iDATA[i].clients[j].mb_slave;
			  shared_memory[i].clients[j].address_shift=iDATA[i].clients[j].address_shift;
			
				copy_stat(&shared_memory[i].clients[j].stat, &iDATA[i].clients[j].stat);
		    }

		copy_stat(&shared_memory[i].stat, &iDATA[i].stat);
	  }
		 
	gate->app_log_current_entry=gate502.app_log_current_entry;
	gate->app_log_entries_total=gate502.app_log_entries_total;
	strcpy(gate->object, gate502.object);
	strcpy(gate->location, gate502.location);
	strcpy(gate->version, gate502.version);
	strcpy(gate->networkName, gate502.networkName);
	gate->IPAddress=gate502.IPAddress;
	gate->start_time=gate502.start_time;

	gate->tcp_port=gate502.tcp_port;
	gate->modbus_address=gate502.modbus_address;
	gate->status_info=gate502.status_info;

	gate->show_data_flow=gate502.show_data_flow;
	gate->show_sys_messages=gate502.show_sys_messages;
	gate->watchdog_timer=gate502.watchdog_timer;
//	gate->use_buzzer=gate502.use_buzzer;
//	gate->back_light=gate502.back_light;

	gate->offset1xStatus=gate502.offset1xStatus;
	gate->offset2xStatus=gate502.offset2xStatus;
	gate->offset3xRegisters=gate502.offset3xRegisters;
	gate->offset4xRegisters=gate502.offset4xRegisters;
	gate->amount1xStatus=gate502.amount1xStatus;
	gate->amount2xStatus=gate502.amount2xStatus;
	gate->amount3xRegisters=gate502.amount3xRegisters;
	gate->amount4xRegisters=gate502.amount4xRegisters;
  ///--------------------------------------------------
	time(&gate->timestamp);
  ///--------------------------------------------------

	if(gate->halt==1) {
		gate502.halt=1;
		show_confirmation_reboot();
		mxlcm_write(mxlcm_handle, 0, 2, "                ", MAX_LCM_COLS); // 16 characters
		mxlcm_write(mxlcm_handle, 0, 3, "                ", MAX_LCM_COLS); // 16 characters
		mxlcm_write(mxlcm_handle, 0, 4, " program stopped", MAX_LCM_COLS); // 16 characters
		mxlcm_write(mxlcm_handle, 0, 5, "by web-interface", MAX_LCM_COLS); // 16 characters
		mxlcm_write(mxlcm_handle, 0, 6, "                ", MAX_LCM_COLS); // 16 characters
		}

	if(gate->back_light!=gate502.back_light) {
		gate502.back_light = screen.back_light = gate->back_light;
	  if(screen.back_light) {	mxlcm_control(mxlcm_handle, IOCTL_LCM_BACK_LIGHT_ON);
	    } else { 							mxlcm_control(mxlcm_handle, IOCTL_LCM_BACK_LIGHT_OFF);}
		}

	if(gate->use_buzzer!=gate502.use_buzzer) {
//		gate502.use_buzzer = screen.buzzer_control = gate->use_buzzer;
		gate502.use_buzzer = gate->use_buzzer;
		mxbuzzer_beep(mxbzr_handle, 400);
		}

  ///--------------------------------------------------

	for(i=0; i<MAX_VIRTUAL_SLAVES; i++) {
		t_rtm[i].start=vslave[i].start;
		t_rtm[i].length=vslave[i].length;
		t_rtm[i].port=vslave[i].port;
		t_rtm[i].device=vslave[i].device;
		t_rtm[i].modbus_table=vslave[i].modbus_table;
		t_rtm[i].address_shift=vslave[i].address_shift;
		strcpy(t_rtm[i].device_name, vslave[i].device_name);
	  }
	
	for(i=0; i<MAX_QUERY_ENTRIES; i++) {
		t_proxy[i].start=query_table[i].start;
		t_proxy[i].length=query_table[i].length;
		t_proxy[i].offset=query_table[i].offset;
		t_proxy[i].port=query_table[i].port;
		t_proxy[i].device=query_table[i].device;
		t_proxy[i].mbf=query_table[i].mbf;
		t_proxy[i].delay=query_table[i].delay;
		t_proxy[i].critical=query_table[i].critical;
		t_proxy[i].err_counter=query_table[i].err_counter;
		t_proxy[i].status_bit=query_table[i].status_bit;
		strcpy(t_proxy[i].device_name, query_table[i].device_name);
	  }
	
	for(i=0; i<MAX_TCP_SERVERS; i++) {
		t_tcpsrv[i].mb_slave=tcp_servers[i].mb_slave;
		t_tcpsrv[i].ip=tcp_servers[i].ip;
		t_tcpsrv[i].port=tcp_servers[i].port;
		t_tcpsrv[i].address_shift=tcp_servers[i].address_shift;
		t_tcpsrv[i].p_num=tcp_servers[i].p_num;
		strcpy(t_tcpsrv[i].device_name, tcp_servers[i].device_name);
	  }

  ///*** Заполняем массив диагностической информации, если он был инициализирован

  for(i=0; i<MAX_MOXA_PORTS; i++) {
    gate502.wData4x[gate502.status_info+3*i+0]=iDATA[i].stat.accepted;
    gate502.wData4x[gate502.status_info+3*i+1]=iDATA[i].stat.sended;
    gate502.wData4x[gate502.status_info+3*i+2]=iDATA[i].stat.request_time_average;
	  }

  for(i=0; i<MAX_QUERY_ENTRIES; i++) {
		j = 0x01 << (i % 16);
    if(query_table[i].status_bit==0)
			gate502.wData4x[gate502.status_info+3*MAX_MOXA_PORTS+(i/16)]&=~j;
			else
			gate502.wData4x[gate502.status_info+3*MAX_MOXA_PORTS+(i/16)]|=j;
    }

	return 0;
	}

///--------------------------------------------------------------------------
int close_shm()
	{
	shmdt(pointer);
	shmctl(shm_segment_id, IPC_RMID, 0);
	printf("shmget CLOSED\n");
	return 0;
	}
///--------------------------------------------------------------------------
