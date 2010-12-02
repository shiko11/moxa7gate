/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///** МОДУЛЬ МОНИТОРИНГА И УПРАВЛЕНИЯ РАБОТОЙ ШЛЮЗА СРЕДСТВАМИ WEB-ИНТЕРФЕЙСА **

///=== HMI_WEB_H IMPLEMENTATION

#include <sys/shm.h>
#include <sys/stat.h>
#include <errno.h>

#include "hmi_web.h"
#include "messages.h"
#include "interfaces.h"
#include "moxagate.h"
#include "hmi_keypad_lcm.h"

///=== HMI_WEB_H private variables

char *pointer;

GW_Iface *shared_memory;
GW_MoxaDevice *gate;

RT_Table_Entry *t_rtm; //[MAX_VIRTUAL_SLAVES];
Query_Table_Entry *t_proxy; //[MAX_QUERY_ENTRIES];
GW_Security *t_security;

key_t access_key;
struct shmid_ds shmbuffer;

//unsigned int buzzer_flag; // зуммер дает 1, 2 и 3 гудка в зависимости от количества ошибок: <15, 15-30, >30
//struct timeval tv;

///=== HMI_WEB_H private functions

int refresh_shm(void *arg);

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
		sizeof(GW_MoxaDevice)+
		sizeof(GW_Iface)*MAX_MOXA_PORTS+
		sizeof(GW_EventLog)*EVENT_LOG_LENGTH+
		sizeof(RT_Table_Entry)*MAX_VIRTUAL_SLAVES+
		sizeof(Query_Table_Entry)*MAX_QUERY_ENTRIES+
		sizeof(GW_Security);

  shm_segment_id=shmget(access_key, mem_size_ttl, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

	if(shm_segment_id==-1) {

		switch(errno) {
			case ENOENT: sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, 34, 0, 0, 0, 0); break;
			case EACCES: sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, 31, 0, 0, 0, 0); break;
			case EINVAL: sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, 33, 0, 0, 0, 0); break;
			case ENOMEM: sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, 35, 0, 0, 0, 0); break;
			case EEXIST: sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, 32, 0, 0, 0, 0); break;
			default: sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, 36, 0, 0, 0, 0); break;
			}

		return shm_segment_id;
	  }

  ///--- permission mode-------
	struct shmid_ds mds;
	mds.shm_perm.mode=438;
	shmctl(shm_segment_id, IPC_SET, &mds);

	shmctl(shm_segment_id, IPC_STAT, &shmbuffer);

	pointer=shmat(shm_segment_id, 0, 0);

	gate=(GW_MoxaDevice *) pointer;
	shared_memory=(GW_Iface *)(pointer+sizeof(GW_MoxaDevice));
	app_log=(GW_EventLog *)(pointer+sizeof(GW_MoxaDevice)+sizeof(GW_Iface)*MAX_MOXA_PORTS);

	t_rtm=(RT_Table_Entry *)(pointer+sizeof(GW_MoxaDevice)+sizeof(GW_Iface)*MAX_MOXA_PORTS+
													 sizeof(GW_EventLog)*EVENT_LOG_LENGTH);
	t_proxy=(Query_Table_Entry *)(pointer+sizeof(GW_MoxaDevice)+sizeof(GW_Iface)*MAX_MOXA_PORTS+
													 sizeof(GW_EventLog)*EVENT_LOG_LENGTH+sizeof(RT_Table_Entry)*MAX_VIRTUAL_SLAVES);
	t_security=(GW_Security *)(pointer+sizeof(GW_MoxaDevice)+sizeof(GW_Iface)*MAX_MOXA_PORTS+
														sizeof(GW_EventLog)*EVENT_LOG_LENGTH+sizeof(RT_Table_Entry)*MAX_VIRTUAL_SLAVES+
														sizeof(Query_Table_Entry)*MAX_QUERY_ENTRIES);

///  printf("%d %d %d\n", shm_data, shared_memory, app_log);
///  printf("%p %p %p\n", shm_data, shared_memory, app_log);
	// SHARED MEM: OK
	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|GATEWAY_SYSTEM, 37, shmbuffer.shm_segsz, 0, 0, 0);

  return 0;
  }

