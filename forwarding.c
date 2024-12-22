/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///*********  МОДУЛЬ ФУНКЦИЙ ПЕРЕНАПРАВЛЕНИЯ, ОЧЕРЕДЕЙ ЗАПРОСОВ MODBUS *********

///=== FRWD_QUEUE_H IMPLEMENTATION

#include <string.h>

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

///=== FRWD_QUEUE_H public variables

// Набор семафоров. Используется для синхронизации работы потоков с очередями.
int semaphore_id;

GW_AddressMap_Entry AddressMap[MODBUS_ADDRESS_MAX+1]; // нумерация с единицы
GW_VSlave_Entry      VSlave[MAX_VIRTUAL_SLAVES];
GW_ProxyQuery_Entry   PQuery[MAX_QUERY_ENTRIES];
GW_Exception        Exception[MOXAGATE_EXCEPTIONS_NUMBER];

unsigned short vsmem_offset1xStatus, vsmem_offset2xStatus, vsmem_offset3xRegisters, vsmem_offset4xRegisters;
unsigned short vsmem_amount1xStatus, vsmem_amount2xStatus, vsmem_amount3xRegisters, vsmem_amount4xRegisters;
unsigned short vsmem_used1xStatus, vsmem_used2xStatus, vsmem_used3xRegisters, vsmem_used4xRegisters;

// массив исключительных ситуаций служит для устранения проблем при обмене, вызванных особенностями
// конечных устройств modbus-slave. он содержит набор флагов, включающих определенные алгоритмы в
// определенных ситуациях.

// obsolete
// исключение для СКС-07, параметр - битовый массив номеров последовательных портов, к которым подключены диогены
unsigned int exceptions; // obsolete // массив из 16 флагов
unsigned int except_prm[16]; // obsolete // параметр исключения

///=== FRWD_QUEUE_H private variables

