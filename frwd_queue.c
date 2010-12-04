/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///*********  МОДУЛЬ ФУНКЦИЙ ПЕРЕНАПРАВЛЕНИЯ, ОЧЕРЕДЕЙ ЗАПРОСОВ MODBUS *********

///=== FRWD_QUEUE_H IMPLEMENTATION

#include "frwd_queue.h"
#include "interfaces.h"
#include "moxagate.h"
#include "messages.h"

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short int *array;
	struct seminfo *__buf;
  };

///=== FRWD_QUEUE_H private variables

///=== FRWD_QUEUE_H private functions

int translateAddress(u8 unit_id, int *port_id, int *device_id);
int translateRegisters(int start_address, int length, int *port_id, int *device_id);
int translateProxyDevice(int start_address, int length, int *port_id, int *device_id);

///-----------------------------------------------------------------------------
int init_AddressMap_Entry(int index)
  {
  /// таблица назначения адресов
  AddressMap[index].iface=GATEWAY_NONE;
  AddressMap[index].address=MODBUS_ADDRESS_BROADCAST;

  return 0;
  }

int check_AddressMap_Entry(int index)
  {

  if( ((AddressMap[index].iface > GATEWAY_P8) && (AddressMap[index].iface < GATEWAY_T01)) ||
      (AddressMap[index].iface > GATEWAY_T32) ||
      (AddressMap[index].iface != GATEWAY_MOXAGATE)
    ) return ATM_IFACE;

  // игнорируем адрес MODBUS, если указан не RTU-интерфейс:
  if(AddressMap[index].iface<=GATEWAY_P8)
    if(  (AddressMap[index].address < MODBUS_ADDRESS_MIN) ||
         (AddressMap[index].address > MODBUS_ADDRESS_MAX)
      ) return ATM_MBADDR;

 	return 0;
  }

///-----------------------------------------------------------------------------
int init_Vslave_Entry(int index)
  {
  /// таблица назначения регистров
	vslave[index].iface=GATEWAY_NONE;
	vslave[index].device=MODBUS_ADDRESS_BROADCAST;
  vslave[index].modbus_table=MB_TABLE_NONE;
  vslave[index].offset=0;

	vslave[index].start=0;
	vslave[index].length=0;

	vslave[index].device_name[0]=0;

  return 0;
  }

int check_Vslave_Entry(int index)
  {

  if( ((vslave[index].iface > GATEWAY_P8) && (vslave[index].iface < GATEWAY_T01)) ||
      (vslave[index].iface > GATEWAY_T32)
    ) return VSLAVE_IFACE;
	
  // игнорируем адрес MODBUS, если указан не RTU-интерфейс:
  if(AddressMap[index].iface<=GATEWAY_P8)
    if(  (vslave[index].device < MODBUS_ADDRESS_MIN) ||
         (vslave[index].device > MODBUS_ADDRESS_MAX)
      ) return VSLAVE_MBADDR;

	if(!(
			vslave[index].modbus_table==COIL_STATUS_TABLE      ||
			vslave[index].modbus_table==INPUT_STATUS_TABLE     ||
			vslave[index].modbus_table==HOLDING_REGISTER_TABLE ||
			vslave[index].modbus_table==INPUT_REGISTER_TABLE
		)) return VSLAVE_MBTABL;

  // параметры, задающие адресуемый диапазон, проверяем в комплексе

  // начальный регистр диапазона	
  if((unsigned int)(vslave[index].offset + vslave[index].start) > MB_ADDRESS_LAST) return VSLAVE_BEGDIAP;

  // конечный регистр диапазона	
  if((unsigned int)( \
    vslave[index].offset + \
    vslave[index].start + \
    vslave[index].length) > MB_ADDRESS_LAST) return VSLAVE_ENDDIAP;

  if(vslave[index].length==0) return VSLAVE_LENDIAP;
	
  /// Наименование устройства
  // vslave[index].device_name

 	return 0;
  }

