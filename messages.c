/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///******************* МОДУЛЬ ЖУРНАЛА СООБЩЕНИЙ ********************************

///=== MESSAGES_H IMPLEMENTATION

#include <pthread.h>

#include "messages.h"
#include "interfaces.h"
#include "moxagate.h"
#include "cli.h"
#include "hmi_klb.h"
#include "hmi_web.h"

///=== MESSAGES_H private variables

char eventmsg[EVENT_MESSAGE_LENGTH+EVENT_MESSAGE_PREFIX];

time_t	curtime;
struct tm *tmd;

///=== MESSAGES_H private functions

int init_message_templates();

//******************************************************************************
int init_messages_h()
  {
  int i;	 

  EventLog.app_log_current_entry=0;
  EventLog.app_log_entries_total=0;
  EventLog.app_log=NULL;
  EventLog.msg_filter=0xffffffff;

  memset(EventLog.cat_msgs_amount,  0, sizeof(EventLog.cat_msgs_amount ));
  memset(EventLog.type_msgs_amount, 0, sizeof(EventLog.type_msgs_amount));

  i = sizeof(char) * EVENT_TEMPLATE_AMOUNT * EVENT_MESSAGE_LENGTH;
  EventLog.msg_tpl=(char *) malloc(i);
  if(EventLog.msg_tpl==NULL) return 1;

  i = sizeof(unsigned int) * EVENT_TEMPLATE_AMOUNT;
  EventLog.msg_index=(unsigned int *) malloc(i);
  if(EventLog.msg_index==NULL) return 2;

  // первая инициализация массива (до инициализации разделяемого сегмента памяти)
  i=init_message_templates();
  if(i!=0) return 4;

  return 0;
  }

