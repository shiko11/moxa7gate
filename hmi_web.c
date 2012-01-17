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
#include "hmi_klb.h"

///=== HMI_WEB_H private variables

char *pointer;

// статические данные
GW_AddressMap_Entry *addrmap;
GW_VSlave_Entry *vslave;
GW_ProxyQuery_Entry *pquery;
GW_Exception *exception;

// динамически обновляемые данные
GW_Security *security;
GW_MoxaDevice *gate;
GW_Iface *iface_rtu;
GW_Iface *iface_tcp;
GW_Client *client;
GW_EventLog *event_log;

key_t access_key;
struct shmid_ds shmbuffer;
unsigned int i, j, k;

///=== HMI_WEB_H private functions

int refresh_shm(); // обновление динамических данных, таких как статистика и другие счетчики
int update_shm();  // обновление статических данных, таких как параметры конфигурации

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
int init_hmi_web_h()
  {
	struct shmid_ds shmbuffer;

	access_key=ftok("/tmp/app", 'a');
												
	unsigned int mem_size_ttl =
		sizeof(GW_AddressMap_Entry)*(MODBUS_ADDRESS_MAX+1)+
		sizeof(GW_VSlave_Entry)*MAX_VIRTUAL_SLAVES+
		sizeof(GW_ProxyQuery_Entry)*MAX_QUERY_ENTRIES+
		sizeof(GW_Exception)*MOXAGATE_EXCEPTIONS_NUMBER+
		sizeof(GW_Security)+
		sizeof(GW_MoxaDevice)+
		sizeof(GW_Iface)*MAX_MOXA_PORTS+
		sizeof(GW_Iface)*MAX_TCP_SERVERS+
		sizeof(GW_Client)*MOXAGATE_CLIENTS_NUMBER+
		sizeof(GW_EventLog)+
		sizeof(unsigned int) * EVENT_TEMPLATE_AMOUNT+
		sizeof(char) * EVENT_TEMPLATE_AMOUNT * EVENT_MESSAGE_LENGTH+
		sizeof(GW_Event)*EVENT_LOG_LENGTH;

  shm_segment_id=shmget(access_key, mem_size_ttl, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

	if(shm_segment_id==-1) {

		switch(errno) {
			case ENOENT: sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_FTL|GATEWAY_HMI, HMI_WEB_ENOENT, 0, 0, 0, 0);
                                                                    return HMI_WEB_ENOENT;
			case EACCES: sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_FTL|GATEWAY_HMI, HMI_WEB_EACCES, 0, 0, 0, 0);
                                                                    return HMI_WEB_EACCES;
			case EINVAL: sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_FTL|GATEWAY_HMI, HMI_WEB_EINVAL, 0, 0, 0, 0);
                                                                    return HMI_WEB_EINVAL;
			case ENOMEM: sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_FTL|GATEWAY_HMI, HMI_WEB_ENOMEM, 0, 0, 0, 0);
                                                                    return HMI_WEB_ENOMEM;
			case EEXIST: sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_FTL|GATEWAY_HMI, HMI_WEB_EEXIST, 0, 0, 0, 0);
                                                                    return HMI_WEB_EEXIST;
			default:     sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_FTL|GATEWAY_HMI, HMI_WEB_UNKNOWN,0, 0, 0, 0);
                                                                    return HMI_WEB_UNKNOWN;
			}
	  }

  ///--- permission mode-------
	struct shmid_ds mds;
	mds.shm_perm.mode=438;
	shmctl(shm_segment_id, IPC_SET, &mds);

	shmctl(shm_segment_id, IPC_STAT, &shmbuffer);

	pointer=shmat(shm_segment_id, 0, 0);

  // назначаем указателям полученную память

  k=0;

	addrmap=(GW_AddressMap_Entry *) (pointer+k);

  k+= sizeof(GW_AddressMap_Entry)*(MODBUS_ADDRESS_MAX+1);

	vslave=(GW_VSlave_Entry *) (pointer+k);

  k+= sizeof(GW_VSlave_Entry)*MAX_VIRTUAL_SLAVES;

	pquery=(GW_ProxyQuery_Entry *) (pointer+k);

  k+= sizeof(GW_ProxyQuery_Entry)*MAX_QUERY_ENTRIES;

	exception=(GW_Exception *) (pointer+k);

  k+= sizeof(GW_Exception)*MOXAGATE_EXCEPTIONS_NUMBER;

	security=(GW_Security *) (pointer+k);

  k+= sizeof(GW_Security);

	gate=(GW_MoxaDevice *) (pointer+k);

  k+= sizeof(GW_MoxaDevice);

	iface_rtu=(GW_Iface *) (pointer+k);

  k+= sizeof(GW_Iface)*MAX_MOXA_PORTS;

	iface_tcp=(GW_Iface *) (pointer+k);

  k+= sizeof(GW_Iface)*MAX_TCP_SERVERS;

	client=(GW_Client *) (pointer+k);

  k+= sizeof(GW_Client)*MOXAGATE_CLIENTS_NUMBER;

	event_log=(GW_EventLog *) (pointer+k);

  k+= sizeof(GW_EventLog);

	EventLog.msg_index=(unsigned int *) (pointer+k);

  k+= sizeof(unsigned int) * EVENT_TEMPLATE_AMOUNT;

	EventLog.msg_tpl=(char *) (pointer+k);

  k+= sizeof(char) * EVENT_TEMPLATE_AMOUNT * EVENT_MESSAGE_LENGTH;

  EventLog.app_log=
  event_log->app_log=
    (GW_Event *) (pointer+k);