///=== FRWD_QUEUE_H private functions

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

  VSlave[index].critical=0;
  VSlave[index].err_counter=0;
	VSlave[index].status_bit=0;

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
  if((unsigned int)(
    VSlave[index].offset +
    VSlave[index].start +
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
			PQuery[index].mbf==MBF_READ_COILS               ||
			PQuery[index].mbf==MBF_READ_DECRETE_INPUTS      ||
			PQuery[index].mbf==MBF_READ_HOLDING_REGISTERS   ||
			PQuery[index].mbf==MBF_WRITE_MULTIPLE_REGISTERS ||
			PQuery[index].mbf==MBF_READ_INPUT_REGISTERS
		)) return PQUERY_MBTABL;

	if(!(
			PQuery[index].access==QT_ACCESS_READWRITE ||
			PQuery[index].access==QT_ACCESS_READONLY  ||
			PQuery[index].access==QT_ACCESS_WRITEONLY ||
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
  if( (PQuery[index].mbf==MBF_WRITE_MULTIPLE_REGISTERS) && (
      (PQuery[index].length < MBF_0x10_MIN_QUANTITY) ||
      (PQuery[index].length > MBF_0x10_MAX_QUANTITY)
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
int init_sem_set()
	{
	key_t sem_key=ftok("/tmp/app", 'b');
	
	union semun sems;
	unsigned short values[1];

	if((semaphore_id = semget(sem_key, GATEWAY_ASSETS, IPC_CREAT|IPC_EXCL|0666)) == -1) {
		sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SYSTEM, SEMAPHORE_SET_EXISTS, 0, 0, 0, 0);
		return 1;
	 	}
	
	values[0]=0;
	sems.array = values;
	/* Инициализируем все элементы одним значением */
	semctl(semaphore_id, 0, SETALL, sems);
	
//		printf("maximal semaphore amount %d\n", SEMMSL);
	return 0;
	}
///-----------------------------------------------------------------------------------------------------------------
int init_queue(GW_Queue *queue, int port)
  {
  queue->port_id=port;

  memset(queue->queue_adu, 0, sizeof(queue->queue_adu));
  memset(queue->queue_adu_len, 0, sizeof(queue->queue_adu_len));

  memset(queue->queue_clients, 0, sizeof(queue->queue_clients));
  memset(queue->queue_slaves, 0, sizeof(queue->queue_slaves));

  queue->queue_start = queue->queue_len = 0;

  pthread_mutex_init(&queue->queue_mutex, NULL);

  queue->operations[0].sem_num=port;
  queue->operations[0].sem_flg=0;

  return 0;
  }
///-----------------------------------------------------------------------------------------------------------------
int enqueue_query_ex(GW_Queue *queue, int client_id, int context, u8 *adu, u16 adu_len)
  {
	struct sembuf operations[1];

	int j, queue_current=(queue->queue_start+queue->queue_len)%MAX_GATEWAY_QUEUE_LENGTH;

  if(queue->queue_len==MAX_GATEWAY_QUEUE_LENGTH) { ///!!! modbus exception response, reset queue
 		sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_WRN|queue->port_id, POLL_QUEUE_OVERL, client_id, 0, 0, 0);
		return 1;
		}

	pthread_mutex_lock(&queue->queue_mutex);

	for(j=0; j<adu_len; j++) {
		queue->queue_adu[queue_current][j]=adu[j];
		}
	queue->queue_adu_len[queue_current]=adu_len;
	queue->queue_clients[queue_current]=client_id;
	queue->queue_slaves[queue_current]=context;

	queue->queue_len++;
	
	pthread_mutex_unlock(&queue->queue_mutex);

//	sysmsg_ex(EVENT_CAT_TRAFFIC|EVENT_TYPE_INF|GATEWAY_FRWD, 220, queue->port_id, client_id, queue->queue_len, 0);

	// структуру queue->operations нельзя использовать здесь
	operations[0].sem_op=1;
	operations[0].sem_num=queue->port_id;
	operations[0].sem_flg=queue->operations[0].sem_flg;
	semop(semaphore_id, operations, 1);

  return 0;
  }
///-----------------------------------------------------------------------------------------------------------------
int get_query_from_queue(GW_Queue *queue, int *client_id, int *context, u8 *adu, u16 *adu_len)
  {
	int status;
	struct sembuf operations[1];

	int j, queue_current=queue->queue_start;

	// структуру queue->operations нельзя использовать здесь
	operations[0].sem_op=-1;
	operations[0].sem_num=queue->port_id;
	operations[0].sem_flg=queue->operations[0].sem_flg;
	status=semop(semaphore_id, operations, 1);
				 
	if(status==-1) return status;

  if(queue->queue_len==0) { /// внутренняя ошибка в программе
 		sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_WRN|queue->port_id, POLL_QUEUE_EMPTY, 0, 0, 0, 0);
		return 1;
		}

 	pthread_mutex_lock(&queue->queue_mutex);

	for(j=0; j<queue->queue_adu_len[queue_current]; j++) {
		adu[j]=queue->queue_adu[queue_current][j];
		}
	*adu_len=queue->queue_adu_len[queue_current];
	*client_id=queue->queue_clients[queue_current];
	*context=queue->queue_slaves[queue_current]; ///!!! ATM & PROXY uses this field


	queue->queue_start=(queue->queue_start+1)%MAX_GATEWAY_QUEUE_LENGTH;
	queue->queue_len--;

//	sysmsg_ex(EVENT_CAT_TRAFFIC|EVENT_TYPE_INF|GATEWAY_FRWD, 221, queue->port_id, *client_id, queue->queue_len, 0);

	pthread_mutex_unlock(&queue->queue_mutex);

  return 0;
  }
///-----------------------------------------------------------------------------------------------------------------
int checkDiapason(int function, int start_address, int length)
	{
	unsigned int internal_start, internal_end;

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
		if(start_address + length <= internal_end) return FRWD_TYPE_PROXY;
		if(start_address >= internal_end) return FRWD_TYPE_REGISTER;
		return FRWD_RESULT_OVERLAPPED;
		} else {
			if(start_address + length <= internal_start) return FRWD_TYPE_REGISTER;
			return FRWD_RESULT_OVERLAPPED;
			}

	return FRWD_RESULT_UNDEFINED;
	}
///-----------------------------------------------------------------------------------------------------------------