///-----------------------------------------------------------------------------
int init_ProxyQuery_Entry(int index)
  {
  /// таблица опроса
	query_table[index].iface=GATEWAY_NONE;
	query_table[index].device=MODBUS_ADDRESS_BROADCAST;
  query_table[index].mbf=MB_FUNC_NONE;
  query_table[index].access=QT_ACCESS_DISABLED;

	query_table[index].start=0;
	query_table[index].length=0;
  query_table[index].offset=0;

  query_table[index].delay=0;
	query_table[index].critical=0;
	query_table[index].device_name[0]=0;

  query_table[index].err_counter=0;
  query_table[index].status_bit=0;

  return 0;
  }

int check_ProxyQuery_Entry(int index)
  {

  if( ((query_table[index].iface > GATEWAY_P8) && (query_table[index].iface < GATEWAY_T01)) ||
      (query_table[index].iface > GATEWAY_T32)
    ) return PQUERY_IFACE;
	
  // игнорируем адрес MODBUS, если указан не RTU-интерфейс:
  if(AddressMap[index].iface<=GATEWAY_P8)
    if(  (query_table[index].device < MODBUS_ADDRESS_MIN) ||
         (query_table[index].device > MODBUS_ADDRESS_MAX)
      ) return PQUERY_MBADDR;

	if(!(
			query_table[index].mbf==MBF_READ_COILS             ||
			query_table[index].mbf==MBF_READ_DECRETE_INPUTS    ||
			query_table[index].mbf==MBF_READ_HOLDING_REGISTERS ||
			query_table[index].mbf==MBF_READ_INPUT_REGISTERS
		)) return PQUERY_MBTABL;

	if(!(
			query_table[index].access==QT_ACCESS_READWRITE ||
			query_table[index].access==QT_ACCESS_READONLY  ||
			query_table[index].access==QT_ACCESS_DISABLED
		)) return PQUERY_ACCESS;

  // начальный регистр области чтения
  // конечный регистр области чтения
  if((unsigned int)(query_table[index].start + query_table[index].length -1) > MB_ADDRESS_LAST)
    return PQUERY_ENDREGREAD;

  // длина пакета данных
  if( (query_table[index].mbf==MBF_READ_COILS) && (
      (query_table[index].length < MBF_0x01_MIN_QUANTITY) ||
      (query_table[index].length > MBF_0x01_MAX_QUANTITY)
      )) return PQUERY_LENPACKET;
  if( (query_table[index].mbf==MBF_READ_DECRETE_INPUTS) && (
      (query_table[index].length < MBF_0x02_MIN_QUANTITY) ||
      (query_table[index].length > MBF_0x02_MAX_QUANTITY)
      )) return PQUERY_LENPACKET;
  if( (query_table[index].mbf==MBF_READ_HOLDING_REGISTERS) && (
      (query_table[index].length < MBF_0x03_MIN_QUANTITY) ||
      (query_table[index].length > MBF_0x03_MAX_QUANTITY)
      )) return PQUERY_LENPACKET;
  if( (query_table[index].mbf==MBF_READ_INPUT_REGISTERS) && (
      (query_table[index].length < MBF_0x04_MIN_QUANTITY) ||
      (query_table[index].length > MBF_0x04_MAX_QUANTITY)
      )) return PQUERY_LENPACKET;

  // начальный регистр области записи
  // конечный регистр области записи
  if((unsigned int)(query_table[index].offset + query_table[index].length) > MB_ADDRESS_LAST)
    return PQUERY_ENDREGWRITE;

  if( (((query_table[index].iface & IFACETCP_MASK)==0) && (query_table[index].delay < QT_DELAY_RTU_MIN)) ||
      (((query_table[index].iface & IFACETCP_MASK)!=0) && (query_table[index].delay < QT_DELAY_TCP_MIN))
    ) return PQUERY_DELAYMIN;
  if( (((query_table[index].iface & IFACETCP_MASK)==0) && (query_table[index].delay > QT_DELAY_RTU_MAX)) ||
      (((query_table[index].iface & IFACETCP_MASK)!=0) && (query_table[index].delay > QT_DELAY_TCP_MAX))
    ) return PQUERY_DELAYMAX;

  if(query_table[index].critical > QT_CRITICAL_MAX) return PQUERY_ERRCNTR;
	
  /// Наименование устройства
  // query_table[i].device_name

 	return 0;
  }