///  printf("%d %d %d\n", shm_data, shared_memory, app_log);
///  printf("%p %p %p\n", shm_data, shared_memory, app_log);

  // вторично инициализируем массив шаблонов сообщений и индексов,
  // код возврата не проверяем, т.к. в первый раз ошибок не было:
  init_message_templates();

  shm_segment_ok=1;

	// SHARED MEM: OK
	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|GATEWAY_HMI, HMI_WEB_OK, shmbuffer.shm_segsz, 0, 0, 0);

  return 0;
  }

///--------------------------------------------------------------------------
// обновление динамических данных, таких как статистика и другие счетчики
int refresh_shm()
	{
  if(shm_segment_ok!=1) return 1;

  for(i==0; i<MAX_QUERY_ENTRIES; i++) {
    pquery[i].err_counter= PQuery[i].err_counter;
    pquery[i].status_bit=  PQuery[i].status_bit;
    }

  security->show_data_flow=    Security.show_data_flow;
  security->show_sys_messages= Security.show_sys_messages;
  security->watchdog_timer=    Security.watchdog_timer;
  security->use_buzzer=        Security.use_buzzer;
  security->halt=              Security.halt;
  security->accepted_connections_number= Security.accepted_connections_number;
  security->current_connections_number=  Security.current_connections_number;
  security->rejected_connections_number= Security.rejected_connections_number;

  //gate->queue= MoxaDevice.queue;
  //gate->stat=  MoxaDevice.stat;
  gate->start_time=     MoxaDevice.start_time;
  gate->modbus_address= MoxaDevice.modbus_address;
  gate->status_info=    MoxaDevice.status_info;
  gate->map2Xto4X=      MoxaDevice.map2Xto4X;
  gate->offset1xStatus=    MoxaDevice.offset1xStatus;
  gate->offset2xStatus=    MoxaDevice.offset2xStatus;
  gate->offset3xRegisters= MoxaDevice.offset3xRegisters;
  gate->offset4xRegisters= MoxaDevice.offset4xRegisters;
  gate->amount1xStatus=    MoxaDevice.amount1xStatus;
  gate->amount2xStatus=    MoxaDevice.amount2xStatus;
  gate->amount3xRegisters= MoxaDevice.amount3xRegisters;
  gate->amount4xRegisters= MoxaDevice.amount4xRegisters;
  gate->used1xStatus=      MoxaDevice.used1xStatus;
  gate->used2xStatus=      MoxaDevice.used2xStatus;
  gate->used3xRegisters=   MoxaDevice.used3xRegisters;
  gate->used4xRegisters=   MoxaDevice.used4xRegisters;

  for(i==0; i<MAX_MOXA_PORTS; i++) {
    //iface_rtu[i].Security= IfaceRTU[i].Security;
    //iface_rtu[i].queue=    IfaceRTU[i].queue;
    //iface_rtu[i].stat=     IfaceRTU[i].stat;
    //iface_rtu[i].bridge_status[4]= IfaceRTU[i].bridge_status[4];
    }

  for(i==0; i<MAX_TCP_SERVERS; i++) {
    //iface_tcp[i].Security= IfaceTCP[i].Security;
    //iface_tcp[i].queue=    IfaceTCP[i].queue;
    //iface_tcp[i].stat=     IfaceTCP[i].stat;
    //iface_tcp[i].bridge_status[4]=IfaceTCP[i].bridge_status[4];
    }

  for(i==0; i<MOXAGATE_CLIENTS_NUMBER; i++) {
    client[i].status=Client[i].status;
    client[i].iface=Client[i].iface;
    ///!!!client[i].device_name[DEVICE_NAME_LENGTH]=Client[i].device_name[DEVICE_NAME_LENGTH];
    client[i].ip=Client[i].ip;
    client[i].port=Client[i].port;
    client[i].connection_time=Client[i].connection_time;
    client[i].disconnection_time=Client[i].disconnection_time;
    client[i].last_activity_time=Client[i].last_activity_time;
    //client[i].stat=Client[i].stat;
    }

  event_log->app_log_current_entry=EventLog.app_log_current_entry;
  event_log->app_log_entries_total=EventLog.app_log_entries_total;
  event_log->msg_filter=      EventLog.msg_filter;
  for(i=0; i<4; i++) {
    event_log-> cat_msgs_amount[i]= EventLog. cat_msgs_amount[i];
    event_log->type_msgs_amount[i]= EventLog.type_msgs_amount[i];
    }

  // обновляем метку времени
  time(&Security.timestamp);
  security->timestamp=Security.timestamp;

  return 0;
  }