///----------------------------------------------------------------------------
int init_message_templates()
  {
	memset(EventLog.msg_tpl,         0, sizeof(char) * EVENT_TEMPLATE_AMOUNT * EVENT_MESSAGE_LENGTH);
	memset(EventLog.msg_index,       0, sizeof(unsigned int) * EVENT_TEMPLATE_AMOUNT);

/// проверка для тестовой компиляции на отсуствие косяков при присвоении значений константам,
/// для генерации тестового кода нужно обработать код этой функции при помощи регулярных выражений,
/// что обеспечит отсуствие ошибок ручного ввода.
/// значения, выданные тестовым кодом, должны быть: 1. уникальными; 2. меньше константы EVENT_TEMPLATE_AMOUNT
/// значения, выданные тестовым кодом, лучше получить путем включения записи сеанса в лог-файл telnet-клиента
/// последовательность регулярных выражений, необходимая для генерации тестового кода:
/// \n\n = \n
/// /^(.*msg_tpl.*)$/mi = #\1
/// /^[^#].*$/mi = ""
/// \n\n = \n
/// printf("EVENT_TEMPLATE_AMOUNT\\t%d\\n", EVENT_TEMPLATE_AMOUNT);
/// /^.*A2D.([^\s]+).*$/mi = printf("\1\\t%d\\t%d\\n", \1, EVENT_TEMPLATE_AMOUNT-\1);

///...

///exit(1);

/// COMAND LINE (КОМАНДНАЯ СТРОКА) [XX..XX, XX]

strcpy(&EventLog.msg_tpl[A2D(COMMAND_LINE_OK        )], "CLI: COMMAND LINE PARSED SUCCESSFULLY");
strcpy(&EventLog.msg_tpl[A2D(COMMAND_LINE_ERROR     )], "CLI: WHERE IS %d PARSING ERROR(S)");
strcpy(&EventLog.msg_tpl[A2D(COMMAND_LINE_INFO      )], "CLI: SYSINFO DISPLAYED AND EXIT");
strcpy(&EventLog.msg_tpl[A2D(COMMAND_LINE_ARGC      )], "CLI: WHERE IS UNKNOWN PARAMETER(S)");
strcpy(&EventLog.msg_tpl[A2D(COMMAND_LINE_UNDEFINED )], "CLI: COMMAND LINE UNDEFINED RESULT");

strcpy(&EventLog.msg_tpl[A2D(SECURITY_CONF_STRUCT   )], "CLI: SECURITY STRUCTURE");
strcpy(&EventLog.msg_tpl[A2D(SECURITY_CONF_DUPLICATE)], "CLI: SECURITY PARAM DUPLICATED");
strcpy(&EventLog.msg_tpl[A2D(SECURITY_CONF_SPELLING )], "CLI: SECURITY PARAM UNKNOWN");

strcpy(&EventLog.msg_tpl[A2D(IFACE_CONF_RTUDUPLICATE)], "CLI: IFACERTU %s DUPLICATED");
					EventLog.msg_index[IFACE_CONF_RTUDUPLICATE]|= EVENT_STR_IFACE;
strcpy(&EventLog.msg_tpl[A2D(IFACE_CONF_RTUSTRUCT   )], "CLI: IFACERTU %s STRUCTURE");
					EventLog.msg_index[IFACE_CONF_RTUSTRUCT   ]|= EVENT_STR_IFACE;
strcpy(&EventLog.msg_tpl[A2D(IFACE_CONF_GWMODE      )], "CLI: IFACE %s GATEWAY MODE");
					EventLog.msg_index[IFACE_CONF_GWMODE      ]|= EVENT_STR_IFACE;

strcpy(&EventLog.msg_tpl[A2D(IFACE_CONF_TCPDUPLICATE)], "CLI: IFACETCP %s DUPLICATED");
					EventLog.msg_index[IFACE_CONF_TCPDUPLICATE]|= EVENT_STR_IFACE;
strcpy(&EventLog.msg_tpl[A2D(IFACE_CONF_TCPSTRUCT   )], "CLI: IFACETCP %s STRUCTURE");
					EventLog.msg_index[IFACE_CONF_TCPSTRUCT   ]|= EVENT_STR_IFACE;

strcpy(&EventLog.msg_tpl[A2D(ATM_CONF_SPELLING      )], "CLI: ADDRESS MAP SPELLING");
strcpy(&EventLog.msg_tpl[A2D(ATM_CONF_STRUCT        )], "CLI: ADDRESS MAP STRUCTURE");

strcpy(&EventLog.msg_tpl[A2D(VSLAVE_CONF_OVERFLOW   )], "CLI: VSLAVES OVERFLOW");
strcpy(&EventLog.msg_tpl[A2D(VSLAVE_CONF_STRUCT     )], "CLI: VSLAVES STRUCTURE");
strcpy(&EventLog.msg_tpl[A2D(VSLAVE_CONF_IFACE      )], "CLI: VSLAVES IFACE");

strcpy(&EventLog.msg_tpl[A2D(PQUERY_CONF_OVERFLOW   )], "CLI: PQUERIES OVERFLOW");
strcpy(&EventLog.msg_tpl[A2D(PQUERY_CONF_STRUCT     )], "CLI: PQUERIES STRUCTURE");
strcpy(&EventLog.msg_tpl[A2D(PQUERY_CONF_IFACE      )], "CLI: PQUERIES IFACE");

strcpy(&EventLog.msg_tpl[A2D(EXPT_CONF_OVERFLOW     )], "CLI: EXCEPTIONS OVERFLOW");
strcpy(&EventLog.msg_tpl[A2D(EXPT_CONF_STRUCT       )], "CLI: EXCEPTIONS STRUCTURE");
strcpy(&EventLog.msg_tpl[A2D(EXPT_CONF_STAGE        )], "CLI: EXCEPTIONS STAGE");

/// SECURITY (ПОДСИСТЕМА РАБОТЫ С КЛИЕНТАМИ) [XX..XX, XX]

strcpy(&EventLog.msg_tpl[A2D(SECURITY_TCPPORT )], "CONFIGURATION TCP PORT");
strcpy(&EventLog.msg_tpl[A2D(CLIENT_NOTAVAIL )],  "NO EMPTY SLOT FOR %d");

	/// MOXAGATE (ШЛЮЗ) [XX..XX, XX]

strcpy(&EventLog.msg_tpl[A2D(MOXAGATE_MBADDR  )], "CONFIGURATION MBADDR");
strcpy(&EventLog.msg_tpl[A2D(MOXAGATE_STATINFO)], "CONFIGURATION STATINFO");

strcpy(&EventLog.msg_tpl[A2D(MOXAGATE_STATINFO_OVERLAPS)], 						"INI: MOXAGATE STATINFO OVERLAPS");
strcpy(&EventLog.msg_tpl[A2D(MOXAGATE_MBTABLES_OVERLAPS)], 						"INI: MOXAGATE MBTABLES 0%dx OVERLAPS");
strcpy(&EventLog.msg_tpl[A2D(MOXAGATE_MBTABLE_ALLOCATED)], 						"INI: MEMORY ALLOCATED 0%dx:%db");

strcpy(&EventLog.msg_tpl[A2D(MOXAGATE_WATCHDOG_STARTED )], "WATCH-DOG TIMER STARTED");

/// IFACES (ИНТЕРФЕЙСЫ) [XX..XX, XX]

strcpy(&EventLog.msg_tpl[A2D(IFACE_MBMODE     )], "CONFIGURATION IFACE GATEWAY MODE");

strcpy(&EventLog.msg_tpl[A2D(IFACE_RTUPHYSPROT)], "CONFIGURATION IFACERTU PHYSPROTO");
strcpy(&EventLog.msg_tpl[A2D(IFACE_RTUSPEED   )], "CONFIGURATION IFACERTU SPEED");
strcpy(&EventLog.msg_tpl[A2D(IFACE_RTUPARITY  )], "CONFIGURATION IFACERTU PARITY");
strcpy(&EventLog.msg_tpl[A2D(IFACE_RTUTIMEOUT )], "CONFIGURATION IFACERTU TIMEOUT");
strcpy(&EventLog.msg_tpl[A2D(IFACE_RTUTCPPORT )], "CONFIGURATION IFACERTU TCP PORT");

strcpy(&EventLog.msg_tpl[A2D(IFACE_TCPIP1     )], "CONFIGURATION IFACETCP IP ADDRESS");
strcpy(&EventLog.msg_tpl[A2D(IFACE_TCPPORT1   )], "CONFIGURATION IFACETCP TCP PORT");
strcpy(&EventLog.msg_tpl[A2D(IFACE_TCPUNITID  )], "CONFIGURATION IFACETCP UNIT ID");
strcpy(&EventLog.msg_tpl[A2D(IFACE_TCPOFFSET  )], "CONFIGURATION IFACETCP OFFSET");
strcpy(&EventLog.msg_tpl[A2D(IFACE_RTUTIMEOUT )], "CONFIGURATION IFACERTU TIMEOUT");
strcpy(&EventLog.msg_tpl[A2D(IFACE_TCPIP2     )], "CONFIGURATION IFACETCP IP ADDRESS 2");
strcpy(&EventLog.msg_tpl[A2D(IFACE_TCPPORT2   )], "CONFIGURATION IFACETCP TCP PORT 2");
strcpy(&EventLog.msg_tpl[A2D(IFACE_TCPIPEQUAL )], "CONFIGURATION IFACETCP IP EQUALS");

strcpy(&EventLog.msg_tpl[A2D(IFACE_TTYINIT)], 						"SERIAL PORT INITIALIZED MODE %d");
strcpy(&EventLog.msg_tpl[A2D(IFACE_THREAD_INIT)], 				"THREAD INITIALIZED CODE %d");
strcpy(&EventLog.msg_tpl[A2D(IFACE_THREAD_STARTED)], 			"THREAD STARTED MODE %d CLIENT %d");
strcpy(&EventLog.msg_tpl[A2D(IFACE_THREAD_STOPPED)], 			"THREAD STOPPED");
strcpy(&EventLog.msg_tpl[A2D(PROGRAM_TERMINATED)], 				"PROGRAM TERMINATED (WORKTIME %d)");

/// HMI (Человеко-машинный интерфейс) [XX..XX, XX]
strcpy(&EventLog.msg_tpl[A2D(HMI_KLB_INIT_KEYPAD)], "KEYPAD INITIALIZATION");
strcpy(&EventLog.msg_tpl[A2D(HMI_KLB_INIT_LCM   )], "LCM INITIALIZATION");
strcpy(&EventLog.msg_tpl[A2D(HMI_KLB_INIT_BUZZER)], "BUZZER INITIALIZATION");
strcpy(&EventLog.msg_tpl[A2D(HMI_KLB_INIT_THREAD)], "HMI THREAD INITIALIZATION");

strcpy(&EventLog.msg_tpl[A2D(HMI_WEB_ENOENT )], "SHARED MEM: ENOENT");
strcpy(&EventLog.msg_tpl[A2D(HMI_WEB_EACCES )], "SHARED MEM: EACCES");
strcpy(&EventLog.msg_tpl[A2D(HMI_WEB_EINVAL )], "SHARED MEM: EINVAL");
strcpy(&EventLog.msg_tpl[A2D(HMI_WEB_ENOMEM )], "SHARED MEM: ENOMEM");
strcpy(&EventLog.msg_tpl[A2D(HMI_WEB_EEXIST )], "SHARED MEM: EEXIST");
strcpy(&EventLog.msg_tpl[A2D(HMI_WEB_UNKNOWN)], "SHARED MEM: UNKNOWN");
strcpy(&EventLog.msg_tpl[A2D(HMI_WEB_OK     )], "SHARED MEM: OK SIZE %d bytes");
strcpy(&EventLog.msg_tpl[A2D(HMI_WEB_CLOSED )], "SHARED MEM: CLOSED");

/// ПРОВЕРКА КОНФИГУРАЦИИ ШЛЮЗА В ЦЕЛОМ

strcpy(&EventLog.msg_tpl[A2D(CONFIG_TCPPORT_CONFLICT)], 	"INI: TCPPORT CONFLICT %s vs %s");
					EventLog.msg_index[CONFIG_TCPPORT_CONFLICT]|= EVENT_STR_IFACE | (EVENT_STR_IFACE<<8);
strcpy(&EventLog.msg_tpl[A2D(CONFIG_ADDRMAP)], 						"INI: ADDRESS MAP INCORRECT FOR %s, %d ENTRIES");
					EventLog.msg_index[CONFIG_ADDRMAP]|= EVENT_STR_IFACE;
strcpy(&EventLog.msg_tpl[A2D(CONFIG_FORWARDING)], 				"INI: NO FORWARDING FOR %s");
					EventLog.msg_index[CONFIG_FORWARDING]|= EVENT_STR_IFACE;
strcpy(&EventLog.msg_tpl[A2D(CONFIG_GATEWAY)], 						"INI: GATEWAY HAS NO ACTIVE INTERFACES");
strcpy(&EventLog.msg_tpl[A2D(CONFIG_ADDRMAP_INTEGRITY)],	"INI: ADDRESS MAP REF INTEGRITY, ENTRY %d");
strcpy(&EventLog.msg_tpl[A2D(CONFIG_VSLAVES_INTEGRITY_IFACE)], 	"INI: VSLAVES REF IFACE, ENTRY %d");
strcpy(&EventLog.msg_tpl[A2D(CONFIG_VSLAVES_INTEGRITY_DIAP)], 	"INI: VSLAVES REF DIAP, ENTRY %d vs %d");
strcpy(&EventLog.msg_tpl[A2D(CONFIG_PQUERIES_INTEGRITY_IFACE)], "INI: PROXY QUERIES REF IFACE, ENTRY %d");
strcpy(&EventLog.msg_tpl[A2D(CONFIG_PQUERIES_INTEGRITY_DIAP)],	"INI: PROXY QUERIES REF DIAP, ENTRY %d vs %d");

/// CONNECTION (СЕТЕВОЕ СОЕДИНЕНИЕ) [XX..XX, XX]

strcpy(&EventLog.msg_tpl[A2D(TCPCON_INITIALIZED)], 				"SOCKET INITIALIZED");
strcpy(&EventLog.msg_tpl[A2D(TCPCON_ACCEPTED)], 					"CONNECTION ACCEPTED FROM %s CLIENT %d");
					EventLog.msg_index[TCPCON_ACCEPTED]|= EVENT_STR_IPADDR;
strcpy(&EventLog.msg_tpl[A2D(TCPCON_REJECTED)], 					"CONNECTION REJECTED FROM %s");
					EventLog.msg_index[TCPCON_REJECTED]|= EVENT_STR_IPADDR;
strcpy(&EventLog.msg_tpl[A2D(TCPCON_CLOSED_LINK)], 					"CONNECTION CLOSED (LINK DOWN) CLIENT %d");
strcpy(&EventLog.msg_tpl[A2D(TCPCON_CLOSED_TIME)], 					"CONNECTION CLOSED (TIMEOUT) CLIENT %d");
strcpy(&EventLog.msg_tpl[A2D(TCPCON_CLOSED_KPALV)],					"CONNECTION CLOSED (KEEPALIVE) CONN TO %s");
					EventLog.msg_index[TCPCON_CLOSED_KPALV]|= EVENT_STR_IPADDR;
strcpy(&EventLog.msg_tpl[A2D(TCPCON_CLOSED_REMSD)],					"CONNECTION CLOSED (REMOTE SIDE) TO %s");
					EventLog.msg_index[TCPCON_CLOSED_REMSD]|= EVENT_STR_IPADDR;
strcpy(&EventLog.msg_tpl[A2D(TCPCON_CLOSED_TMOUT)],					"CONNECTION CLOSED (TIMEOUT) TO %s");
					EventLog.msg_index[TCPCON_CLOSED_TMOUT]|= EVENT_STR_IPADDR;
strcpy(&EventLog.msg_tpl[A2D(TCPCON_ESTABLISHED)], 					"CONNECTION ESTABLISHED WITH %s");
					EventLog.msg_index[TCPCON_ESTABLISHED]|= EVENT_STR_IPADDR;
strcpy(&EventLog.msg_tpl[A2D(TCPCON_FAILED)], 							"CONNECTION FAILED TO %s");
					EventLog.msg_index[TCPCON_FAILED]|= EVENT_STR_IPADDR;

/// FORWARDING (ПЕРЕНАПРАВЛЕНИЕ) [XX..XX, XX]

strcpy(&EventLog.msg_tpl[A2D(ATM_IFACE         )], "CONFIGURATION ADDRESS MAP #%d IFACE");
strcpy(&EventLog.msg_tpl[A2D(ATM_MBADDR        )], "CONFIGURATION ADDRESS MAP #%d MBADDR");

strcpy(&EventLog.msg_tpl[A2D(VSLAVE_IFACE      )], "CONFIGURATION VSLAVE #%d IFACE");
strcpy(&EventLog.msg_tpl[A2D(VSLAVE_MBADDR     )], "CONFIGURATION VSLAVE #%d MBADDR");
strcpy(&EventLog.msg_tpl[A2D(VSLAVE_MBTABL     )], "CONFIGURATION VSLAVE #%d MBTABLE");
strcpy(&EventLog.msg_tpl[A2D(VSLAVE_BEGDIAP    )], "INCORRECT VSLAVE #%d BEGDIAP");
strcpy(&EventLog.msg_tpl[A2D(VSLAVE_ENDDIAP    )], "INCORRECT VSLAVE #%d ENDDIAP");
strcpy(&EventLog.msg_tpl[A2D(VSLAVE_LENDIAP    )], "INCORRECT VSLAVE #%d LENDIAP");

strcpy(&EventLog.msg_tpl[A2D(PQUERY_IFACE      )], "CONFIGURATION PQUERY #%d IFACE");
strcpy(&EventLog.msg_tpl[A2D(PQUERY_MBADDR     )], "CONFIGURATION PQUERY #%d MBADDR");
strcpy(&EventLog.msg_tpl[A2D(PQUERY_MBTABL     )], "CONFIGURATION PQUERY #%d MBTABLE");
strcpy(&EventLog.msg_tpl[A2D(PQUERY_ACCESS     )], "CONFIGURATION PQUERY #%d ACCESS");
strcpy(&EventLog.msg_tpl[A2D(PQUERY_ENDREGREAD )], "CONFIGURATION PQUERY #%d ENDREAD");
strcpy(&EventLog.msg_tpl[A2D(PQUERY_LENPACKET  )], "CONFIGURATION PQUERY #%d PACKET LENGTH");
strcpy(&EventLog.msg_tpl[A2D(PQUERY_ENDREGWRITE)], "CONFIGURATION PQUERY #%d ENDWRITE");
strcpy(&EventLog.msg_tpl[A2D(PQUERY_DELAYMIN   )], "CONFIGURATION PQUERY #%d DELAY MIN");
strcpy(&EventLog.msg_tpl[A2D(PQUERY_DELAYMAX   )], "CONFIGURATION PQUERY #%d DELAY MAX");
strcpy(&EventLog.msg_tpl[A2D(PQUERY_ERRCNTR    )], "CONFIGURATION PQUERY #%d CRITICAL");

strcpy(&EventLog.msg_tpl[A2D(EXPT_STAGE        )], "CONFIGURATION EXCEPTION #%d STAGE");
strcpy(&EventLog.msg_tpl[A2D(EXPT_ACTION       )], "CONFIGURATION EXCEPTION #%d ACTION");
strcpy(&EventLog.msg_tpl[A2D(EXPT_PRM1         )], "CONFIGURATION EXCEPTION #%d PRM1");
strcpy(&EventLog.msg_tpl[A2D(EXPT_PRM2         )], "CONFIGURATION EXCEPTION #%d PRM2");
strcpy(&EventLog.msg_tpl[A2D(EXPT_PRM3         )], "CONFIGURATION EXCEPTION #%d PRM3");
strcpy(&EventLog.msg_tpl[A2D(EXPT_PRM4         )], "CONFIGURATION EXCEPTION #%d PRM4");

/// POLLING (ОПРОС) [XX..XX, XX]

strcpy(&EventLog.msg_tpl[A2D(POLL_RTU_RECV)], "RTU RECV %s FOR %s");
					EventLog.msg_index[POLL_RTU_RECV]|= EVENT_STR_IOFUNC | (EVENT_STR_CLIENT<<8);
strcpy(&EventLog.msg_tpl[A2D(POLL_TCP_RECV)], "TCP RECV %s FOR %s");
					EventLog.msg_index[POLL_TCP_RECV]|= EVENT_STR_IOFUNC | (EVENT_STR_CLIENT<<8);

strcpy(&EventLog.msg_tpl[A2D(POLL_WRMBFUNC)], "WRONG MBFUNC [%d] FOR %d");

strcpy(&EventLog.msg_tpl[A2D(FRWD_TRANS_ADDRESS)], "FRWD ADDRESS [%d] FOR %d");
strcpy(&EventLog.msg_tpl[A2D(FRWD_TRANS_OVERLAP)], "FRWD OVERLAPPED FOR %d");
strcpy(&EventLog.msg_tpl[A2D(FRWD_TRANS_PQUERY )], "FRWD PROXY FOR %d");
strcpy(&EventLog.msg_tpl[A2D(FRWD_TRANS_VSLAVE )], "FRWD REGISTERS FOR %d");

strcpy(&EventLog.msg_tpl[A2D(POLL_QUEUE_EMPTY)],				 "QUEUE EMPTY");
strcpy(&EventLog.msg_tpl[A2D(POLL_QUEUE_OVERL)],				 "QUEUE OVERLOAD CLIENT %d");

strcpy(&EventLog.msg_tpl[A2D(POLL_RTU_SEND)], "RTU SEND %s FOR %s");
					EventLog.msg_index[POLL_RTU_SEND]|= EVENT_STR_IOFUNC | (EVENT_STR_CLIENT<<8);
strcpy(&EventLog.msg_tpl[A2D(POLL_TCP_SEND)], "TCP SEND %s FOR %s");
					EventLog.msg_index[POLL_TCP_SEND]|= EVENT_STR_IOFUNC | (EVENT_STR_CLIENT<<8);

/// QUEUE (ОЧЕРЕДЬ) [XX..XX, XX]

strcpy(&EventLog.msg_tpl[A2D(SEMAPHORE_SET_EXISTS)], 	"SEMAPHORE SET EXISTS");

/*

/// TRAFFIC (ДАННЫЕ) [220..239, 20]
strcpy(&EventLog.msg_tpl[A2D(220], "CLIENT\tTRAFFIC: QUEUE  IN [%d)]");
strcpy(&EventLog.msg_tpl[A2D(221], "CLIENT\tTRAFFIC: QUEUE OUT [%d)]");
*/

  /// анализируем массив шаблонов сообщений, заполняем массив параметров сообщений

  int i, j, msglen;
  int d, s;

  for(i=0; i<EVENT_TEMPLATE_AMOUNT; i++) {
								 
    msglen=strlen(&EventLog.msg_tpl[A2D(i)]);

    // проверка на длину текста сообщения без учета длины параметров при подстановке
    if(msglen >= EVENT_MESSAGE_LENGTH) return 1;

    if(msglen>0) {
			d=s=0;
      for(j=0; j<msglen; j++)
        if(EventLog.msg_tpl[A2D(i)+j]  =='%')
        if(EventLog.msg_tpl[A2D(i)+j+1]=='d') {EventLog.msg_index[i]|=(EVENT_PRM_DGT<<d); d+=8;}
        else
        if(EventLog.msg_tpl[A2D(i)+j+1]=='s') {EventLog.msg_index[i]|=(EVENT_PRM_STR<<d); d+=8;}
      }
			 
    }

  return 0;
  }

