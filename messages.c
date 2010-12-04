/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///******************* ������ ������� ��������� ********************************

///=== MESSAGES_H IMPLEMENTATION

#include <pthread.h>

#include "messages.h"
#include "interfaces.h"
#include "moxagate.h"
#include "cli.h"

///=== MESSAGES_H private variables

char eventmsg[EVENT_MESSAGE_LENGTH];
//char message_template[GATEWAY_IFACE+1][EVENT_TEMPLATE_AMOUNT][EVENT_MESSAGE_LENGTH];
char message_template[EVENT_TEMPLATE_AMOUNT][EVENT_MESSAGE_LENGTH];

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

/// COMAND LINE (��������� ������) [XX..XX, XX]

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

/// SECURITY (���������� ������ � ���������) [XX..XX, XX]

strcpy(message_template[SECURITY_TCPPORT ], "CONFIGURATION TCP PORT");

	/// MOXAGATE (����) [XX..XX, XX]

strcpy(message_template[MOXAGATE_MBADDR  ], "CONFIGURATION MBADDR");
strcpy(message_template[MOXAGATE_STATINFO], "CONFIGURATION STATINFO");

/// IFACES (����������) [XX..XX, XX]

strcpy(message_template[IFACE_MBMODE     ], "CONFIGURATION IFACE %s GATEWAY MODE");

strcpy(message_template[IFACE_RTUPHYSPROT], "CONFIGURATION IFACERTU %s PHYSPROTO");
strcpy(message_template[IFACE_RTUSPEED   ], "CONFIGURATION IFACERTU %s SPEED");
strcpy(message_template[IFACE_RTUPARITY  ], "CONFIGURATION IFACERTU %s PARITY");
strcpy(message_template[IFACE_RTUTIMEOUT ], "CONFIGURATION IFACERTU %s TIMEOUT");
strcpy(message_template[IFACE_RTUTCPPORT ], "CONFIGURATION IFACERTU %s TCP PORT");

strcpy(message_template[IFACE_TCPIP1     ], "CONFIGURATION IFACETCP %s IP ADDRESS");
strcpy(message_template[IFACE_TCPPORT1   ], "CONFIGURATION IFACETCP %s TCP PORT");
strcpy(message_template[IFACE_TCPUNITID  ], "CONFIGURATION IFACETCP %s UNIT ID");
strcpy(message_template[IFACE_TCPOFFSET  ], "CONFIGURATION IFACETCP %s OFFSET");
strcpy(message_template[IFACE_TCPMBADDR  ], "CONFIGURATION IFACETCP %s ATM ADDRESS");
strcpy(message_template[IFACE_TCPIP2     ], "CONFIGURATION IFACETCP %s IP ADDRESS 2");
strcpy(message_template[IFACE_TCPPORT2   ], "CONFIGURATION IFACETCP %s TCP PORT 2");
strcpy(message_template[IFACE_TCPIPEQUAL ], "IFACETCP %s IP EQUALS");


/// SYSTEM (���������) [25..64, 40]
strcpy(message_template[ 25], "BUZZER INITIALIZED");
strcpy(message_template[ 26], "KEYPAD INITIALIZED");
strcpy(message_template[ 27], "LCM INITIALIZED");
strcpy(message_template[ 28], "MEMORY ALLOCATED 0%dx:%db");
strcpy(message_template[ 29], "SEMAPHORE SET EXISTS");
strcpy(message_template[ 30], "SHARED MEM: CLOSED");
strcpy(message_template[ 31], "SHARED MEM: EACCES");
strcpy(message_template[ 32], "SHARED MEM: EEXIST");
strcpy(message_template[ 33], "SHARED MEM: EINVAL");
strcpy(message_template[ 34], "SHARED MEM: ENOENT");
strcpy(message_template[ 35], "SHARED MEM: ENOMEM");
strcpy(message_template[ 36], "SHARED MEM: UNKNOWN");
strcpy(message_template[ 37], "SHARED MEM: OK SIZE %db");
strcpy(message_template[ 38], ""); // ������
strcpy(message_template[ 39], "STATUS INFO OVERLAPS");