///--------------------------------------------------------------------------
// обновление статических данных, таких как параметры конфигурации
int update_shm()
  {
  if(shm_segment_ok!=1) return 1;

  for(i==0; i<MODBUS_ADDRESS_MAX+1; i++) {
    addrmap[i].iface=   AddressMap[i].iface;
    addrmap[i].address= AddressMap[i].address;
    }

  for(i==0; i<MAX_VIRTUAL_SLAVES; i++) {
    vslave[i].iface=        VSlave[i].iface;
    vslave[i].device=       VSlave[i].device;
    vslave[i].modbus_table= VSlave[i].modbus_table;
    vslave[i].offset=       VSlave[i].offset;
    vslave[i].start=        VSlave[i].start;
    vslave[i].length=       VSlave[i].length;
    //vslave[i].device_name[DEVICE_NAME_LENGTH]=VSlave[i].device_name[DEVICE_NAME_LENGTH];
    }

  for(i==0; i<MAX_QUERY_ENTRIES; i++) {
    pquery[i].iface=    PQuery[i].iface;
    pquery[i].device=   PQuery[i].device;
    pquery[i].mbf=      PQuery[i].mbf;
    pquery[i].access=   PQuery[i].access;
    pquery[i].start=    PQuery[i].start;
    pquery[i].length=   PQuery[i].length;
    pquery[i].offset=   PQuery[i].offset;
    pquery[i].critical= PQuery[i].critical;
    //pquery[i].device_name[DEVICE_NAME_LENGTH]=PQuery[i].device_name[DEVICE_NAME_LENGTH];
    }

  for(i==0; i<MOXAGATE_EXCEPTIONS_NUMBER; i++) {
    exception[i].stage=  Exception[i].stage;
    exception[i].action= Exception[i].action;
    exception[i].prm1=   Exception[i].prm1;
    exception[i].prm2=   Exception[i].prm2;
    exception[i].prm3=   Exception[i].prm3;
    exception[i].prm4=   Exception[i].prm4;
    //exception[i].comment[DEVICE_NAME_LENGTH]=Exception[i].comment[DEVICE_NAME_LENGTH];
    }

  security->tcp_port=Security.tcp_port;
  security->start_time=Security.start_time;
  //security->Object[DEVICE_NAME_LENGTH]=Security.Object[DEVICE_NAME_LENGTH];
  //security->Location[DEVICE_NAME_LENGTH]=Security.Location[DEVICE_NAME_LENGTH];
  //security->Label[DEVICE_NAME_LENGTH]=Security.Label[DEVICE_NAME_LENGTH];
  //security->NetworkName[DEVICE_NAME_LENGTH]=Security.NetworkName[DEVICE_NAME_LENGTH];
  //security->LAN1Address=Security.LAN1Address;
  //security->LAN2Address=Security.LAN2Address;
  //security->VersionNumber[DEVICE_NAME_LENGTH]=Security.VersionNumber[DEVICE_NAME_LENGTH];
  //security->VersionTime[DEVICE_NAME_LENGTH]=Security.VersionTime[DEVICE_NAME_LENGTH];
  //security->Model[DEVICE_NAME_LENGTH]=Security.Model[DEVICE_NAME_LENGTH];
  //security->TCPIndex[MAX_TCP_SERVERS+1]=Security.TCPIndex[MAX_TCP_SERVERS+1];

  for(i==0; i<MAX_MOXA_PORTS; i++) {
    iface_rtu[i].modbus_mode=      IfaceRTU[i].modbus_mode;
    //iface_rtu[i].description[DEVICE_NAME_LENGTH]=IfaceRTU[i].description[DEVICE_NAME_LENGTH];
    //iface_rtu[i].serial=   IfaceRTU[i].serial;
    ///!!!iface_rtu[i].ethernet= IfaceRTU[i].ethernet;
    //iface_rtu[i].PQueryIndex[MAX_QUERY_ENTRIES+1]=IfaceRTU[i].PQueryIndex[MAX_QUERY_ENTRIES+1];
    }

  for(i==0; i<MAX_TCP_SERVERS; i++) {
    iface_tcp[i].modbus_mode=IfaceTCP[i].modbus_mode;
    //iface_tcp[i].description[DEVICE_NAME_LENGTH]=IfaceTCP[i].description[DEVICE_NAME_LENGTH];
    ///!!!iface_tcp[i].serial=IfaceTCP[i].serial;
    //iface_tcp[i].ethernet=IfaceTCP[i].ethernet;
    //iface_tcp[i].PQueryIndex[MAX_QUERY_ENTRIES+1]=IfaceTCP[i].PQueryIndex[MAX_QUERY_ENTRIES+1];
    }

  // обновляем метку времени
  time(&Security.timestamp);
  security->timestamp=Security.timestamp;

  return 0;
  }

///--------------------------------------------------------------------------
int close_shm()
	{
  if(shm_segment_ok!=1) return 1;

	//shmdt(pointer); // Вызывает ошибку "Segmentation fault"
	shmctl(shm_segment_id, IPC_RMID, 0);

	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|GATEWAY_HMI, HMI_WEB_CLOSED, 0, 0, 0, 0);
	return 0;
	}
///--------------------------------------------------------------------------
