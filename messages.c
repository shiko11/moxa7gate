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

char eventmsg[EVENT_MESSAGE_LENGTH];
//char message_template[GATEWAY_IFACE+1][EVENT_TEMPLATE_AMOUNT][EVENT_MESSAGE_LENGTH];
char message_template[EVENT_TEMPLATE_AMOUNT][EVENT_MESSAGE_LENGTH];
unsigned int message_index[EVENT_TEMPLATE_AMOUNT];

///=== MESSAGES_H private functions

void init_message_templates();

//******************************************************************************
int init_messages_h()
  {

  app_log_current_entry=app_log_entries_total=0;
  app_log=NULL;
  msg_filter=0xFFffFFff;

  init_message_templates();

  return 0;
  }

///----------------------------------------------------------------------------
void init_message_templates()
  {
	memset(message_template, 0, sizeof(message_template));
	memset(message_index   , 0, sizeof(message_index   ));


/// чисто дурацкая проверка для тестовой компиляции на отсуствие косяков при присвоении значений константам,
/// для генерации кода ниже рекомендуется обработать код этой функции при помощи регулярных выражений,
/// что обеспечит отсуствие ошибок ручного ввода.
/// значения, выданные кодом ниже, должны быть: 1. уникальными; 2. меньше константы EVENT_TEMPLATE_AMOUNT
/// /^.*?\[(.+)\].*$/mi printf("\1\\t%d\\n", \1);
printf("COMMAND_LINE_OK\t%d\n", COMMAND_LINE_OK);
printf("COMMAND_LINE_ERROR\t%d\n", COMMAND_LINE_ERROR);
printf("COMMAND_LINE_INFO\t%d\n", COMMAND_LINE_INFO);
printf("COMMAND_LINE_ARGC\t%d\n", COMMAND_LINE_ARGC);
printf("COMMAND_LINE_UNDEFINED\t%d\n", COMMAND_LINE_UNDEFINED);
printf("SECURITY_CONF_STRUCT\t%d\n", SECURITY_CONF_STRUCT);
printf("SECURITY_CONF_DUPLICATE\t%d\n", SECURITY_CONF_DUPLICATE);
printf("SECURITY_CONF_SPELLING\t%d\n", SECURITY_CONF_SPELLING);
printf("IFACE_CONF_RTUDUPLICATE\t%d\n", IFACE_CONF_RTUDUPLICATE);
printf("IFACE_CONF_RTUSTRUCT\t%d\n", IFACE_CONF_RTUSTRUCT);
printf("IFACE_CONF_GWMODE\t%d\n", IFACE_CONF_GWMODE);
printf("IFACE_CONF_TCPDUPLICATE\t%d\n", IFACE_CONF_TCPDUPLICATE);
printf("IFACE_CONF_TCPSTRUCT\t%d\n", IFACE_CONF_TCPSTRUCT);
printf("ATM_CONF_SPELLING\t%d\n", ATM_CONF_SPELLING);
printf("ATM_CONF_STRUCT\t%d\n", ATM_CONF_STRUCT);
printf("VSLAVE_CONF_OVERFLOW\t%d\n", VSLAVE_CONF_OVERFLOW);
printf("VSLAVE_CONF_STRUCT\t%d\n", VSLAVE_CONF_STRUCT);
printf("VSLAVE_CONF_IFACE\t%d\n", VSLAVE_CONF_IFACE);
printf("PQUERY_CONF_OVERFLOW\t%d\n", PQUERY_CONF_OVERFLOW);
printf("PQUERY_CONF_STRUCT\t%d\n", PQUERY_CONF_STRUCT);
printf("PQUERY_CONF_IFACE\t%d\n", PQUERY_CONF_IFACE);
printf("EXPT_CONF_OVERFLOW\t%d\n", EXPT_CONF_OVERFLOW);
printf("EXPT_CONF_STRUCT\t%d\n", EXPT_CONF_STRUCT);
printf("EXPT_CONF_STAGE\t%d\n", EXPT_CONF_STAGE);
printf("SECURITY_TCPPORT\t%d\n", SECURITY_TCPPORT);
printf("MOXAGATE_MBADDR\t%d\n", MOXAGATE_MBADDR);
printf("MOXAGATE_STATINFO\t%d\n", MOXAGATE_STATINFO);
printf("IFACE_MBMODE\t%d\n", IFACE_MBMODE);
printf("IFACE_RTUPHYSPROT\t%d\n", IFACE_RTUPHYSPROT);
printf("IFACE_RTUSPEED\t%d\n", IFACE_RTUSPEED);
printf("IFACE_RTUPARITY\t%d\n", IFACE_RTUPARITY);
printf("IFACE_RTUTIMEOUT\t%d\n", IFACE_RTUTIMEOUT);
printf("IFACE_RTUTCPPORT\t%d\n", IFACE_RTUTCPPORT);
printf("IFACE_TCPIP1\t%d\n", IFACE_TCPIP1);
printf("IFACE_TCPPORT1\t%d\n", IFACE_TCPPORT1);
printf("IFACE_TCPUNITID\t%d\n", IFACE_TCPUNITID);
printf("IFACE_TCPOFFSET\t%d\n", IFACE_TCPOFFSET);
printf("IFACE_TCPMBADDR\t%d\n", IFACE_TCPMBADDR);
printf("IFACE_TCPIP2\t%d\n", IFACE_TCPIP2);
printf("IFACE_TCPPORT2\t%d\n", IFACE_TCPPORT2);
printf("IFACE_TCPIPEQUAL\t%d\n", IFACE_TCPIPEQUAL);
printf("HMI_KLB_INIT_KEYPAD\t%d\n", HMI_KLB_INIT_KEYPAD);
printf("HMI_KLB_INIT_LCM\t%d\n", HMI_KLB_INIT_LCM);
printf("HMI_KLB_INIT_BUZZER\t%d\n", HMI_KLB_INIT_BUZZER);
printf("HMI_KLB_INIT_THREAD\t%d\n", HMI_KLB_INIT_THREAD);
printf("HMI_WEB_ENOENT\t%d\n", HMI_WEB_ENOENT);
printf("HMI_WEB_EACCES\t%d\n", HMI_WEB_EACCES);
printf("HMI_WEB_EINVAL\t%d\n", HMI_WEB_EINVAL);
printf("HMI_WEB_ENOMEM\t%d\n", HMI_WEB_ENOMEM);
printf("HMI_WEB_EEXIST\t%d\n", HMI_WEB_EEXIST);
printf("HMI_WEB_UNKNOWN\t%d\n", HMI_WEB_UNKNOWN);
printf("HMI_WEB_OK\t%d\n", HMI_WEB_OK);
printf("HMI_WEB_CLOSED\t%d\n", HMI_WEB_CLOSED);
printf("ATM_IFACE\t%d\n", ATM_IFACE);
printf("ATM_MBADDR\t%d\n", ATM_MBADDR);
printf("VSLAVE_IFACE\t%d\n", VSLAVE_IFACE);
printf("VSLAVE_MBADDR\t%d\n", VSLAVE_MBADDR);
printf("VSLAVE_MBTABL\t%d\n", VSLAVE_MBTABL);
printf("VSLAVE_BEGDIAP\t%d\n", VSLAVE_BEGDIAP);
printf("VSLAVE_ENDDIAP\t%d\n", VSLAVE_ENDDIAP);
printf("VSLAVE_LENDIAP\t%d\n", VSLAVE_LENDIAP);
printf("PQUERY_IFACE\t%d\n", PQUERY_IFACE);
printf("PQUERY_MBADDR\t%d\n", PQUERY_MBADDR);
printf("PQUERY_MBTABL\t%d\n", PQUERY_MBTABL);
printf("PQUERY_ACCESS\t%d\n", PQUERY_ACCESS);
printf("PQUERY_ENDREGREAD\t%d\n", PQUERY_ENDREGREAD);
printf("PQUERY_LENPACKET\t%d\n", PQUERY_LENPACKET);
printf("PQUERY_ENDREGWRITE\t%d\n", PQUERY_ENDREGWRITE);
printf("PQUERY_DELAYMIN\t%d\n", PQUERY_DELAYMIN);
printf("PQUERY_DELAYMAX\t%d\n", PQUERY_DELAYMAX);
printf("PQUERY_ERRCNTR\t%d\n", PQUERY_ERRCNTR);
printf("EXPT_STAGE\t%d\n", EXPT_STAGE);
printf("EXPT_ACTION\t%d\n", EXPT_ACTION);
printf("EXPT_PRM1\t%d\n", EXPT_PRM1);
printf("EXPT_PRM2\t%d\n", EXPT_PRM2);
printf("EXPT_PRM3\t%d\n", EXPT_PRM3);
printf("EXPT_PRM4\t%d\n", EXPT_PRM4);

exit(1);


/// COMAND LINE (КОМАНДНАЯ СТРОКА) [XX..XX, XX]

strcpy(message_template[COMMAND_LINE_OK        ], "CLI: COMMAND LINE PARSED SUCCESSFULLY");
strcpy(message_template[COMMAND_LINE_ERROR     ], "CLI: WHERE IS %d PARSING ERROR(S)");
strcpy(message_template[COMMAND_LINE_INFO      ], "CLI: SYSINFO DISPLAYED AND EXIT");
strcpy(message_template[COMMAND_LINE_ARGC      ], "CLI: WHERE IS UNKNOWN PARAMETER(S)");
strcpy(message_template[COMMAND_LINE_UNDEFINED ], "CLI: COMMAND LINE UNDEFINED RESULT");

strcpy(message_template[SECURITY_CONF_STRUCT   ], "CLI: SECURITY STRUCTURE");
strcpy(message_template[SECURITY_CONF_DUPLICATE], "CLI: SECURITY PARAM DUPLICATED");
strcpy(message_template[SECURITY_CONF_SPELLING ], "CLI: SECURITY PARAM UNKNOWN");

strcpy(message_template[IFACE_CONF_RTUDUPLICATE], "CLI: IFACERTU %s DUPLICATED");
strcpy(message_template[IFACE_CONF_RTUSTRUCT   ], "CLI: IFACERTU %s STRUCTURE");
strcpy(message_template[IFACE_CONF_GWMODE      ], "CLI: IFACE %s GATEWAY MODE");

strcpy(message_template[IFACE_CONF_TCPDUPLICATE], "CLI: IFACETCP %s DUPLICATED");
strcpy(message_template[IFACE_CONF_TCPSTRUCT   ], "CLI: IFACETCP %s STRUCTURE");

strcpy(message_template[ATM_CONF_SPELLING      ], "CLI: ADDRESS MAP SPELLING");
strcpy(message_template[ATM_CONF_STRUCT        ], "CLI: ADDRESS MAP STRUCTURE");

strcpy(message_template[VSLAVE_CONF_OVERFLOW   ], "CLI: VSLAVES OVERFLOW");
strcpy(message_template[VSLAVE_CONF_STRUCT     ], "CLI: VSLAVES STRUCTURE");
strcpy(message_template[VSLAVE_CONF_IFACE      ], "CLI: VSLAVES IFACE");

strcpy(message_template[PQUERY_CONF_OVERFLOW   ], "CLI: PQUERIES OVERFLOW");
strcpy(message_template[PQUERY_CONF_STRUCT     ], "CLI: PQUERIES STRUCTURE");
strcpy(message_template[PQUERY_CONF_IFACE      ], "CLI: PQUERIES IFACE");

strcpy(message_template[EXPT_CONF_OVERFLOW     ], "CLI: EXCEPTIONS OVERFLOW");
strcpy(message_template[EXPT_CONF_STRUCT       ], "CLI: EXCEPTIONS STRUCTURE");
strcpy(message_template[EXPT_CONF_STAGE        ], "CLI: EXCEPTIONS STAGE");

/// SECURITY (ПОДСИСТЕМА РАБОТЫ С КЛИЕНТАМИ) [XX..XX, XX]

strcpy(message_template[SECURITY_TCPPORT ], "CONFIGURATION TCP PORT");

	/// MOXAGATE (ШЛЮЗ) [XX..XX, XX]

strcpy(message_template[MOXAGATE_MBADDR  ], "CONFIGURATION MBADDR");
strcpy(message_template[MOXAGATE_STATINFO], "CONFIGURATION STATINFO");

/// IFACES (ИНТЕРФЕЙСЫ) [XX..XX, XX]

strcpy(message_template[IFACE_MBMODE     ], "CONFIGURATION IFACE GATEWAY MODE");

strcpy(message_template[IFACE_RTUPHYSPROT], "CONFIGURATION IFACERTU PHYSPROTO");
strcpy(message_template[IFACE_RTUSPEED   ], "CONFIGURATION IFACERTU SPEED");
strcpy(message_template[IFACE_RTUPARITY  ], "CONFIGURATION IFACERTU PARITY");
strcpy(message_template[IFACE_RTUTIMEOUT ], "CONFIGURATION IFACERTU TIMEOUT");
strcpy(message_template[IFACE_RTUTCPPORT ], "CONFIGURATION IFACERTU TCP PORT");

strcpy(message_template[IFACE_TCPIP1     ], "CONFIGURATION IFACETCP IP ADDRESS");
strcpy(message_template[IFACE_TCPPORT1   ], "CONFIGURATION IFACETCP TCP PORT");
strcpy(message_template[IFACE_TCPUNITID  ], "CONFIGURATION IFACETCP UNIT ID");
strcpy(message_template[IFACE_TCPOFFSET  ], "CONFIGURATION IFACETCP OFFSET");
strcpy(message_template[IFACE_TCPMBADDR  ], "CONFIGURATION IFACETCP ATM ADDRESS");
strcpy(message_template[IFACE_TCPIP2     ], "CONFIGURATION IFACETCP IP ADDRESS 2");
strcpy(message_template[IFACE_TCPPORT2   ], "CONFIGURATION IFACETCP TCP PORT 2");
strcpy(message_template[IFACE_TCPIPEQUAL ], "CONFIGURATION IFACETCP IP EQUALS");


/// HMI (Человеко-машинный интерфейс) [XX..XX, XX]
strcpy(message_template[HMI_KLB_INIT_KEYPAD], "KEYPAD INITIALIZATION");
strcpy(message_template[HMI_KLB_INIT_LCM   ], "LCM INITIALIZATION");
strcpy(message_template[HMI_KLB_INIT_BUZZER], "BUZZER INITIALIZATION");
strcpy(message_template[HMI_KLB_INIT_THREAD], "HMI THREAD INITIALIZATION");

strcpy(message_template[HMI_WEB_ENOENT ], "SHARED MEM: ENOENT");
strcpy(message_template[HMI_WEB_EACCES ], "SHARED MEM: EACCES");
strcpy(message_template[HMI_WEB_EINVAL ], "SHARED MEM: EINVAL");
strcpy(message_template[HMI_WEB_ENOMEM ], "SHARED MEM: ENOMEM");
strcpy(message_template[HMI_WEB_EEXIST ], "SHARED MEM: EEXIST");
strcpy(message_template[HMI_WEB_UNKNOWN], "SHARED MEM: UNKNOWN");
strcpy(message_template[HMI_WEB_OK     ], "SHARED MEM: OK SIZE %db");
strcpy(message_template[HMI_WEB_CLOSED ], "SHARED MEM: CLOSED");

/*
strcpy(message_template[ 29], "SEMAPHORE SET EXISTS");
strcpy(message_template[ 28], "MEMORY ALLOCATED 0%dx:%db");
strcpy(message_template[ 38], ""); // РЕЗЕРВ
strcpy(message_template[ 39], "STATUS INFO OVERLAPS");

strcpy(message_template[ 40], "SERIAL PORT INITIALIZED MODE %s");
strcpy(message_template[ 41], "THREAD INITIALIZED CODE %d");
strcpy(message_template[ 42], "THREAD STARTED MODE %s CLIENT %s");
strcpy(message_template[ 43], "THREAD STOPPED");
strcpy(message_template[ 44], "PROGRAM TERMINATED (WORKTIME %d)");
	
/// CONNECTION (СЕТЕВОЕ СОЕДИНЕНИЕ) [65..127, 63]
strcpy(message_template[ 65], "SOCKET INITIALIZED STAGE %d TCPSERVER %s");
strcpy(message_template[ 66], ""); // РЕЗЕРВ
strcpy(message_template[ 67], "CONNECTION ACCEPTED FROM %s CLIENT %d");
strcpy(message_template[ 68], "CONNECTION ESTABLISHED WITH %d.%d.%d.%d");
strcpy(message_template[ 69], "CONNECTION FAILED TO %d.%d.%d.%d");
strcpy(message_template[ 70], "CONNECTION REJECTED FROM %d.%d.%d.%d");
strcpy(message_template[ 71], "CONNECTION CLOSED (LINK DOWN) CLIENT %d");
strcpy(message_template[ 72], "CONNECTION CLOSED (TIMEOUT) CLIENT %d");
*/
	
/// FORWARDING (ПЕРЕНАПРАВЛЕНИЕ) [XX..XX, XX]

strcpy(message_template[ATM_IFACE         ], "CONFIGURATION ADDRESS MAP #%d IFACE");
strcpy(message_template[ATM_MBADDR        ], "CONFIGURATION ADDRESS MAP #%d MBADDR");

strcpy(message_template[VSLAVE_IFACE      ], "CONFIGURATION VSLAVE #%d IFACE");
strcpy(message_template[VSLAVE_MBADDR     ], "CONFIGURATION VSLAVE #%d MBADDR");
strcpy(message_template[VSLAVE_MBTABL     ], "CONFIGURATION VSLAVE #%d MBTABLE");
strcpy(message_template[VSLAVE_BEGDIAP    ], "INCORRECT VSLAVE #%d BEGDIAP");
strcpy(message_template[VSLAVE_ENDDIAP    ], "INCORRECT VSLAVE #%d ENDDIAP");
strcpy(message_template[VSLAVE_LENDIAP    ], "INCORRECT VSLAVE #%d LENDIAP");

strcpy(message_template[PQUERY_IFACE      ], "CONFIGURATION PQUERY #%d IFACE");
strcpy(message_template[PQUERY_MBADDR     ], "CONFIGURATION PQUERY #%d MBADDR");
strcpy(message_template[PQUERY_MBTABL     ], "CONFIGURATION PQUERY #%d MBTABLE");
strcpy(message_template[PQUERY_ACCESS     ], "CONFIGURATION PQUERY #%d ACCESS");
strcpy(message_template[PQUERY_ENDREGREAD ], "CONFIGURATION PQUERY #%d ENDREAD");
strcpy(message_template[PQUERY_LENPACKET  ], "CONFIGURATION PQUERY #%d PACKET LENGTH");
strcpy(message_template[PQUERY_ENDREGWRITE], "CONFIGURATION PQUERY #%d ENDWRITE");
strcpy(message_template[PQUERY_DELAYMIN   ], "CONFIGURATION PQUERY #%d DELAY MIN");
strcpy(message_template[PQUERY_DELAYMAX   ], "CONFIGURATION PQUERY #%d DELAY MAX");
strcpy(message_template[PQUERY_ERRCNTR    ], "CONFIGURATION PQUERY #%d CRITICAL");

strcpy(message_template[EXPT_STAGE        ], "CONFIGURATION EXCEPTION #%d STAGE");
strcpy(message_template[EXPT_ACTION       ], "CONFIGURATION EXCEPTION #%d ACTION");
strcpy(message_template[EXPT_PRM1         ], "CONFIGURATION EXCEPTION #%d PRM1");
strcpy(message_template[EXPT_PRM2         ], "CONFIGURATION EXCEPTION #%d PRM2");
strcpy(message_template[EXPT_PRM3         ], "CONFIGURATION EXCEPTION #%d PRM3");
strcpy(message_template[EXPT_PRM4         ], "CONFIGURATION EXCEPTION #%d PRM4");

/*
strcpy(message_template[128], "CLIENT\tFRWD: ADDRESS [%d] NOT TRANSLATED");
strcpy(message_template[129], "CLIENT\tFRWD: BLOCK OVERLAPS [%d, %d]");
strcpy(message_template[130], "CLIENT\tFRWD: PROXY TRANSLATION [%d, %d]");
strcpy(message_template[131], "CLIENT\tFRWD: REGISTERS TRANSLATION [%d, %d]");

/// QUEUE (ОЧЕРЕДЬ) [148..179, 32]
strcpy(message_template[148], "QUEUE EMPTY");
strcpy(message_template[149], "QUEUE OVERLOADED CLIENT %d");

/// POLLING (ОПРОС) [180..219, 40]
strcpy(message_template[180], "CLIENT\tPOLLING: FUNCTION [%d] NOT SUPPORTED");
strcpy(message_template[181], ""); // РЕЗЕРВ
strcpy(message_template[182], "CLIENT\tPOLLING: RTU  RECV - %s");
strcpy(message_template[183], "CLIENT\tPOLLING: RTU  SEND - %s");
strcpy(message_template[184], "CLIENT\tPOLLING: TCP  RECV - %s");
strcpy(message_template[185], "CLIENT\tPOLLING: TCP  SEND - %s");

/// TRAFFIC (ДАННЫЕ) [220..239, 20]
strcpy(message_template[220], "CLIENT\tTRAFFIC: QUEUE  IN [%d]");
strcpy(message_template[221], "CLIENT\tTRAFFIC: QUEUE OUT [%d]");
*/

  /// анализируем массив шаблонов сообщений, заполняем массив типов шаблонов по комбинациям параметров

  int i, j, k, msglen;
  int d[5], s[5];

  for(i=0; i<EVENT_TEMPLATE_AMOUNT; i++) {
								 
    d[0]=d[1]=d[2]=d[3]=d[4]=0;
    s[0]=s[1]=s[2]=s[3]=s[4]=0;

    msglen=strlen(message_template[i]);
    if(msglen>0) {

      for(j=0; j<msglen; j++)
        if(message_template[i][j]=='%')
        if(message_template[i][j+1]=='d')      {d[d[4]]=1; if(d[4]<3) d[4]++;}
        else if(message_template[i][j+1]=='s') {s[s[4]]=1; if(s[4]<3) s[4]++;}

      message_index[i]=\
        (d[3]<<3)|(d[2]<<2)|(d[1]<<1)|d[0]|\
        (s[3]<<7)|(s[2]<<6)|(s[1]<<5)|(s[0]<<4);

      }

    }

  return;
  }