/*
strcpy(message_template[ 40], "SERIAL PORT INITIALIZED MODE %s");
strcpy(message_template[ 41], "THREAD INITIALIZED CODE %d");
strcpy(message_template[ 42], "THREAD STARTED MODE %s CLIENT %s");
strcpy(message_template[ 43], "THREAD STOPPED");
strcpy(message_template[ 44], "PROGRAM TERMINATED (WORKTIME %d)");
	
/// CONNECTION (������� ����������) [65..127, 63]
strcpy(message_template[ 65], "SOCKET INITIALIZED STAGE %d TCPSERVER %s");
strcpy(message_template[ 66], ""); // ������
strcpy(message_template[ 67], "CONNECTION ACCEPTED FROM %s CLIENT %d");
strcpy(message_template[ 68], "CONNECTION ESTABLISHED WITH %d.%d.%d.%d");
strcpy(message_template[ 69], "CONNECTION FAILED TO %d.%d.%d.%d");
strcpy(message_template[ 70], "CONNECTION REJECTED FROM %d.%d.%d.%d");
strcpy(message_template[ 71], "CONNECTION CLOSED (LINK DOWN) CLIENT %d");
strcpy(message_template[ 72], "CONNECTION CLOSED (TIMEOUT) CLIENT %d");
*/
	
/// FORWARDING (���������������) [XX..XX, XX]

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

/// QUEUE (�������) [148..179, 32]
strcpy(message_template[148], "QUEUE EMPTY");
strcpy(message_template[149], "QUEUE OVERLOADED CLIENT %d");

/// POLLING (�����) [180..219, 40]
strcpy(message_template[180], "CLIENT\tPOLLING: FUNCTION [%d] NOT SUPPORTED");
strcpy(message_template[181], ""); // ������
strcpy(message_template[182], "CLIENT\tPOLLING: RTU  RECV - %s");
strcpy(message_template[183], "CLIENT\tPOLLING: RTU  SEND - %s");
strcpy(message_template[184], "CLIENT\tPOLLING: TCP  RECV - %s");
strcpy(message_template[185], "CLIENT\tPOLLING: TCP  SEND - %s");

/// TRAFFIC (������) [220..239, 20]
strcpy(message_template[220], "CLIENT\tTRAFFIC: QUEUE  IN [%d]");
strcpy(message_template[221], "CLIENT\tTRAFFIC: QUEUE OUT [%d]");
*/

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
	
	/// ��������� ���������

//	if((
//		gate502.msg_filter & 
//		(0x01 << ((msgtype & EVENT_SRC_MASK)-1))
//		)==0) return;

	if(Security.show_sys_messages==0 && (msgtype & EVENT_CAT_MASK)==EVENT_CAT_DEBUG) return;
	//if(Security.show_data_flow==0    && (msgtype & EVENT_CAT_MASK)==EVENT_CAT_TRAFFIC) return;
	
	/// ������� ������ � �������
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
	
	/// ������� ������� �� �������
	
	struct tm *tmd;
	tmd=gmtime(&curtime);
	printf("%2.2d.%2.2d.%4.4d %2.2d:%2.2d:%2.2d\t", tmd->tm_mday, tmd->tm_mon+1, tmd->tm_year+1900,
																									tmd->tm_hour, tmd->tm_min, tmd->tm_sec);

	switch(msgtype & EVENT_TYPE_MASK) {
			case EVENT_TYPE_INF: printf("INF\t"); break;
			case EVENT_TYPE_WRN: printf("WRN\t"); break;
			case EVENT_TYPE_ERR: printf("ERR\t"); break;
			default: printf("***\t");
			}

	switch(msgtype & EVENT_SRC_MASK) {
			case GATEWAY_SYSTEM: 	printf("SYSTEM\t"); 	break;
			case GATEWAY_SECURITY:printf("SECURITY\t"); 	break;
			case GATEWAY_P1: 			printf("PORT1\t"); 		break;
			case GATEWAY_P2: 			printf("PORT2\t"); 		break;
			case GATEWAY_P3: 			printf("PORT3\t"); 		break;
			case GATEWAY_P4: 			printf("PORT4\t"); 		break;
			case GATEWAY_P5: 			printf("PORT5\t"); 		break;
			case GATEWAY_P6: 			printf("PORT6\t"); 		break;
			case GATEWAY_P7: 			printf("PORT7\t"); 		break;
			case GATEWAY_P8: 			printf("PORT8\t"); 		break;
			case GATEWAY_MOXAGATE: 	printf("MOXAGATE\t"); 	break;
			case GATEWAY_LANTCP: printf("LANTCP\t"); 	break;
			// case GATEWAY_TCPBRIDGE: printf("BRIDGE%d\t", prm2+1); 	break;

			default: 								printf("NONAME\t");
			}

	eventmsg[0]=0;
	make_msgstr(msgcode, eventmsg, prm1, prm2, prm3, prm4);
	printf("%s\n", eventmsg);

	// �������������� ������� ���������� ������ ���������
	if(app_log!=NULL)
		app_log_current_entry=\
			app_log_current_entry==EVENT_LOG_LENGTH-1?\
			0:app_log_current_entry+1;
	
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