///-----------------------------------------------------------------------------------------------------------------

void sysmsg_ex(unsigned char msgtype, unsigned char msgcode,
								unsigned int prm1,
								unsigned int prm2,
								unsigned int prm3,
								unsigned int prm4)
	{
	
	static int first_message=1;
	
	if(Security.show_sys_messages==0 && (msgtype & EVENT_CAT_MASK)==EVENT_CAT_DEBUG) return;
	//if(Security.show_data_flow==0    && (msgtype & EVENT_CAT_MASK)==EVENT_CAT_TRAFFIC) return;
	
	if(first_message==1) {printf("\n"); first_message=0;}

	/// создаем запись в журнале
	time(&curtime);
	if(EventLog.app_log!=NULL) {
		EventLog.app_log[EventLog.app_log_current_entry].time=curtime;
		EventLog.app_log[EventLog.app_log_current_entry].msgtype=msgtype;
		EventLog.app_log[EventLog.app_log_current_entry].msgcode=msgcode;
		
		EventLog.app_log[EventLog.app_log_current_entry].prm[0]=prm1;
		EventLog.app_log[EventLog.app_log_current_entry].prm[1]=prm2;
		EventLog.app_log[EventLog.app_log_current_entry].prm[2]=prm3;
		EventLog.app_log[EventLog.app_log_current_entry].prm[3]=prm4;
		
		EventLog.app_log_entries_total++;
	  } else printf("!");

	// инкрементируем счетчик кольцевого буфера сообщений
	if(EventLog.app_log!=NULL)
		EventLog.app_log_current_entry=\
			EventLog.app_log_current_entry==EVENT_LOG_LENGTH-1?\
			0:EventLog.app_log_current_entry+1;
	
  EventLog. cat_msgs_amount[ EVENT_CAT_ORD(msgtype & EVENT_CAT_MASK )]++;
  EventLog.type_msgs_amount[EVENT_TYPE_ORD(msgtype & EVENT_TYPE_MASK)]++;
	/// выводим событие на консоль
	
	tmd=gmtime(&curtime);
  //strftime(eventmsg, 16, " %b %y", tmd);
  strftime(eventmsg, 16, " %b", tmd);
	printf("%2.2d%s %2.2d:%2.2d:%2.2d ", tmd->tm_mday,
                                       eventmsg,
                                       tmd->tm_hour,
                                       tmd->tm_min,
                                       tmd->tm_sec);

  get_msgtype_str(msgtype, eventmsg);
	printf("%s ", eventmsg);

	get_msgsrc_str(msgtype, prm4, eventmsg);
	printf("%s\t", eventmsg);

	eventmsg[0]=0;
	make_msgstr(msgcode, eventmsg, prm1, prm2, prm3, prm4);
	printf("%s\n", eventmsg);

	return;
	}