///--------------------------------------------------------------------------
int refresh_shm(void *arg)
	{
  GW_Iface *IfaceRTU=(GW_Iface *) arg;
  
  int i, j, k;
  int stat1, stat2;

  for(i=0; i<MAX_MOXA_PORTS; i++) {

//		IfaceRTU[i].stat.request_time_average=0;
//    for(j=0; j<MAX_TCP_CLIENTS_PER_PORT; j++)
//  	  if(IfaceRTU[i].clients[j].stat.request_time_average!=0)
//  	    if(IfaceRTU[i].stat.request_time_average<IfaceRTU[i].clients[j].stat.request_time_average)
//  	      IfaceRTU[i].stat.request_time_average=IfaceRTU[i].clients[j].stat.request_time_average;
  	      
  	if(IfaceRTU[i].modbus_mode!=MODBUS_PORT_ERROR && IfaceRTU[i].modbus_mode!=MODBUS_PORT_OFF) {
	    sprintf(IfaceRTU[i].bridge_status, "***");
			switch(IfaceRTU[i].modbus_mode) {

				case GATEWAY_SIMPLE:
					k=IfaceRTU[i].Security.current_connections_number;
			    sprintf(IfaceRTU[i].bridge_status, "%2.2dG", k);
					break;

				case GATEWAY_ATM:
					k=IfaceRTU[i].queue.queue_len*100/MAX_GATEWAY_QUEUE_LENGTH;
					if(k<100) sprintf(IfaceRTU[i].bridge_status, "%2.2dA", k);
						else sprintf(IfaceRTU[i].bridge_status, "OVR"); /// отладочный код
					break;

				case GATEWAY_RTM:
					k=IfaceRTU[i].queue.queue_len*100/MAX_GATEWAY_QUEUE_LENGTH;
					if(k<100) sprintf(IfaceRTU[i].bridge_status, "%2.2dR", k);
						else sprintf(IfaceRTU[i].bridge_status, "OVR"); /// отладочный код
					break;

				case GATEWAY_PROXY:
					/// количество успешно выполняемых запросов из таблицы QUERY_TABLE для этого порта (00-99%)
				  stat1=stat2=0;
				  for(j=0; j<MAX_QUERY_ENTRIES; j++)
						if(	(query_table[j].iface==i) 		&&
								(query_table[j].length!=0) 	&&
								(query_table[j].mbf!=0)			&&
								(query_table[j].device!=0)	) {

							stat1++;
							if(query_table[j].status_bit==1) stat2++;
							}
					k=(stat1==stat2)?99:((stat2*100)/stat1);
//					printf("stat1=%d; stat2=%d;\n", stat1, stat2);
			    sprintf(IfaceRTU[i].bridge_status, "%2.2dP", k);
					break;

				case BRIDGE_PROXY:
					//k=IfaceRTU[i].current_connections_number;
			    // sprintf(IfaceRTU[i].bridge_status, "BPR");
					break;

/*				case BRIDGE_SIMPLE:
					k=IfaceRTU[i].current_connections_number;
					j=IfaceRTU[i].accepted_connections_number;
			    sprintf(IfaceRTU[i].bridge_status, "%1.1d%1.1dB", k, j);
					break;*/
				default:;
				}
			}

    }

/* ЗВУК ЗУММЕРА ПРИ ОШИБКАХ ОБМЕНА
	gettimeofday(&tv, &tz);
	int sec=(tv.tv_sec-tv_mem.tv_sec)+(tv.tv_usec-tv_mem.tv_usec)/1000000;
	if(sec>=LCM_BUZZER_CONTROL_PERIOD) {
//		printf("buzzer:%d; sec:%d; difference:%d", screen.buzzer_control, sec, IfaceRTU[i].stat.errors-p_errors[i]);
		tv_mem.tv_sec=tv.tv_sec;
		tv_mem.tv_usec=tv.tv_usec;
		for(i=0; i<MAX_MOXA_PORTS; i++) {
		  if(IfaceRTU[i].stat.errors-p_errors[i] >= LCM_BUZZER_CONTROL_ERRORS)
//		    if(screen.buzzer_control==1) mxbuzzer_beep(mxbzr_handle, 400);
		  p_errors[i]=IfaceRTU[i].stat.errors;
		  }
	  } */
  
	//shmctl(shm_segment_id, IPC_STAT, &shmbuffer);
	//unsigned segment_size=shmbuffer.shm_segsz;
	//printf("segment size: %d, IfaceRTU size: %d\n", segment_size, sizeof(input_cfg)*MAX_MOXA_PORTS);
	if(shm_segment_id==-1) return 1;

  ///--------------------------------------------------
  for(i=0; i<MAX_MOXA_PORTS; i++) {
	  shared_memory[i].Security.tcp_port=IfaceRTU[i].Security.tcp_port;
	  shared_memory[i].modbus_mode=IfaceRTU[i].modbus_mode;
	  strcpy(shared_memory[i].bridge_status, IfaceRTU[i].bridge_status);

	  shared_memory[i].Security.accepted_connections_number=IfaceRTU[i].Security.accepted_connections_number;
	  shared_memory[i].Security.current_connections_number=IfaceRTU[i].Security.current_connections_number;
	  shared_memory[i].Security.rejected_connections_number=IfaceRTU[i].Security.rejected_connections_number;

		shared_memory[i].Security.start_time=IfaceRTU[i].Security.start_time;

	  strcpy(shared_memory[i].serial.p_name, IfaceRTU[i].serial.p_name);
	  strcpy(shared_memory[i].serial.p_mode, IfaceRTU[i].serial.p_mode);
	  strcpy(shared_memory[i].serial.speed, IfaceRTU[i].serial.speed);
	  strcpy(shared_memory[i].serial.parity, IfaceRTU[i].serial.parity);
	  shared_memory[i].serial.timeout=IfaceRTU[i].serial.timeout;

	  strcpy(shared_memory[i].description, IfaceRTU[i].description);

/*
	    for(j=0; j<MAX_TCP_CLIENTS_PER_PORT; j++) {
			  shared_memory[i].clients[j].connection_time=IfaceRTU[i].clients[j].connection_time;
			  shared_memory[i].clients[j].ip=IfaceRTU[i].clients[j].ip;
			  shared_memory[i].clients[j].port=IfaceRTU[i].clients[j].port;
			  shared_memory[i].clients[j].rc=IfaceRTU[i].clients[j].rc;
			  shared_memory[i].clients[j].status=IfaceRTU[i].clients[j].status;
			  shared_memory[i].clients[j].mb_slave=IfaceRTU[i].clients[j].mb_slave;
			  shared_memory[i].clients[j].address_shift=IfaceRTU[i].clients[j].address_shift;
			
				copy_stat(&shared_memory[i].clients[j].stat, &IfaceRTU[i].clients[j].stat);
		    }
	*/
		copy_stat(&shared_memory[i].stat, &IfaceRTU[i].stat);
	  }
		 
	app_log_current_entry=app_log_current_entry;
	app_log_entries_total=app_log_entries_total;

	strcpy(t_security->Object, Security.Object);
	strcpy(t_security->Location, Security.Location);
	strcpy(t_security->VersionNumber, Security.VersionNumber);
	strcpy(t_security->NetworkName, Security.NetworkName);
	t_security->LAN1Address=Security.LAN1Address;
	t_security->LAN2Address=Security.LAN2Address;
	t_security->start_time=Security.start_time;

	t_security->start_time=Security.start_time;
	t_security->tcp_port=Security.tcp_port;

	gate->modbus_address=MoxaDevice.modbus_address;
	gate->status_info=MoxaDevice.status_info;

	t_security->show_data_flow=Security.show_data_flow;
	t_security->show_sys_messages=Security.show_sys_messages;
	t_security->watchdog_timer=Security.watchdog_timer;
//	gate->use_buzzer=gate502.use_buzzer;
//	gate->back_light=gate502.back_light;

	gate->offset1xStatus=MoxaDevice.offset1xStatus;
	gate->offset2xStatus=MoxaDevice.offset2xStatus;
	gate->offset3xRegisters=MoxaDevice.offset3xRegisters;
	gate->offset4xRegisters=MoxaDevice.offset4xRegisters;
	gate->amount1xStatus=MoxaDevice.amount1xStatus;
	gate->amount2xStatus=MoxaDevice.amount2xStatus;
	gate->amount3xRegisters=MoxaDevice.amount3xRegisters;
	gate->amount4xRegisters=MoxaDevice.amount4xRegisters;
  ///--------------------------------------------------
	time(&timestamp);
  ///--------------------------------------------------

	if(t_security->halt==1) {
		Security.halt=1;
		show_confirmation_reboot();
		mxlcm_write(mxlcm_handle, 0, 2, "                ", MAX_LCM_COLS); // 16 characters
		mxlcm_write(mxlcm_handle, 0, 3, "                ", MAX_LCM_COLS); // 16 characters
		mxlcm_write(mxlcm_handle, 0, 4, " program stopped", MAX_LCM_COLS); // 16 characters
		mxlcm_write(mxlcm_handle, 0, 5, "by web-interface", MAX_LCM_COLS); // 16 characters
		mxlcm_write(mxlcm_handle, 0, 6, "                ", MAX_LCM_COLS); // 16 characters
		}

// проверка состояния флага "screen.back_light" в потоке HMI_KEYPAD_LCM_H и управление подсветкой
//	if(gate->back_light!=gate502.back_light) {
//		gate502.back_light = screen.back_light = gate->back_light;
//	  if(screen.back_light) {	mxlcm_control(mxlcm_handle, IOCTL_LCM_BACK_LIGHT_ON);
//	    } else { 							mxlcm_control(mxlcm_handle, IOCTL_LCM_BACK_LIGHT_OFF);}
//		}

	if(t_security->use_buzzer!=Security.use_buzzer) {
//		gate502.use_buzzer = screen.buzzer_control = gate->use_buzzer;
		Security.use_buzzer = t_security->use_buzzer;
		mxbuzzer_beep(mxbzr_handle, 400);
		}

  ///--------------------------------------------------

	for(i=0; i<MAX_VIRTUAL_SLAVES; i++) {
		t_rtm[i].start=vslave[i].start;
		t_rtm[i].length=vslave[i].length;
		t_rtm[i].iface=vslave[i].iface;
		t_rtm[i].device=vslave[i].device;
		t_rtm[i].modbus_table=vslave[i].modbus_table;
		t_rtm[i].offset=vslave[i].offset;
		strcpy(t_rtm[i].device_name, vslave[i].device_name);
	  }
	
	for(i=0; i<MAX_QUERY_ENTRIES; i++) {
		t_proxy[i].start=query_table[i].start;
		t_proxy[i].length=query_table[i].length;
		t_proxy[i].offset=query_table[i].offset;
		t_proxy[i].iface=query_table[i].iface;
		t_proxy[i].device=query_table[i].device;
		t_proxy[i].mbf=query_table[i].mbf;
		t_proxy[i].delay=query_table[i].delay;
		t_proxy[i].critical=query_table[i].critical;
		t_proxy[i].err_counter=query_table[i].err_counter;
		t_proxy[i].status_bit=query_table[i].status_bit;
		strcpy(t_proxy[i].device_name, query_table[i].device_name);
	  }
	
//	for(i=0; i<MAX_TCP_SERVERS; i++) {
//		t_tcpsrv[i].mb_slave=tcp_servers[i].mb_slave;
//		t_tcpsrv[i].ip=tcp_servers[i].ip;
//		t_tcpsrv[i].port=tcp_servers[i].port;
//		t_tcpsrv[i].offset=tcp_servers[i].offset;
//		t_tcpsrv[i].p_num=tcp_servers[i].p_num;
//		strcpy(t_tcpsrv[i].device_name, tcp_servers[i].device_name);
//	  }

  ///*** Заполняем массив диагностической информации, если он был инициализирован

  for(i=0; i<MAX_MOXA_PORTS; i++) {
    MoxaDevice.wData4x[MoxaDevice.status_info+3*i+0]=IfaceRTU[i].stat.accepted;
    MoxaDevice.wData4x[MoxaDevice.status_info+3*i+1]=IfaceRTU[i].stat.sended;
    MoxaDevice.wData4x[MoxaDevice.status_info+3*i+2]=IfaceRTU[i].stat.request_time;
	  }

  for(i=0; i<MAX_QUERY_ENTRIES; i++) {
		j = 0x01 << (i % 16);
    if(query_table[i].status_bit==0)
			MoxaDevice.wData4x[MoxaDevice.status_info+3*MAX_MOXA_PORTS+(i/16)]&=~j;
			else
			MoxaDevice.wData4x[MoxaDevice.status_info+3*MAX_MOXA_PORTS+(i/16)]|=j;
    }

	return 0;
	}

///--------------------------------------------------------------------------
int close_shm()
	{
	shmdt(pointer);
	shmctl(shm_segment_id, IPC_RMID, 0);
	// SHARED MEM: CLOSED
	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, 30, 0, 0, 0, 0);
	return 0;
	}
///--------------------------------------------------------------------------