///-----------------------------------------------------------------------------------------------------------------

void sysmsg_ex(unsigned char msgtype, unsigned char msgcode,
								unsigned int prm1,
								unsigned int prm2,
								unsigned int prm3,
								unsigned int prm4)
	{
	time_t	curtime;
	//int i;
	
	/// фильтруем сообщения

//	if((
//		gate502.msg_filter & 
//		(0x01 << ((msgtype & EVENT_SRC_MASK)-1))
//		)==0) return;

	if(Security.show_sys_messages==0 && (msgtype & EVENT_CAT_MASK)==EVENT_CAT_DEBUG) return;
	//if(Security.show_data_flow==0    && (msgtype & EVENT_CAT_MASK)==EVENT_CAT_TRAFFIC) return;
	
	/// создаем запись в журнале
	time(&curtime);
	if(app_log!=NULL) {
		app_log[app_log_current_entry].time=curtime;
		app_log[app_log_current_entry].msgtype=msgtype;
		app_log[app_log_current_entry].msgcode=msgcode;
		
		app_log[app_log_current_entry].prm[0]=prm1;
		app_log[app_log_current_entry].prm[1]=prm2;
		app_log[app_log_current_entry].prm[2]=prm3;
		app_log[app_log_current_entry].prm[3]=prm4;
		
		app_log_entries_total++;
	  } else printf("!");
	
	// инкрементируем счетчик кольцевого буфера сообщений
	if(app_log!=NULL)
		app_log_current_entry=\
			app_log_current_entry==EVENT_LOG_LENGTH-1?\
			0:app_log_current_entry+1;
	
	/// выводим событие на консоль
	
	struct tm *tmd;
	tmd=gmtime(&curtime);
//  strftime(eventmsg, 16, "%b %y", &tmd);
	printf("%2.2d.%2.2d.%4.4d %2.2d:%2.2d:%2.2d\t", tmd->tm_mday, tmd->tm_mon+1, tmd->tm_year+1900,
																									tmd->tm_hour, tmd->tm_min, tmd->tm_sec);
//	printf("%2.2d %s %2.2d:%2.2d:%2.2d\t", tmd->tm_mday, eventmsg,
//																									tmd->tm_hour, tmd->tm_min, tmd->tm_sec);

	switch(msgtype & EVENT_TYPE_MASK) {
			case EVENT_TYPE_INF: printf("INF\t"); break;
			case EVENT_TYPE_WRN: printf("WRN\t"); break;
			case EVENT_TYPE_ERR: printf("ERR\t"); break;
			default: printf("***\t");
			}

	switch(msgtype & EVENT_SRC_MASK) {
			case GATEWAY_P1: 			printf("PORT1\t"); 		break;
			case GATEWAY_P2: 			printf("PORT2\t"); 		break;
			case GATEWAY_P3: 			printf("PORT3\t"); 		break;
			case GATEWAY_P4: 			printf("PORT4\t"); 		break;
			case GATEWAY_P5: 			printf("PORT5\t"); 		break;
			case GATEWAY_P6: 			printf("PORT6\t"); 		break;
			case GATEWAY_P7: 			printf("PORT7\t"); 		break;
			case GATEWAY_P8: 			printf("PORT8\t"); 		break;

			case GATEWAY_SYSTEM:   printf("SYSTEM\t"); 	break;
			case GATEWAY_MOXAGATE: printf("MOXAGATE\t"); 	break;

			case GATEWAY_LANTCP: if((prm1>=GATEWAY_T01) && (prm1<=GATEWAY_T32))
                             printf("TCP%0.2d\t", prm1);
                             else printf("LANTCP\t");
                           break;

			case GATEWAY_FRWD: printf("FRWD\t"); 	break;
			case GATEWAY_HMI: printf("HMI\t"); 	break;

			default: 								printf("***\t");
			}

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
  if(message_index[msgcode]==EVENT_TPL_DEFAULT)
		sprintf(str, message_template[msgcode]);

  // сообщения c первым числовым параметром
  if(message_index[msgcode]==EVENT_TPL_000D)
		sprintf(str, message_template[msgcode], prm1);

  // сообщения c двумя числовыми параметрами
  if(message_index[msgcode]==EVENT_TPL_00DD)
		sprintf(str, message_template[msgcode], prm1, prm2);

  // сообщения c первым строковым параметром
  if(message_index[msgcode]==EVENT_TPL_000S) {
    // преобразуем строку в название интерфейса
    strcpy(aux, "***");
    if(prm1<=GATEWAY_P8) sprintf(aux, "PORT%d", prm1+1);
    if((prm1>=GATEWAY_T01)&&(prm1<=GATEWAY_T32))
      sprintf(aux, "TCP%0.2d", prm1-GATEWAY_T01+1);
		sprintf(str, message_template[msgcode], aux);
    }

	return;
	}

///-----------------------------------------------------------------------------------------------------------------
