/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///*********  МОДУЛЬ ФУНКЦИЙ ПЕРЕНАПРАВЛЕНИЯ, ОЧЕРЕДЕЙ ЗАПРОСОВ MODBUS *********

///=== FRWD_QUEUE_H IMPLEMENTATION

#include "forwarding.h"
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

  if( ( (AddressMap[index].iface > GATEWAY_P8) &&
        (AddressMap[index].iface < GATEWAY_T01) &&
        (AddressMap[index].iface != GATEWAY_MOXAGATE)
      ) ||
      (AddressMap[index].iface > GATEWAY_T32)
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
	VSlave[index].iface=GATEWAY_NONE;
	VSlave[index].device=MODBUS_ADDRESS_BROADCAST;
  VSlave[index].modbus_table=MB_TABLE_NONE;
  VSlave[index].offset=0;

	VSlave[index].start=0;
	VSlave[index].length=0;

	VSlave[index].device_name[0]=0;

  return 0;
  }

int check_Vslave_Entry(int index)
  {

  if( ((VSlave[index].iface > GATEWAY_P8) && (VSlave[index].iface < GATEWAY_T01)) ||
      (VSlave[index].iface > GATEWAY_T32)
    ) return VSLAVE_IFACE;
	
  // игнорируем адрес MODBUS, если указан не RTU-интерфейс:
  if(AddressMap[index].iface<=GATEWAY_P8)
    if(  (VSlave[index].device < MODBUS_ADDRESS_MIN) ||
         (VSlave[index].device > MODBUS_ADDRESS_MAX)
      ) return VSLAVE_MBADDR;

	if(!(
			VSlave[index].modbus_table==COIL_STATUS_TABLE      ||
			VSlave[index].modbus_table==INPUT_STATUS_TABLE     ||
			VSlave[index].modbus_table==HOLDING_REGISTER_TABLE ||
			VSlave[index].modbus_table==INPUT_REGISTER_TABLE
		)) return VSLAVE_MBTABL;

  // параметры, задающие адресуемый диапазон, проверяем в комплексе

  // начальный регистр диапазона	
  if((unsigned int)(VSlave[index].offset + VSlave[index].start) > MB_ADDRESS_LAST) return VSLAVE_BEGDIAP;

  // конечный регистр диапазона	
  if((unsigned int)( \
    VSlave[index].offset + \
    VSlave[index].start + \
    VSlave[index].length) > MB_ADDRESS_LAST) return VSLAVE_ENDDIAP;

  if(VSlave[index].length==0) return VSLAVE_LENDIAP;
	
  /// Наименование устройства
  // VSlave[index].device_name

 	return 0;
  }

///-----------------------------------------------------------------------------
int init_ProxyQuery_Entry(int index)
  {
  /// таблица опроса
	PQuery[index].iface=GATEWAY_NONE;
	PQuery[index].device=MODBUS_ADDRESS_BROADCAST;
  PQuery[index].mbf=MB_FUNC_NONE;
  PQuery[index].access=QT_ACCESS_DISABLED;

	PQuery[index].start=0;
	PQuery[index].length=0;
  PQuery[index].offset=0;

  PQuery[index].delay=0;
	PQuery[index].critical=0;
	PQuery[index].device_name[0]=0;

  PQuery[index].err_counter=0;
  PQuery[index].status_bit=0;

  return 0;
  }