///--- COMAND LINE (��������� ������) [1..24, 24]
///--- ��������� ��� ����������
  if(	msgcode==43 || msgcode==39 ||
		  (msgcode<=36 && msgcode>=29) ||
		  (msgcode<=27 && msgcode>=25) ||
		  (msgcode<=14 && msgcode>=1))
		sprintf(str, message_template[msgcode]);

///--- SYSTEM (���������) [25..64, 40]

	if(msgcode==28) // MEMORY ALLOCATED 0%dx:%db; LEN25
			sprintf(str, message_template[msgcode], prm1, prm2);

	if(msgcode==37) // SHARED MEM: OK SIZE %db; LEN23
			sprintf(str, message_template[msgcode], prm1);

	if(msgcode==40) // SERIAL PORT INITIALIZED MODE %s; LEN40
		switch(prm1) {
			case GATEWAY_SIMPLE: 	sprintf(str, message_template[msgcode], "GATEWAY SIMPLE"); break;
			case GATEWAY_ATM: 		sprintf(str, message_template[msgcode], "GATEWAY ATM"); break;
			case GATEWAY_RTM: 		sprintf(str, message_template[msgcode], "GATEWAY RTM"); break;
			case GATEWAY_PROXY: 	sprintf(str, message_template[msgcode], "GATEWAY PROXY"); break;
			case BRIDGE_PROXY: 		sprintf(str, message_template[msgcode], "BRIDGE PROXY"); break;
//			case BRIDGE_SIMPLE: 	sprintf(str, message_template[msgcode], "BRIDGE SIMPLE"); break;
			case MODBUS_PORT_ERROR: sprintf(str, message_template[msgcode], "PORT ERROR"); break;
			default: 							sprintf(str, message_template[msgcode], "PORT OFF");
			}

	if(msgcode==41) // THREAD INITIALIZED CODE %d
			sprintf(str, message_template[msgcode], prm1);

	if(msgcode==42) // THREAD STARTED MODE %s CLIENT %s
		switch(prm1) {
			case GATEWAY_SIMPLE: 																					 
				sscanf(aux, "%d", &prm2);
				sprintf(str, message_template[msgcode], "GATEWAY SIMPLE", aux);
				break;
			case GATEWAY_ATM: 		sprintf(str, message_template[msgcode], "GATEWAY ATM", "MOXA"); break;
			case GATEWAY_RTM: 		sprintf(str, message_template[msgcode], "GATEWAY RTM", "MOXA"); break;
			case GATEWAY_PROXY: 	sprintf(str, message_template[msgcode], "GATEWAY PROXY", "MOXA"); break;
			case BRIDGE_PROXY: 		sprintf(str, message_template[msgcode], "BRIDGE PROXY", "RTU"); break;
			case BRIDGE_TCP: 			sprintf(str, message_template[msgcode], "BRIDGE TCP", "MOXA"); break;
			case MOXA_MB_DEVICE: 	sprintf(str, message_template[msgcode], "MOXA DEVICE", "MOXA"); break;
			case MODBUS_PORT_ERROR: sprintf(str, message_template[msgcode], "PORT ERROR", "N/A"); break;
			default: 							sprintf(str, message_template[msgcode], "PORT OFF", "N/A");
			}

	if(msgcode==44) // PROGRAM TERMINATED (WORKTIME %d)
			sprintf(str, message_template[msgcode], prm1);

///--- CONNECTION (������� ����������) [65..127, 63]

	if(msgcode==65) // SOCKET INITIALIZED STAGE %d TCPSERVER %s;
//		if(prm2==DEFAULT_CLIENT) 	sprintf(str, message_template[msgcode], prm1, "N/A");
//		  else  									
             sprintf(str, message_template[msgcode], prm1, "xxx.xxx.xxx.xxx");

	if(msgcode==67) // CONNECTION ACCEPTED FROM %s CLIENT %d;
		if(prm1==0) sprintf(str, message_template[msgcode], "N/A", 0);
		  else {
				sprintf(aux, "%d.%d.%d.%d", prm1 >> 24, (prm1 >> 16) & 0xff, (prm1 >> 8) & 0xff, prm1 & 0xff);
				sprintf(str, message_template[msgcode], aux, prm2);
		    }

	if(	msgcode==68 || 	// CONNECTION ESTABLISHED WITH %d.%d.%d.%d;
			msgcode==69 ||	// CONNECTION FAILED TO %d.%d.%d.%d;
			msgcode==70) 		// CONNECTION REJECTED FROM %d.%d.%d.%d;
		sprintf(str, message_template[msgcode], prm1 >> 24, (prm1 >> 16) & 0xff, (prm1 >> 8) & 0xff, prm1 & 0xff);

	if(	msgcode==71 || 	// CONNECTION CLOSED (LINK DOWN) CLIENT %d
			msgcode==72) 		// CONNECTION CLOSED (TIMEOUT) CLIENT %d
		sprintf(str, message_template[msgcode], prm1);