int translateRegisters(int start_address, int length, int *port_id, int *device_id)
  {
	int i;

  for(i=0; i<MAX_VIRTUAL_SLAVES; i++) {
    if(VSlave[i].iface==GATEWAY_NONE) continue;
		if((start_address>=VSlave[i].start)&&(start_address<(VSlave[i].start+VSlave[i].length))) break;
		}
	if(i==MAX_VIRTUAL_SLAVES) return FRWD_RESULT_UNDEFINED;
  //printf("%d + %d < %d + %d (i=%d)\n", VSlave[i].start, VSlave[i].length, start_address, length, i);
  if((VSlave[i].start+VSlave[i].length)<(start_address+length)) return FRWD_RESULT_OVERLAPPED;

	*port_id=VSlave[i].iface;
	*device_id=i; // индекс блока адресов виртуального устройства
	
  return FRWD_TYPE_REGISTER;
  }

int translateProxyDevice(int start_address, int length, int *port_id, int *device_id)
  {
	int i;

  for(i=0; i<MAX_QUERY_ENTRIES; i++) {
    if(PQuery[i].iface==GATEWAY_NONE) continue;
		if((start_address>=PQuery[i].offset)&&(start_address<(PQuery[i].offset+PQuery[i].length))) break;
		}
	if(i==MAX_QUERY_ENTRIES) return FRWD_RESULT_UNDEFINED;
  //printf("%d + %d < %d + %d (i=%d)\n", VSlave[i].start, VSlave[i].length, start_address, length, i);
  if((PQuery[i].offset+PQuery[i].length)<(start_address+length)) return FRWD_RESULT_OVERLAPPED;

	*port_id=PQuery[i].iface;
	*device_id=i; // индекс записи таблицы опроса
	
  return FRWD_TYPE_PROXY;
  }

/*//-----------------------------------------------------------------------------
   разбор входящего запроса происходит в несколько этапов:
	- проверяем modbus адрес, указанный в запросе;
	- проверяем диапазон регистров, указанный в запросе;
	- если и адрес и диапазон принадлежат устройству Moxa, ставим запрос в очередь внутренних запросов;
	- если только адрес принадлежит устройству Moxa, используем механизм трансляции регистров;
	- если адрес не принадлежит устройству Moxa, используем механизм тарнсляции адресов.
 */