///-----------------------------------------------------------------------------
int init_Exception(int index)
  {
  /// таблица исключений
	Exception[index].stage=EXPT_STAGE_UNDEFINED;
	Exception[index].action=EXPT_ACT_NONE;

  Exception[index].prm1=0xffffffff;
  Exception[index].prm2=0xffffffff;
  Exception[index].prm3=0xffffffff;
  Exception[index].prm4=0xffffffff;

	Exception[index].comment[0]=0;

  return 0;
  }

int check_Exception(int index)
  {

	if(!( // этап прохождения запроса
			Exception[index].stage==EXPT_STAGE_QUERY_RECV_RAW ||
			Exception[index].stage==EXPT_STAGE_QUERY_RECV ||
			Exception[index].stage==EXPT_STAGE_QUERY_FRWD ||
			Exception[index].stage==EXPT_STAGE_RESPONSE_RECV_RAW ||
			Exception[index].stage==EXPT_STAGE_RESPONSE_RECV ||
			Exception[index].stage==EXPT_STAGE_RESPONSE_SEND
		)) return EXPT_STAGE;

	if(!( // действие
			Exception[index].action==EXPT_ACT_SKS07_DIOGEN
		)) return EXPT_ACTION;

  // проверяем набор параметров по каждому из определенных действий

	if(Exception[index].action==EXPT_ACT_SKS07_DIOGEN) {
    if( ((Exception[index].prm1 > GATEWAY_P8) && (Exception[index].prm1 < GATEWAY_T01)) ||
        (Exception[index].prm1 > GATEWAY_T32)
      ) return EXPT_PRM1;
	
    if(  (Exception[index].prm2 < MODBUS_ADDRESS_MIN) ||
         (Exception[index].prm2 > MODBUS_ADDRESS_MAX)
      ) return EXPT_PRM2;
    }
	
  // Exception[i].prm1
  // Exception[i].prm2
  // Exception[i].prm3
  // Exception[i].prm4																		

  /// Комментарий
  // Exception[i].comment

 	return 0;
  }

///-----------------------------------------------------------------------------
int init_frwd_queue_h()
  {
  unsigned int i;			

	// memset(vslave,0,sizeof(vslave));
	// memset(query_table,0,sizeof(query_table));

  for(i=0; i<=MODBUS_ADDRESS_MAX; i++)        init_AddressMap_Entry(i);
  for(i=0; i<MAX_VIRTUAL_SLAVES; i++)         init_Vslave_Entry(i);
  for(i=0; i<MAX_QUERY_ENTRIES; i++)          init_ProxyQuery_Entry(i);
  for(i=0; i<MOXAGATE_EXCEPTIONS_NUMBER; i++) init_Exception(i);

  return 0;
  }

///-----------------------------------------------------------------------------
int init_queue()
	{

	key_t sem_key=ftok("/tmp/app", 'b');
	
	if((semaphore_id = semget(sem_key, GATEWAY_ASSETS, IPC_CREAT|IPC_EXCL|0666)) == -1) {
		sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, 29, 0, 0, 0, 0);
		return 1;
	 	}
	
	union semun sems;
	unsigned short values[1];

	values[0]=0;
	sems.array = values;
	/* Инициализируем все элементы одним значением */
	semctl(semaphore_id, 0, SETALL, sems);
	
//		printf("maximal semaphore amount %d\n", SEMMSL);
	return 0;
	}