///-----------------------------------------------------------------------------------------------------------------

void show_traffic(int traffic, int port_id, int client_id, u8 *adu, u16 adu_len)
  {
///	if(port_id!=SERIAL_P3) return; ///!!! фильтр по интерфейсам
  if(adu_len>MB_UNIVERSAL_ADU_LEN) return;

  int i;
	char str[8];
	strcpy(str, "[%0.2X]");

	switch(traffic) {
		case TRAFFIC_TCP_RECV: printf("TCP  IN: ");	strcpy(str, "{%0.2X}"); break;
		case TRAFFIC_RTU_SEND: printf("RTU OUT: ");	break;
		case TRAFFIC_RTU_RECV: printf("RTU  IN: ");	break;
		case TRAFFIC_TCP_SEND: printf("TCP OUT: ");	strcpy(str, "{%0.2X}"); break;
		default: printf("show_traffic() error\n");	return;
		}

  for(i=0; i<adu_len; i++) printf(str, adu[i]);
  printf("\n");

	return;
  }

///-----------------------------------------------------------------------------------------------------------------
void make_msgstr(	unsigned char msgcode, char *str,
									unsigned int prm1,
									unsigned int prm2,
									unsigned int prm3,
									unsigned int prm4)
	{
	char prm1str[24];
	char prm2str[24];
	char prm3str[24];
	char prm4str[24];
	int p1, p2, p3, p4;
	
	p1=p2=p3=p4=EVENT_PRM_NO;

//	(EventLog.msg_index[msgcode]>> 0)&EVENT_PRM_MASK
//	(EventLog.msg_index[msgcode]>> 8)&EVENT_PRM_MASK
//	(EventLog.msg_index[msgcode]>>16)&EVENT_PRM_MASK
//	(EventLog.msg_index[msgcode]>>24)&EVENT_PRM_MASK

	switch((EventLog.msg_index[msgcode]>> 0)&EVENT_PRM_MASK) {

		case EVENT_PRM_DGT: p1=EVENT_PRM_DGT; break;

		case EVENT_PRM_STR:
			p1=EVENT_PRM_STR;
			switch((EventLog.msg_index[msgcode]>> 0)&EVENT_STR_MASK) {
				case EVENT_STR_IFACE:  iface2str(prm1str, prm1); break;
				case EVENT_STR_CLIENT: strcpy(prm1str, Client[prm1].device_name); break;
				case EVENT_STR_IOFUNC: iofunc2str(prm1str, prm1); break;
				case EVENT_STR_IPADDR: ipaddr2str(prm1str, prm1); break;
				default:;
			  }
			break;

		default:;
	  }

	switch((EventLog.msg_index[msgcode]>> 8)&EVENT_PRM_MASK) {

		case EVENT_PRM_DGT: p2=EVENT_PRM_DGT; break;

		case EVENT_PRM_STR:
			p2=EVENT_PRM_STR;
			switch((EventLog.msg_index[msgcode]>> 8)&EVENT_STR_MASK) {
				case EVENT_STR_IFACE:  iface2str(prm2str, prm2); break;
				case EVENT_STR_CLIENT: strcpy(prm2str, Client[prm2].device_name); break;
				case EVENT_STR_IOFUNC: iofunc2str(prm2str, prm2); break;
				case EVENT_STR_IPADDR: ipaddr2str(prm2str, prm2); break;
				default:;
			  }
			break;

		default:;
	  }

  // все сообщения без параметров
  if(p1==EVENT_PRM_NO)
		sprintf(str, &EventLog.msg_tpl[A2D(msgcode)]);

  // сообщения c одним (первым) числовым параметром
  if(p1==EVENT_PRM_DGT)
		sprintf(str, &EventLog.msg_tpl[A2D(msgcode)], prm1);

  // сообщения c двумя числовыми параметрами
  if((p1==EVENT_PRM_DGT)&&(p2==EVENT_PRM_DGT))
		sprintf(str, &EventLog.msg_tpl[A2D(msgcode)], prm1, prm2);

  // сообщения c одним (первым) строковым параметром
  if(p1==EVENT_PRM_STR)
		sprintf(str, &EventLog.msg_tpl[A2D(msgcode)], prm1str);

  // сообщения c двумя строковыми параметрами
  if((p1==EVENT_PRM_STR)&&(p2==EVENT_PRM_STR))
		sprintf(str, &EventLog.msg_tpl[A2D(msgcode)], prm1str, prm2str);

  // сообщения c первым строковым и вторым числовым параметром
  if((p1==EVENT_PRM_STR)&&(p2==EVENT_PRM_DGT))
		sprintf(str, &EventLog.msg_tpl[A2D(msgcode)], prm1str, prm2);

  // сообщения c первым числовым и вторым строковым параметром
  if((p1==EVENT_PRM_DGT)&&(p2==EVENT_PRM_STR))
		sprintf(str, &EventLog.msg_tpl[A2D(msgcode)], prm2, prm1str);

	return;
	}