int check_ProxyQuery_Entry(int index)
  {

  if( ((PQuery[index].iface > GATEWAY_P8) && (PQuery[index].iface < GATEWAY_T01)) ||
      (PQuery[index].iface > GATEWAY_T32)
    ) return PQUERY_IFACE;
	
  // игнорируем адрес MODBUS, если указан не RTU-интерфейс:
  if(AddressMap[index].iface<=GATEWAY_P8)
    if(  (PQuery[index].device < MODBUS_ADDRESS_MIN) ||
         (PQuery[index].device > MODBUS_ADDRESS_MAX)
      ) return PQUERY_MBADDR;

	if(!(
			PQuery[index].mbf==MBF_READ_COILS             ||
			PQuery[index].mbf==MBF_READ_DECRETE_INPUTS    ||
			PQuery[index].mbf==MBF_READ_HOLDING_REGISTERS ||
			PQuery[index].mbf==MBF_READ_INPUT_REGISTERS
		)) return PQUERY_MBTABL;

	if(!(
			PQuery[index].access==QT_ACCESS_READWRITE ||
			PQuery[index].access==QT_ACCESS_READONLY  ||
			PQuery[index].access==QT_ACCESS_DISABLED
		)) return PQUERY_ACCESS;

  // начальный регистр области чтения
  // конечный регистр области чтения
  if((unsigned int)(PQuery[index].start + PQuery[index].length -1) > MB_ADDRESS_LAST)
    return PQUERY_ENDREGREAD;

  // длина пакета данных
  if( (PQuery[index].mbf==MBF_READ_COILS) && (
      (PQuery[index].length < MBF_0x01_MIN_QUANTITY) ||
      (PQuery[index].length > MBF_0x01_MAX_QUANTITY)
      )) return PQUERY_LENPACKET;
  if( (PQuery[index].mbf==MBF_READ_DECRETE_INPUTS) && (
      (PQuery[index].length < MBF_0x02_MIN_QUANTITY) ||
      (PQuery[index].length > MBF_0x02_MAX_QUANTITY)
      )) return PQUERY_LENPACKET;
  if( (PQuery[index].mbf==MBF_READ_HOLDING_REGISTERS) && (
      (PQuery[index].length < MBF_0x03_MIN_QUANTITY) ||
      (PQuery[index].length > MBF_0x03_MAX_QUANTITY)
      )) return PQUERY_LENPACKET;
  if( (PQuery[index].mbf==MBF_READ_INPUT_REGISTERS) && (
      (PQuery[index].length < MBF_0x04_MIN_QUANTITY) ||
      (PQuery[index].length > MBF_0x04_MAX_QUANTITY)
      )) return PQUERY_LENPACKET;

  // начальный регистр области записи
  // конечный регистр области записи
  if((unsigned int)(PQuery[index].offset + PQuery[index].length) > MB_ADDRESS_LAST)
    return PQUERY_ENDREGWRITE;

  if( (((PQuery[index].iface & IFACETCP_MASK)==0) && (PQuery[index].delay < QT_DELAY_RTU_MIN)) ||
      (((PQuery[index].iface & IFACETCP_MASK)!=0) && (PQuery[index].delay < QT_DELAY_TCP_MIN))
    ) return PQUERY_DELAYMIN;
  if( (((PQuery[index].iface & IFACETCP_MASK)==0) && (PQuery[index].delay > QT_DELAY_RTU_MAX)) ||
      (((PQuery[index].iface & IFACETCP_MASK)!=0) && (PQuery[index].delay > QT_DELAY_TCP_MAX))
    ) return PQUERY_DELAYMAX;

  if(PQuery[index].critical > QT_CRITICAL_MAX) return PQUERY_ERRCNTR;
	
  /// Наименование устройства
  // PQuery[i].device_name

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

	memset(AddressMap,  0, sizeof(AddressMap));
	memset(VSlave,      0, sizeof(VSlave));
	memset(PQuery, 0, sizeof(PQuery));
	memset(Exception,   0, sizeof(Exception));

  for(i=0; i<=MODBUS_ADDRESS_MAX; i++)        init_AddressMap_Entry(i);
  for(i=0; i<MAX_VIRTUAL_SLAVES; i++)         init_Vslave_Entry(i);
  for(i=0; i<MAX_QUERY_ENTRIES; i++)          init_ProxyQuery_Entry(i);
  for(i=0; i<MOXAGATE_EXCEPTIONS_NUMBER; i++) init_Exception(i);

  vsmem_offset1xStatus=vsmem_offset2xStatus=vsmem_offset3xRegisters=vsmem_offset4xRegisters=0;
  vsmem_amount1xStatus=vsmem_amount2xStatus=vsmem_amount3xRegisters=vsmem_amount4xRegisters=0;
  vsmem_used1xStatus=  vsmem_used2xStatus=  vsmem_used3xRegisters=  vsmem_used4xRegisters  =0;

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
//			(VSlave[i].start==0) ||
			(VSlave[i].length==0) ||
//			(VSlave[i].port==SERIAL_STUB) ||
			(VSlave[i].device==0)
			) continue;

			if((start_address>=VSlave[i].start)&&(start_address<(VSlave[i].start+VSlave[i].length))) break;
			}
	if(i==MAX_VIRTUAL_SLAVES) return 1;
  //printf("%d + %d < %d + %d (i=%d)\n", VSlave[i].start, VSlave[i].length, start_address, length, i);
  if((VSlave[i].start+VSlave[i].length)<(start_address+length)) return 2;

	*port_id=VSlave[i].iface;
	*device_id=i; // индекс блока адресов виртуального устройства
	
  return 0;
  }

int translateProxyDevice(int start_address, int length, int *port_id, int *device_id)
  {
	int i;

  for(i=0; i<MAX_QUERY_ENTRIES; i++) {
    if(
			(PQuery[i].length==0) ||
			(PQuery[i].mbf==0) ||
			(PQuery[i].device==0)
			) continue;

			if((start_address>=PQuery[i].offset)&&(start_address<(PQuery[i].offset+PQuery[i].length)))
				break;
			}
	if(i==MAX_VIRTUAL_SLAVES) return 1;
  //printf("%d + %d < %d + %d (i=%d)\n", VSlave[i].start, VSlave[i].length, start_address, length, i);
  if((PQuery[i].offset+PQuery[i].length)<(start_address+length)) return 2;

	*port_id=PQuery[i].iface;
	*device_id=i; // индекс блока адресов виртуального устройства
	
  return 0;
  }

///-----------------------------------------------------------------------------
///-----------------------------------------------------------------------------
///-----------------------------------------------------------------------------
///-----------------------------------------------------------------------------
///-----------------------------------------------------------------------------