///-----------------------------------------------------------------------------------------------------------------
int enqueue_query_ex(GW_Queue *queue, int client_id, int device_id, u8 *adu, u16 adu_len)
  {

			  if(queue->queue_len==MAX_GATEWAY_QUEUE_LENGTH) { ///!!! modbus exception response, reset queue
			 		// QUEUE OVERLOADED
			 		sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|GATEWAY_FRWD, 149, queue->port_id, client_id, 0, 0);
					return 1;
					}
	
		  	pthread_mutex_lock(&queue->queue_mutex);
	
				int j, queue_current=(queue->queue_start+queue->queue_len)%MAX_GATEWAY_QUEUE_LENGTH;
	
//				printf("Queue %d\n", queue->port_id+1);
				for(j=0; j<adu_len; j++) {
					queue->queue_adu[queue_current][j]=adu[j];
//					printf("(%2.2X)", adu[j]);
					}
//				printf("\n");
				queue->queue_adu_len[queue_current]=adu_len;
				queue->queue_clients[queue_current]=client_id;
				queue->queue_slaves[queue_current]=device_id; ///!!! ATM & PROXY uses this field
	
				queue->queue_len++;
	
	///$$$	struct timeval tv1, tv2;
	///$$$	struct timezone tz;
	//(tv2.tv_sec%10)*1000+tv2.tv_usec/1000
	///$$$		gettimeofday(&tv2, &tz);
	///$$$			printf("\nP%d TCP%4.4d time%d query %d begin[%d]", port_id+1, gate502.clients[i].port, (tv2.tv_sec%10)*10+tv2.tv_usec/100000, IfaceRTU[port_id].queue_len, queue_current);
	
	pthread_mutex_unlock(&queue->queue_mutex);
	//printf("mutex passed\n");

//	printf("query_queued %d P%d, len=%d\n", queue_current, queue->port_id+1, queue->queue_len);
//	sysmsg_ex(EVENT_CAT_TRAFFIC|EVENT_TYPE_INF|GATEWAY_FRWD, 220, queue->port_id, client_id, queue->queue_len, 0);

	queue->operations[0].sem_op=1;
//	printf("semaphore inc: port=%d\n", queue->port_id+1);
	semop(semaphore_id, queue->operations, 1);

  return 0;
  }
///-----------------------------------------------------------------------------------------------------------------
int get_query_from_queue(GW_Queue *queue, int *client_id, int *device_id, u8 *adu, u16 *adu_len)
  {
	int		status;

	queue->operations[0].sem_op=-1;
//	printf("semaphore dec: port=%d\n", queue->port_id+1);
	status=semop(semaphore_id, queue->operations, 1);
				 
		if(status==-1) return status;

	  if(queue->queue_len==0) { /// внутренняя ошибка в программе
	 		// QUEUE EMPTY
	 		sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_FRWD, 148, queue->port_id, 0, 0, 0);
			return 1;
			}

  	pthread_mutex_lock(&queue->queue_mutex);

		int j, queue_current=queue->queue_start;

		for(j=0; j<queue->queue_adu_len[queue_current]; j++) {
			adu[j]=queue->queue_adu[queue_current][j];
//			printf("(%2.2X)", adu[j]);
			}
		*adu_len=queue->queue_adu_len[queue_current];
//		printf("\n");
		*client_id=queue->queue_clients[queue_current];
		*device_id=queue->queue_slaves[queue_current]; ///!!! ATM & PROXY uses this field
	

		///$$$printf(" go[%d]", queue_current);
		queue->queue_start=(queue->queue_start+1)%MAX_GATEWAY_QUEUE_LENGTH;
		queue->queue_len--;

  	pthread_mutex_unlock(&queue->queue_mutex);
	
//		printf("query_accepted P%d, len=%d\n", queue->port_id+1, queue->queue_len);
//	sysmsg_ex(EVENT_CAT_TRAFFIC|EVENT_TYPE_INF|GATEWAY_FRWD, 221, queue->port_id, *client_id, queue->queue_len, 0);

  return 0;
  }