int forward_query(int client_id, u8 *tcp_adu, u16 tcp_adu_len)
  {
	int j, k, i;
	int port_id, device_id;
	int status;
	GW_Iface *iface;

	k=tcp_adu[TCPADU_ADDRESS]==MoxaDevice.modbus_address?FRWD_RESULT_UNDEFINED:FRWD_TYPE_ADDRESS;

			if(k!=FRWD_TYPE_ADDRESS) {

			switch(tcp_adu[TCPADU_FUNCTION]) { 	// определяем диапазон регистров, требуемых в запросе,
																					// а также фильтруем входящие запросы по функциям ModBus
				case MBF_READ_COILS:
				case MBF_READ_DECRETE_INPUTS:
				case MBF_READ_HOLDING_REGISTERS:
				case MBF_READ_INPUT_REGISTERS:
		
				case MBF_WRITE_MULTIPLE_COILS:
				case MBF_WRITE_MULTIPLE_REGISTERS:
		
					j=(tcp_adu[TCPADU_LEN_HI]<<8)|tcp_adu[TCPADU_LEN_LO];
					break;

				case MBF_WRITE_SINGLE_COIL:
				case MBF_WRITE_SINGLE_REGISTER:
					j=1;
					break;
		
				default: ///функции, не поддерживаемые шлюзом

			 		sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_WRN|GATEWAY_SECURITY, POLL_WRMBFUNC, (unsigned) tcp_adu[TCPADU_FUNCTION], client_id, 0, 0);

					return FRWD_RESULT_UNSUP_FUNC+FRWD_TYPE_PROXY;
				  }
	
				k=checkDiapason( tcp_adu[TCPADU_FUNCTION],
												(tcp_adu[TCPADU_START_HI]<<8)|tcp_adu[TCPADU_START_LO],
												 j);
				switch(k) {
	
					case 	FRWD_TYPE_PROXY:
						/// ставим запрос в очередь MOXA MODBUS DEVICE, если клиентом является GW_CLIENT_TCP_502
						if(Client[client_id].status==GW_CLIENT_TCP_502) {
						  status=enqueue_query_ex(&MoxaDevice.queue, client_id, device_id, tcp_adu, tcp_adu_len);
						  if(status!=0) return FRWD_RESULT_QUEUE_FAIL+k;
						  }
						break;
					
					case FRWD_TYPE_REGISTER:
	
						status=translateRegisters(
							(tcp_adu[TCPADU_START_HI]<<8)|tcp_adu[TCPADU_START_LO],
							j, &port_id, &device_id);

					  if(status!=FRWD_TYPE_REGISTER) {

			 			  sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_WRN|GATEWAY_SECURITY, FRWD_TRANS_VSLAVE, client_id, (unsigned) (tcp_adu[TCPADU_START_HI]<<8)|tcp_adu[TCPADU_START_LO], (unsigned) j, 0);

							return status+k;
							}

						/// ставим запрос в очередь MASTER-интерфейса
						i = port_id-GATEWAY_T01;
						iface= port_id<=GATEWAY_P8? &IfaceRTU[port_id]: &IfaceTCP[i];
						if((status=enqueue_query_ex(&iface->queue, client_id, (FRWD_TYPE_REGISTER<<8)|(device_id&0xff), tcp_adu, tcp_adu_len))!=0) return FRWD_RESULT_QUEUE_FAIL+k;
						break;

					case FRWD_RESULT_OVERLAPPED:

			 			sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_WRN|GATEWAY_SECURITY, FRWD_TRANS_OVERLAP, client_id, (unsigned) (tcp_adu[TCPADU_START_HI]<<8)|tcp_adu[TCPADU_START_LO], (unsigned) j, 0);
	
					default: // FRWD_RESULT_UNDEFINED

					  return k+FRWD_TYPE_PROXY;
					}
				} else { /// обработка запроса в режиме трансляции адресов

					status=0;
					iface=NULL;
					device_id=tcp_adu[TCPADU_ADDRESS];
					if(AddressMap[device_id].iface!=GATEWAY_NONE) {
					  port_id=AddressMap[device_id].iface;
					  i = port_id-GATEWAY_T01;
					  iface= port_id<=GATEWAY_P8? &IfaceRTU[port_id]: &IfaceTCP[i];
					  }
						
					if(iface!=NULL) status=
					  (iface->modbus_mode==IFACE_RTUMASTER) ||  (iface->modbus_mode==IFACE_TCPMASTER)?
					  1:0;
				  		
				  if(status==0) {

			 			sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_WRN|GATEWAY_SECURITY, FRWD_TRANS_ADDRESS, (unsigned) tcp_adu[TCPADU_ADDRESS], client_id, 0, 0);

						return FRWD_RESULT_UNDEFINED+k;
						}

					/// ставим запрос в очередь MASTER-интерфейса
					if((status=enqueue_query_ex(&iface->queue, client_id, (FRWD_TYPE_ADDRESS<<8)|(device_id&0xff), tcp_adu, tcp_adu_len))!=0) return FRWD_RESULT_QUEUE_FAIL+k;
					}

	return k;
  }
///-----------------------------------------------------------------------------
void prepare_request (int context, u8 *tcp_adu, u16 tcp_adu_len)
  {
	int j, device_id;

	device_id=context&0xff;

	switch((context>>8)&0xff) {

		// для всех поддерживаемых функций modbus процедура подготовки
		// запроса для отправки на сервер одинакова и включает либо
		// изменение стартового регистра, либо адреса устройства.

		case FRWD_TYPE_PROXY:

				tcp_adu[TCPADU_ADDRESS]=PQuery[device_id].device;
				j=	(((tcp_adu[TCPADU_START_HI]<<8) | tcp_adu[TCPADU_START_LO])&0xffff)-
						PQuery[device_id].offset+
						PQuery[device_id].start;
				tcp_adu[TCPADU_START_HI]=(j>>8)&0xff;
				tcp_adu[TCPADU_START_LO]=j&0xff;

			break;

		case FRWD_TYPE_REGISTER:
			tcp_adu[TCPADU_ADDRESS]=VSlave[device_id].device;
			j=	(((tcp_adu[TCPADU_START_HI]<<8) | tcp_adu[TCPADU_START_LO])&0xffff)-
					VSlave[device_id].start+
					VSlave[device_id].offset;
			tcp_adu[TCPADU_START_HI]=(j>>8)&0xff;
			tcp_adu[TCPADU_START_LO]=j&0xff;
			break;

		case FRWD_TYPE_ADDRESS:
			tcp_adu[TCPADU_ADDRESS]=AddressMap[device_id].address;
			break;

		default:;
	  }

	return;
  }