///-----------------------------------------------------------------------------------------------------------------
void get_msgtype_str(unsigned char msgtype, char *str)
  {
	switch(msgtype & EVENT_TYPE_MASK) {
			case EVENT_TYPE_INF: sprintf(str, "INF"); break;
			case EVENT_TYPE_WRN: sprintf(str, "WRN"); break;
			case EVENT_TYPE_ERR: sprintf(str, "ERR"); break;
			case EVENT_TYPE_FTL: sprintf(str, "FTL"); break;
			default: sprintf(str, "***\t");
			}

  return;
  }

///-----------------------------------------------------------------------------------------------------------------
void get_msgsrc_str(unsigned char msgtype, unsigned int prm4, char *str)
  {
	switch(msgtype & EVENT_SRC_MASK) {
			case GATEWAY_P1: 			 sprintf(str, "   PORT1"); 		break;
			case GATEWAY_P2: 			 sprintf(str, "   PORT2"); 		break;
			case GATEWAY_P3: 			 sprintf(str, "   PORT3"); 		break;
			case GATEWAY_P4: 			 sprintf(str, "   PORT4"); 		break;
			case GATEWAY_P5: 			 sprintf(str, "   PORT5"); 		break;
			case GATEWAY_P6: 			 sprintf(str, "   PORT6"); 		break;
			case GATEWAY_P7: 			 sprintf(str, "   PORT7"); 		break;
			case GATEWAY_P8: 			 sprintf(str, "   PORT8"); 		break;

			case GATEWAY_SYSTEM:   sprintf(str, "  SYSTEM"); 	break;
			case GATEWAY_MOXAGATE: sprintf(str, "MOXAGATE"); break;

			case GATEWAY_LANTCP: if((prm4>=GATEWAY_T01) && (prm4<=GATEWAY_T32))
                             sprintf(str, "   TCP%0.2d", prm4-GATEWAY_T01+1);
                             else 
                             sprintf(str, " !LANTCP");
                             break;

			case GATEWAY_SECURITY: sprintf(str, "SECURITY"); 	break;
			case GATEWAY_FRWD:     sprintf(str, " FORWARD"); 	break;
			case GATEWAY_HMI:      sprintf(str, "     HMI"); 	break;

			default: 							 sprintf(str, "    ****");
			}
			
  return;
  }