///-----------------------------------------------------------------------------------------------------------------
int checkDiapason(int function, int start_address, int length)
	{
	static unsigned int internal_start, internal_end;

	switch(function) {

		case MBF_READ_COILS:
		case MBF_WRITE_SINGLE_COIL:
		case MBF_WRITE_MULTIPLE_COILS:
			internal_start = MoxaDevice.offset1xStatus;
			internal_end = MoxaDevice.offset1xStatus + MoxaDevice.amount1xStatus;
			break;

		case MBF_READ_DECRETE_INPUTS:
			internal_start = MoxaDevice.offset2xStatus;
			internal_end = MoxaDevice.offset2xStatus + MoxaDevice.amount2xStatus;
			break;

		case MBF_READ_INPUT_REGISTERS:
			internal_start = MoxaDevice.offset3xRegisters;
			internal_end = MoxaDevice.offset3xRegisters + MoxaDevice.amount3xRegisters;
			break;

		case MBF_READ_HOLDING_REGISTERS:
		case MBF_WRITE_SINGLE_REGISTER:
		case MBF_WRITE_MULTIPLE_REGISTERS:
			internal_start = MoxaDevice.offset4xRegisters;
			internal_end = MoxaDevice.offset4xRegisters + MoxaDevice.amount4xRegisters;
			break;

		default: internal_start = internal_end = 0;
		}

	if(start_address >= internal_start) {
		if(start_address + length <= internal_end) return MOXA_DIAPASON_INSIDE;
		if(start_address >= internal_end) return MOXA_DIAPASON_OUTSIDE;
		return MOXA_DIAPASON_OVERLAPPED;
		} else {
			if(start_address + length <= internal_start) return MOXA_DIAPASON_OUTSIDE;
			return MOXA_DIAPASON_OVERLAPPED;
			}

	return MOXA_DIAPASON_UNDEFINED;
	}
///-----------------------------------------------------------------------------------------------------------------
int translateAddress(u8 unit_id, int *port_id, int *device_id)
  {
	if((unit_id < 1) || (unit_id > 247)) return 1;
	*port_id = unit_id / 30;
	if(*port_id > GATEWAY_P8) *port_id=GATEWAY_P8;
	*device_id = unit_id - *port_id * 30;

  if(IfaceRTU[*port_id].modbus_mode!=GATEWAY_ATM) return 2;

  return 0;
  }


int translateRegisters(int start_address, int length, int *port_id, int *device_id)
  {
	int i;

  for(i=0; i<MAX_VIRTUAL_SLAVES; i++) {
    if(
//			(vslave[i].start==0) ||
			(vslave[i].length==0) ||
//			(vslave[i].port==SERIAL_STUB) ||
			(vslave[i].device==0)
			) continue;

			if((start_address>=vslave[i].start)&&(start_address<(vslave[i].start+vslave[i].length))) break;
			}
	if(i==MAX_VIRTUAL_SLAVES) return 1;
  //printf("%d + %d < %d + %d (i=%d)\n", vslave[i].start, vslave[i].length, start_address, length, i);
  if((vslave[i].start+vslave[i].length)<(start_address+length)) return 2;

	*port_id=vslave[i].iface;
	*device_id=i; // индекс блока адресов виртуального устройства
	
  return 0;
  }

int translateProxyDevice(int start_address, int length, int *port_id, int *device_id)
  {
	int i;

  for(i=0; i<MAX_QUERY_ENTRIES; i++) {
    if(
			(query_table[i].length==0) ||
			(query_table[i].mbf==0) ||
			(query_table[i].device==0)
			) continue;

			if((start_address>=query_table[i].offset)&&(start_address<(query_table[i].offset+query_table[i].length)))
				break;
			}
	if(i==MAX_VIRTUAL_SLAVES) return 1;
  //printf("%d + %d < %d + %d (i=%d)\n", vslave[i].start, vslave[i].length, start_address, length, i);
  if((query_table[i].offset+query_table[i].length)<(start_address+length)) return 2;

	*port_id=query_table[i].iface;
	*device_id=i; // индекс блока адресов виртуального устройства
	
  return 0;
  }

///-----------------------------------------------------------------------------
///-----------------------------------------------------------------------------
///-----------------------------------------------------------------------------
///-----------------------------------------------------------------------------
///-----------------------------------------------------------------------------