///-----------------------------------------------------------------------------
void prepare_response(int context, u8 *tcp_adu, u16 tcp_adu_len)
  {
	int j, device_id;

	device_id=context&0xff;

	switch((context>>8)&0xff) {

		case FRWD_TYPE_PROXY:

			switch(tcp_adu[TCPADU_FUNCTION]) {
				
				// ответы на функции чтения в режиме FRWD_TYPE_PROXY шлюз выдает
				// самостоятельно без перенаправления на master-интерфейсы
				case MBF_READ_COILS:
				case MBF_READ_DECRETE_INPUTS:
				case MBF_READ_HOLDING_REGISTERS:
				case MBF_READ_INPUT_REGISTERS:

				tcp_adu[TCPADU_ADDRESS]=MoxaDevice.modbus_address;

				default : break;

				case MBF_WRITE_SINGLE_COIL:
				case MBF_WRITE_SINGLE_REGISTER:
				case MBF_WRITE_MULTIPLE_COILS:
				case MBF_WRITE_MULTIPLE_REGISTERS:

				tcp_adu[TCPADU_ADDRESS]=MoxaDevice.modbus_address;

				// исключение изменять не требуется
				if((tcp_adu[TCPADU_FUNCTION]&0x80)!=0) break;

				j=	(((tcp_adu[TCPADU_START_HI]<<8) | tcp_adu[TCPADU_START_LO])&0xffff)+
						PQuery[device_id].offset-
						PQuery[device_id].start;
				tcp_adu[TCPADU_START_HI]=(j>>8)&0xff;
				tcp_adu[TCPADU_START_LO]=j&0xff;
				}

			break;

		case FRWD_TYPE_REGISTER:

			switch(tcp_adu[TCPADU_FUNCTION]) {
				case MBF_READ_COILS:
				case MBF_READ_DECRETE_INPUTS:
				case MBF_READ_HOLDING_REGISTERS:
				case MBF_READ_INPUT_REGISTERS:

				tcp_adu[TCPADU_ADDRESS]=MoxaDevice.modbus_address;

				default : break;

				case MBF_WRITE_SINGLE_COIL:
				case MBF_WRITE_SINGLE_REGISTER:
				case MBF_WRITE_MULTIPLE_COILS:
				case MBF_WRITE_MULTIPLE_REGISTERS:

				tcp_adu[TCPADU_ADDRESS]=MoxaDevice.modbus_address;

				// исключение изменять не требуется
				if((tcp_adu[TCPADU_FUNCTION]&0x80)!=0) break;

				j=	(((tcp_adu[TCPADU_START_HI]<<8) | tcp_adu[TCPADU_START_LO])&0xffff)+
						VSlave[device_id].start-
						VSlave[device_id].offset;
				tcp_adu[TCPADU_START_HI]=(j>>8)&0xff;
				tcp_adu[TCPADU_START_LO]=j&0xff;
				}

			break;

		case FRWD_TYPE_ADDRESS:
			tcp_adu[TCPADU_ADDRESS]=device_id;
			break;

		default:;
	  }

	return;
  }
/*//-----------------------------------------------------------------------------

///---------- специальный случай при подаче команд на СКС-7 Диоген, обрабатываем
///--- убираем третий с конца байт в полученном ответе на запрос
if((exceptions&EXPT_ACT_SKS07_DIOGEN)!=0)					
		if(serial_adu[RTUADU_FUNCTION]==0x06)
		if((except_prm[0]&(1 << port_id))!=0) {
			serial_adu[serial_adu_len-3]=serial_adu[serial_adu_len-2];
			serial_adu[serial_adu_len-2]=serial_adu[serial_adu_len-1];
			serial_adu_len--;
			//if(status==MB_SERIAL_PDU_ERR) status=0;
			status=0;
			}
///-----------------------------------------------------------------------------

*///-----------------------------------------------------------------------------