///-----------------------------------------------------------------------------------------------------------------
void iface2str(char *str, int prm)
	{
	
  // преобразуем числовой параметр в название интерфейса
  strcpy(str, "***");
  if(prm<=GATEWAY_P8) sprintf(str, "PORT%d", prm+1);
  if((prm>=GATEWAY_T01)&&(prm<=GATEWAY_T32))
    sprintf(str, "TCP%0.2d", prm-GATEWAY_T01+1);

	return;
	}

void iofunc2str(char *str, int prm)
	{
	
  // преобразуем числовой параметр в описательную символьную строку
  // ошибки ввода/вывода
  strcpy(str, "***");

  if(prm==MB_SERIAL_WRITE_ERR		) strcpy(str, "ERROR");
  if(prm==MB_SERIAL_READ_FAILURE) strcpy(str, "ERROR");
  if(prm==MB_SERIAL_COM_TIMEOUT	) strcpy(str, "TIMEOUT");

  if(prm==MB_SERIAL_ADU_ERR_MIN	) strcpy(str, "ADU MIN");
  if(prm==MB_SERIAL_ADU_ERR_MAX	) strcpy(str, "ADU MAX");
  if(prm==MB_SERIAL_ADU_ERR_UID	) strcpy(str, "ADU UID");

  if(prm==MB_SERIAL_CRC_ERROR		) strcpy(str, "CRC");
  if(prm==MB_SERIAL_PDU_ERR			) strcpy(str, "PDU");

// modbus TCP
  if(prm==TCP_COM_ERR_SEND			) strcpy(str, "ERROR");
  if(prm==TCP_COM_ERR_NULL			) strcpy(str, "ERROR");
  if(prm==TCP_COM_ERR_TIMEOUT		) strcpy(str, "TIMEOUT");

  if(prm==TCP_ADU_ERR_MIN				) strcpy(str, "ADU MIN");
  if(prm==TCP_ADU_ERR_MAX				) strcpy(str, "ADU MAX");
  if(prm==TCP_ADU_ERR_UID				) strcpy(str, "ADU UID");
  if(prm==TCP_ADU_ERR_PROTOCOL	) strcpy(str, "PROTO");
  if(prm==TCP_ADU_ERR_LEN				) strcpy(str, "ADU LEN");

  if(prm==TCP_PDU_ERR						) strcpy(str, "PDU");

	return;
	}

void ipaddr2str(char *str, int prm)
	{
	
  // преобразуем числовой параметр в название интерфейса
	sprintf(str, " %d.%d.%d.%d",
		(prm >> 24) & 0xff,
		(prm >> 16) & 0xff,
		(prm >>  8) & 0xff,
		 prm        & 0xff
		);

	return;
	}
///-----------------------------------------------------------------------------------------------------------------