///--- QUEUE (�������) [148..179, 32]
	if(msgcode==148) // QUEUE EMPTY
		sprintf(str, message_template[msgcode]);

	if(msgcode==149)  // QUEUE OVERLOADED CLIENT %d
		sprintf(str, message_template[msgcode], prm1);

///--- FORWARDING (���������������) [128..147, 20]
	if(msgcode==128) // CLIENT\tFRWD: ADDRESS [%d] NOT TRANSLATED
		sprintf(str, message_template[msgcode], prm2);

	if(msgcode==129) // CLIENT\tFRWD: BLOCK OVERLAPS [%d, %d]
		sprintf(str, message_template[msgcode], prm2, prm3);

	if(msgcode==130) // CLIENT\tFRWD: PROXY TRANSLATION [%d, %d]
		sprintf(str, message_template[msgcode], prm2, prm3);

	if(msgcode==131) // CLIENT\tFRWD: REGISTERS TRANSLATION [%d, %d]
		sprintf(str, message_template[msgcode], prm2, prm3);

///--- POLLING (�����) [180..191, 12]
	if(msgcode==180) // CLIENT\tPOLLING: FUNCTION [%d] NOT SUPPORTED
		sprintf(str, message_template[msgcode], prm2);

	if(	(msgcode==182) ||	// CLIENT\tPOLLING: RTU  RECV - %s
			(msgcode==183))  	// CLIENT\tPOLLING: RTU  SEND - %s
		switch(prm1) {
			case MB_SERIAL_WRITE_ERR:
				sprintf(str, message_template[msgcode], "COM FAILURE");
				break;
			case MB_SERIAL_READ_FAILURE:
				sprintf(str, message_template[msgcode], "COM FAILURE");
				break;
			case MB_SERIAL_COM_TIMEOUT:
				sprintf(str, message_template[msgcode], "TIMEOUT");
				break;
			case MB_SERIAL_ADU_ERR_MIN:
				sprintf(str, message_template[msgcode], "ADU MIN LEN");
				break;
			case MB_SERIAL_ADU_ERR_MAX:
				sprintf(str, message_template[msgcode], "ADU MAX LEN");
				break;
			case MB_SERIAL_CRC_ERROR:
				sprintf(str, message_template[msgcode], "WRONG CRC");
				break;
			case MB_SERIAL_PDU_ERR:
				sprintf(str, message_template[msgcode], "WRONG PDU");
				break;
			default:
				sprintf(str, message_template[msgcode], "UNKNOWN");
			}

	if(	(msgcode==184) ||	// CLIENT\tPOLLING: TCP  RECV - %s
			(msgcode==185))  	// CLIENT\tPOLLING: TCP  SEND - %s
		switch(prm1) {
			case TCP_COM_ERR_NULL:
				sprintf(str, message_template[msgcode], "NO INPUT DATA");
				break;
			case TCP_ADU_ERR_MIN:
				sprintf(str, message_template[msgcode], "ADU MIN LEN");
				break;
			case TCP_ADU_ERR_MAX:
				sprintf(str, message_template[msgcode], "ADU MAX LEN");
				break;
			case TCP_ADU_ERR_PROTOCOL:
				sprintf(str, message_template[msgcode], "WRONG PROTOCOL");
				break;
			case TCP_ADU_ERR_LEN:
				sprintf(str, message_template[msgcode], "WRONG ADU LEN");
				break;
			case TCP_ADU_ERR_UID:
				sprintf(str, message_template[msgcode], "WRONG UID");
				break;
			case TCP_PDU_ERR:
				sprintf(str, message_template[msgcode], "WRONG PDU");
				break;
			case TCP_COM_ERR_SEND:
				sprintf(str, message_template[msgcode], "COM FAILURE");
				break;
			default:
				sprintf(str, message_template[msgcode], "UNKNOWN");
			}

	/// TRAFFIC (������) [220..239, 20]
	if(msgcode==220) // CLIENT\tTRAFFIC: QUEUE  IN [%d]
		sprintf(str, message_template[msgcode], prm2);

	if(msgcode==221) // CLIENT\tTRAFFIC: QUEUE OUT [%d]
		sprintf(str, message_template[msgcode], prm2);

	return;
	}

///-----------------------------------------------------------------------------------------------------------------
