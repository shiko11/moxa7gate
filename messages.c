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
  if(i!=0) return 3;

  return 0;
  }

///----------------------------------------------------------------------------
int init_message_templates()
  {
	memset(EventLog.msg_tpl,   0, sizeof(char) * EVENT_TEMPLATE_AMOUNT * EVENT_MESSAGE_LENGTH);
	memset(EventLog.msg_index, 0, sizeof(unsigned int) * EVENT_TEMPLATE_AMOUNT);

/// проверка для тестовой компиляции на отсуствие косяков при присвоении значений константам,
/// для генерации тестового кода нужно обработать код этой функции при помощи регулярных выражений,
/// что обеспечит отсуствие ошибок ручного ввода.
/// значения, выданные тестовым кодом, должны быть: 1. уникальными; 2. меньше константы EVENT_TEMPLATE_AMOUNT
/// значения, выданные тестовым кодом, лучше получить путем включения записи сеанса в лог-файл telnet-клиента
/// последовательность регулярных выражений, необходимая для генерации тестового кода:
/// \n\n = \n
/// /^(.*message_template.*)$/mi = #\1
/// /^[^#].*$/mi = ""
/// \n\n = \n
/// printf("EVENT_TEMPLATE_AMOUNT\\t%d\\n", EVENT_TEMPLATE_AMOUNT);
/// /^.*?\[(.+)\].*$/mi = printf("\1\\t%d\\t%d\\n", \1, EVENT_TEMPLATE_AMOUNT-\1);

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
strcpy(&EventLog.msg_tpl[A2D(IFACE_CONF_RTUSTRUCT   )], "CLI: IFACERTU %s STRUCTURE");
strcpy(&EventLog.msg_tpl[A2D(IFACE_CONF_GWMODE      )], "CLI: IFACE %s GATEWAY MODE");

strcpy(&EventLog.msg_tpl[A2D(IFACE_CONF_TCPDUPLICATE)], "CLI: IFACETCP %s DUPLICATED");
strcpy(&EventLog.msg_tpl[A2D(IFACE_CONF_TCPSTRUCT   )], "CLI: IFACETCP %s STRUCTURE");

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

	/// MOXAGATE (ШЛЮЗ) [XX..XX, XX]

strcpy(&EventLog.msg_tpl[A2D(MOXAGATE_MBADDR  )], "CONFIGURATION MBADDR");
strcpy(&EventLog.msg_tpl[A2D(MOXAGATE_STATINFO)], "CONFIGURATION STATINFO");

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
strcpy(&EventLog.msg_tpl[A2D(IFACE_TCPMBADDR  )], "CONFIGURATION IFACETCP ATM ADDRESS");
strcpy(&EventLog.msg_tpl[A2D(IFACE_TCPIP2     )], "CONFIGURATION IFACETCP IP ADDRESS 2");
strcpy(&EventLog.msg_tpl[A2D(IFACE_TCPPORT2   )], "CONFIGURATION IFACETCP TCP PORT 2");
strcpy(&EventLog.msg_tpl[A2D(IFACE_TCPIPEQUAL )], "CONFIGURATION IFACETCP IP EQUALS");


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

/*
strcpy(&EventLog.msg_tpl[A2D( 29)], "SEMAPHORE SET EXISTS");
strcpy(&EventLog.msg_tpl[A2D( 28)], "MEMORY ALLOCATED 0%dx:%db");
strcpy(&EventLog.msg_tpl[A2D( 38)], ""); // РЕЗЕРВ
strcpy(&EventLog.msg_tpl[A2D( 39)], "STATUS INFO OVERLAPS");

strcpy(&EventLog.msg_tpl[A2D( 40)], "SERIAL PORT INITIALIZED MODE %s");
strcpy(&EventLog.msg_tpl[A2D( 41)], "THREAD INITIALIZED CODE %d");
strcpy(&EventLog.msg_tpl[A2D( 42)], "THREAD STARTED MODE %s CLIENT %s");
strcpy(&EventLog.msg_tpl[A2D( 43)], "THREAD STOPPED");
strcpy(&EventLog.msg_tpl[A2D( 44)], "PROGRAM TERMINATED (WORKTIME %d)");
	
/// CONNECTION (СЕТЕВОЕ СОЕДИНЕНИЕ) [65..127, 63]
strcpy(&EventLog.msg_tpl[A2D( 65)], "SOCKET INITIALIZED STAGE %d TCPSERVER %s");
strcpy(&EventLog.msg_tpl[A2D( 66)], ""); // РЕЗЕРВ
strcpy(&EventLog.msg_tpl[A2D( 67)], "CONNECTION ACCEPTED FROM %s CLIENT %d");
strcpy(&EventLog.msg_tpl[A2D( 68)], "CONNECTION ESTABLISHED WITH %d.%d.%d.%d");
strcpy(&EventLog.msg_tpl[A2D( 69)], "CONNECTION FAILED TO %d.%d.%d.%d");
strcpy(&EventLog.msg_tpl[A2D( 70)], "CONNECTION REJECTED FROM %d.%d.%d.%d");
strcpy(&EventLog.msg_tpl[A2D( 71)], "CONNECTION CLOSED (LINK DOWN) CLIENT %d");
strcpy(&EventLog.msg_tpl[A2D( 72)], "CONNECTION CLOSED (TIMEOUT) CLIENT %d");
*/
	
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

/*
strcpy(&EventLog.msg_tpl[A2D(128], "CLIENT\tFRWD: ADDRESS [%d)] NOT TRANSLATED");
strcpy(&EventLog.msg_tpl[A2D(129], "CLIENT\tFRWD: BLOCK OVERLAPS [%d, %d)]");
strcpy(&EventLog.msg_tpl[A2D(130], "CLIENT\tFRWD: PROXY TRANSLATION [%d, %d)]");
strcpy(&EventLog.msg_tpl[A2D(131], "CLIENT\tFRWD: REGISTERS TRANSLATION [%d, %d)]");

/// QUEUE (ОЧЕРЕДЬ) [148..179, 32]
strcpy(&EventLog.msg_tpl[A2D(148)], "QUEUE EMPTY");
strcpy(&EventLog.msg_tpl[A2D(149)], "QUEUE OVERLOADED CLIENT %d");

/// POLLING (ОПРОС) [180..219, 40]
strcpy(&EventLog.msg_tpl[A2D(180], "CLIENT\tPOLLING: FUNCTION [%d)] NOT SUPPORTED");
strcpy(&EventLog.msg_tpl[A2D(181)], ""); // РЕЗЕРВ
strcpy(&EventLog.msg_tpl[A2D(182)], "CLIENT\tPOLLING: RTU  RECV - %s");
strcpy(&EventLog.msg_tpl[A2D(183)], "CLIENT\tPOLLING: RTU  SEND - %s");
strcpy(&EventLog.msg_tpl[A2D(184)], "CLIENT\tPOLLING: TCP  RECV - %s");
strcpy(&EventLog.msg_tpl[A2D(185)], "CLIENT\tPOLLING: TCP  SEND - %s");

/// TRAFFIC (ДАННЫЕ) [220..239, 20]
strcpy(&EventLog.msg_tpl[A2D(220], "CLIENT\tTRAFFIC: QUEUE  IN [%d)]");
strcpy(&EventLog.msg_tpl[A2D(221], "CLIENT\tTRAFFIC: QUEUE OUT [%d)]");
*/

  /// анализируем массив шаблонов сообщений, заполняем массив типов шаблонов по комбинациям параметров

  int i, j, k, msglen;
  int d[5], s[5];

  for(i=0; i<EVENT_TEMPLATE_AMOUNT; i++) {
								 
    d[0]=d[1]=d[2]=d[3]=d[4]=0;
    s[0]=s[1]=s[2]=s[3]=s[4]=0;

    msglen=strlen(&EventLog.msg_tpl[A2D(i)]);

    // проверка на длину текста сообщения без учета длины параметров при подстановке
    if(msglen >= EVENT_MESSAGE_LENGTH) return 1;

    if(msglen>0) {

      for(j=0; j<msglen; j++)
        if(EventLog.msg_tpl[A2D(i)+j]  =='%')
        if(EventLog.msg_tpl[A2D(i)+j+1]=='d') {d[d[4]]=1; if(d[4]<3) d[4]++;}
        else
        if(EventLog.msg_tpl[A2D(i)+j+1]=='s') {s[s[4]]=1; if(s[4]<3) s[4]++;}

      EventLog.msg_index[i]=\
        (d[3]<<3)|(d[2]<<2)|(d[1]<<1)|d[0]|\
        (s[3]<<7)|(s[2]<<6)|(s[1]<<5)|(s[0]<<4);

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
	
	if(Security.show_sys_messages==0 && (msgtype & EVENT_CAT_MASK)==EVENT_CAT_DEBUG) return;
	//if(Security.show_data_flow==0    && (msgtype & EVENT_CAT_MASK)==EVENT_CAT_TRAFFIC) return;
	
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

	get_msgsrc_str(msgtype, prm1, eventmsg);
	printf("%s\t", eventmsg);

	eventmsg[0]=0;
	make_msgstr(msgcode, eventmsg, prm1, prm2, prm3, prm4);
	printf("%s\n", eventmsg);

	return;
	}

///-----------------------------------------------------------------------------------------------------------------

void show_traffic(int traffic, int port_id, int client_id, u8 *adu, u16 adu_len)
  {
///	if(port_id!=SERIAL_P3) return; ///!!!

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
	 
	char aux[24];

  // все сообщения без параметров
  if(EventLog.msg_index[msgcode]==EVENT_TPL_DEFAULT)
		sprintf(str, &EventLog.msg_tpl[A2D(msgcode)]);

  // сообщения c первым числовым параметром
  if(EventLog.msg_index[msgcode]==EVENT_TPL_000D)
		sprintf(str, &EventLog.msg_tpl[A2D(msgcode)], prm1);

  // сообщения c двумя числовыми параметрами
  if(EventLog.msg_index[msgcode]==EVENT_TPL_00DD)
		sprintf(str, &EventLog.msg_tpl[A2D(msgcode)], prm1, prm2);

  // сообщения c первым строковым параметром
  if(EventLog.msg_index[msgcode]==EVENT_TPL_000S) {
    // преобразуем строку в название интерфейса
    strcpy(aux, "***");
    if(prm1<=GATEWAY_P8) sprintf(aux, "PORT%d", prm1+1);
    if((prm1>=GATEWAY_T01)&&(prm1<=GATEWAY_T32))
      sprintf(aux, "TCP%0.2d", prm1-GATEWAY_T01+1);
		sprintf(str, &EventLog.msg_tpl[A2D(msgcode)], aux);
    }

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
void get_msgsrc_str(unsigned char msgtype, unsigned int prm1, char *str)
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

			case GATEWAY_LANTCP: if((prm1>=GATEWAY_T01) && (prm1<=GATEWAY_T32))
                             sprintf(str, "   TCP%0.2d", prm1-GATEWAY_T01+1);
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
