#include "global.h"

void copy_stat(GW_StaticData *dst, GW_StaticData *src)
	{
	dst->accepted=src->accepted;
	dst->errors_input_communication=src->errors_input_communication;
	dst->errors_tcp_adu=src->errors_tcp_adu;
	dst->errors_tcp_pdu=src->errors_tcp_pdu;
	dst->errors_serial_sending=src->errors_serial_sending;
	dst->errors_serial_accepting=src->errors_serial_accepting;
	dst->timeouts=src->timeouts;
	dst->crc_errors=src->crc_errors;
	dst->errors_serial_adu=src->errors_serial_adu;
	dst->errors_serial_pdu=src->errors_serial_pdu;
	dst->errors_tcp_sending=src->errors_tcp_sending;
	dst->errors=src->errors;
	dst->sended=src->sended;

	int i;
	for(i=0; i<STAT_FUNC_AMOUNT; i++) {
		dst->func[i][STAT_RES_OK]= 	src->func[i][STAT_RES_OK];
		dst->func[i][STAT_RES_ERR]=	src->func[i][STAT_RES_ERR];
		dst->func[i][STAT_RES_EXP]=	src->func[i][STAT_RES_EXP];
		}

	return;
	}
	
void update_stat(GW_StaticData *dst, GW_StaticData *src)
	{
	dst->accepted+=src->accepted;
	dst->errors_input_communication+=src->errors_input_communication;
	dst->errors_tcp_adu+=src->errors_tcp_adu;
	dst->errors_tcp_pdu+=src->errors_tcp_pdu;
	dst->errors_serial_sending+=src->errors_serial_sending;
	dst->errors_serial_accepting+=src->errors_serial_accepting;
	dst->timeouts+=src->timeouts;
	dst->crc_errors+=src->crc_errors;
	dst->errors_serial_adu+=src->errors_serial_adu;
	dst->errors_serial_pdu+=src->errors_serial_pdu;
	dst->errors_tcp_sending+=src->errors_tcp_sending;
	dst->errors+=src->errors;
	dst->sended+=src->sended;

	int i;
	for(i=0; i<STAT_FUNC_AMOUNT; i++) {
		dst->func[i][STAT_RES_OK]+= 	src->func[i][STAT_RES_OK];
		dst->func[i][STAT_RES_ERR]+=	src->func[i][STAT_RES_ERR];
		dst->func[i][STAT_RES_EXP]+=	src->func[i][STAT_RES_EXP];
		}

	return;
	}
	
void clear_stat(GW_StaticData *dst)
	{
	dst->accepted=\
	dst->errors_input_communication=\
	dst->errors_tcp_adu=\
	dst->errors_tcp_pdu=\
	dst->errors_serial_sending=\
	dst->errors_serial_accepting=\
	dst->timeouts=\
	dst->crc_errors=\
	dst->errors_serial_adu=\
	dst->errors_serial_pdu=\
	dst->errors_tcp_sending=\
	dst->errors=\
	dst->sended=0;

	memset(dst->func, 0, sizeof(dst->func));

	return;
	}

void func_res_ok(int mbf, GW_StaticData *dst)
	{

	static int res;
	res=(mbf&0x80)==0?STAT_RES_OK:STAT_RES_EXP;

	switch(mbf & 0x7f) {
		case MBF_READ_COILS:							dst->func[STAT_FUNC_0x01][res]++; break;
		case MBF_READ_DECRETE_INPUTS:			dst->func[STAT_FUNC_0x02][res]++; break;
		case MBF_READ_HOLDING_REGISTERS:	dst->func[STAT_FUNC_0x03][res]++; break;
		case MBF_READ_INPUT_REGISTERS:		dst->func[STAT_FUNC_0x04][res]++; break;

		case MBF_WRITE_SINGLE_COIL:					dst->func[STAT_FUNC_0x05][res]++; break;
		case MBF_WRITE_SINGLE_REGISTER:			dst->func[STAT_FUNC_0x06][res]++; break;
		case MBF_WRITE_MULTIPLE_COILS:			dst->func[STAT_FUNC_0x0f][res]++; break;
		case MBF_WRITE_MULTIPLE_REGISTERS:	dst->func[STAT_FUNC_0x10][res]++; break;

		default: dst->func[STAT_FUNC_OTHER][res]++;
		}

	return;
	}

void func_res_err(int mbf, GW_StaticData *dst)
	{

	static int res;
	res=(mbf&0x80)==0?STAT_RES_ERR:STAT_RES_EXP;

	switch(mbf & 0x7f) {
		case MBF_READ_COILS:							dst->func[STAT_FUNC_0x01][res]++; break;
		case MBF_READ_DECRETE_INPUTS:			dst->func[STAT_FUNC_0x02][res]++; break;
		case MBF_READ_HOLDING_REGISTERS:	dst->func[STAT_FUNC_0x03][res]++; break;
		case MBF_READ_INPUT_REGISTERS:		dst->func[STAT_FUNC_0x04][res]++; break;

		case MBF_WRITE_SINGLE_COIL:					dst->func[STAT_FUNC_0x05][res]++; break;
		case MBF_WRITE_SINGLE_REGISTER:			dst->func[STAT_FUNC_0x06][res]++; break;
		case MBF_WRITE_MULTIPLE_COILS:			dst->func[STAT_FUNC_0x0f][res]++; break;
		case MBF_WRITE_MULTIPLE_REGISTERS:	dst->func[STAT_FUNC_0x10][res]++; break;

		default: dst->func[STAT_FUNC_OTHER][res]++;
		}

	return;
	}

//*************************************************************************************

void init_message_templates()
  {
	memset(message_template, 0, sizeof(message_template));

  /// COMAND LINE ( ŒÃ¿ÕƒÕ¿ﬂ —“–Œ ¿) [1..24, 24]
strcpy(message_template[  1], "CMD LINE: NO PARAMETERS");
strcpy(message_template[  2], "CMD LINE: INVALID AMOUNT OF PARAMETERS");
strcpy(message_template[  3], "CMD LINE: KEYWORD PORT NOT FOUND");
strcpy(message_template[  4], "CMD LINE: MUTUALY EXLUSIVE PARAMETERS");
strcpy(message_template[  5], "CMD LINE: PARAMETER NOT ALLOWED");
strcpy(message_template[  6], "CMD LINE: TOO LOW PARAMETERS");
strcpy(message_template[  7], "CMD LINE: WRONG GATEWAY MODE");
strcpy(message_template[  8], "CMD LINE: WRONG MAP DATA");
strcpy(message_template[  9], "CMD LINE: WRONG PORT PARAMETERS");
strcpy(message_template[ 10], "CMD LINE: WRONG PROXY_TABLE");
strcpy(message_template[ 11], "CMD LINE: WRONG RTM_TABLE");
strcpy(message_template[ 12], "CMD LINE: WRONG TCP_SERVERS");
strcpy(message_template[ 13], "CMD LINE: OK");
strcpy(message_template[ 14], "CMD LINE: UNCERTAIN RESULT");

	/// SYSTEM (—»—“≈ÃÕ€≈) [25..64, 40]
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
strcpy(message_template[ 38], ""); // –≈«≈–¬
strcpy(message_template[ 39], "STATUS INFO OVERLAPS");
strcpy(message_template[ 40], "SERIAL PORT INITIALIZED MODE %s");
strcpy(message_template[ 41], "THREAD INITIALIZED CODE %d");
strcpy(message_template[ 42], "THREAD STARTED MODE %s CLIENT %s");
strcpy(message_template[ 43], "THREAD STOPPED");
strcpy(message_template[ 44], "PROGRAM TERMINATED (WORKTIME %d)");
	
	/// CONNECTION (—≈“≈¬Œ≈ —Œ≈ƒ»Õ≈Õ»≈) [65..127, 63]
strcpy(message_template[ 65], "SOCKET INITIALIZED STAGE %d TCPSERVER %s");
strcpy(message_template[ 66], ""); // –≈«≈–¬
strcpy(message_template[ 67], "CONNECTION ACCEPTED FROM %s CLIENT %d");
strcpy(message_template[ 68], "CONNECTION ESTABLISHED WITH %d.%d.%d.%d");
strcpy(message_template[ 69], "CONNECTION FAILED TO %d.%d.%d.%d");
strcpy(message_template[ 70], "CONNECTION REJECTED FROM %d.%d.%d.%d");
strcpy(message_template[ 71], "CONNECTION CLOSED (LINK DOWN) CLIENT %d");
strcpy(message_template[ 72], "CONNECTION CLOSED (TIMEOUT) CLIENT %d");
	
	/// FORWARDING (œ≈–≈Õ¿œ–¿¬À≈Õ»≈) [128..147, 20]
	strcpy(message_template[128], "FRWD: ADDRESS TRANSLATION [CLIENT %d ADDRESS %d]");
	strcpy(message_template[129], "FRWD: BLOCK OVERLAPS [CLIENT %d START %d LEN %d]");
	strcpy(message_template[130], "FRWD: PROXY TRANSLATION [STATUS %d PORT %d CLIENT %d]");
	strcpy(message_template[131], "FRWD: REGISTERS TRANSLATION [CLIENT %d START %d LEN %d]");
	
	/// QUEUE (Œ◊≈–≈ƒ‹) [148..179, 32]
	strcpy(message_template[148], "QUEUE EMPTY %s");
	strcpy(message_template[149], "QUEUE OVERLOADED %s [CLIENT %d]");
	
	/// POLLING (Œœ–Œ—) [180..191, 12]
	strcpy(message_template[180], "POLLING: FUNC NOT SUPPORTED [FUNC %d]");
	strcpy(message_template[181], "POLLING: REQUEST TIMED OUT"); ///!!! ÒÓÓ·˘ÂÌËÂ ÓÚÒÛÚÒÚ‚ÛÂÚ, ‰Ó·‡‚ËÚ¸ ÍÓ‰
	strcpy(message_template[182], "POLLING: RTU RECV [CODE %d CLIENT %d]");
	strcpy(message_template[183], "POLLING: RTU SEND [CODE %d CLIENT %d]");
	strcpy(message_template[184], "POLLING: TCP RECV [CODE %d CLIENT %d]");
	strcpy(message_template[185], "POLLING: TCP SEND [CODE %d CLIENT %d]");
	strcpy(message_template[186], "POLLING: QUEUE IN [CODE %d CLIENT %d]");
	strcpy(message_template[187], "POLLING: QUEUE OUT [CODE %d CLIENT %d]");

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
	
	/// ÙËÎ¸ÚÛÂÏ ÒÓÓ·˘ÂÌËˇ
	if((
		gate502.msg_filter & 
		(0x01 << ((msgtype & EVENT_SRC_MASK)-1))
		)==0) return;
	if((msgtype & EVENT_CAT_MASK)==EVENT_CAT_TRAFFIC) return;
	
	if(gate502.show_sys_messages==0 && (msgtype & EVENT_CAT_MASK)==EVENT_CAT_DEBUG) return;
	
	/// ÒÓÁ‰‡ÂÏ Á‡ÔËÒ¸ ‚ ÊÛÌ‡ÎÂ
	time(&curtime);
	if(app_log!=NULL) {
		app_log[gate502.app_log_current_entry].time=curtime;
		app_log[gate502.app_log_current_entry].msgtype=msgtype;
		app_log[gate502.app_log_current_entry].msgcode=msgcode;
		
		app_log[gate502.app_log_current_entry].prm[0]=prm1;
		app_log[gate502.app_log_current_entry].prm[1]=prm2;
		app_log[gate502.app_log_current_entry].prm[2]=prm3;
		app_log[gate502.app_log_current_entry].prm[3]=prm4;
		
		gate502.app_log_entries_total++;
	  } else printf("Event log isn't initialized!\n");
	
	/// ‚˚‚Ó‰ËÏ ÒÓ·˚ÚËÂ Ì‡ ÍÓÌÒÓÎ¸
	
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
			case EVENT_SRC_SYSTEM: 	printf("SYSTEM\t"); 	break;
			case EVENT_SRC_GATE502: printf("GATE502\t"); 	break;
			case EVENT_SRC_P1: 			printf("PORT1\t"); 		break;
			case EVENT_SRC_P2: 			printf("PORT2\t"); 		break;
			case EVENT_SRC_P3: 			printf("PORT3\t"); 		break;
			case EVENT_SRC_P4: 			printf("PORT4\t"); 		break;
			case EVENT_SRC_P5: 			printf("PORT5\t"); 		break;
			case EVENT_SRC_P6: 			printf("PORT6\t"); 		break;
			case EVENT_SRC_P7: 			printf("PORT7\t"); 		break;
			case EVENT_SRC_P8: 			printf("PORT8\t"); 		break;
			case EVENT_SRC_MOXAMB: 	printf("MOXAMB\t"); 	break;
			case EVENT_SRC_MOXATCP: printf("MOXATCP\t"); 	break;

			default: 								printf("***\t");
			}

	make_msgstr(msgcode, eventmsg, prm1, prm2, prm3, prm4);
	printf("%s\n", eventmsg);

	// ËÌÍÂÏÂÌÚËÛÂÏ Ò˜ÂÚ˜ËÍ ÍÓÎ¸ˆÂ‚Ó„Ó ·ÛÙÂ‡ ÒÓÓ·˘ÂÌËÈ
	if(app_log!=NULL)
		gate502.app_log_current_entry=\
			gate502.app_log_current_entry==EVENT_LOG_LENGTH-1?\
			0:gate502.app_log_current_entry+1;
	
	return;
	}

///-----------------------------------------------------------------------------------------------------------------

void show_traffic(int traffic, int port_id, int client_id, u8 *adu, u16 adu_len)
  {

	/// POLLING (Œœ–Œ—) [180..191, 12]

//  printf("TCP%4.4d  IN: ", inputDATA->clients[client_id].port);	

//	if(gate502.show_data_flow==1) {
//    for (i=0;i<cur_pos;i++) printf("[%0.2X]",adu[i]);
//    printf("\n");
//		}

//	if(gate502.show_data_flow==1) {
//    int i;
//    for (i=0;i<mb_received_adu_len;i++) printf("[%0.2X]",adu[i]);
//		printf("\n");
//	  }

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

///--- COMAND LINE ( ŒÃ¿ÕƒÕ¿ﬂ —“–Œ ¿) [1..24, 24]

///--- SYSTEM (—»—“≈ÃÕ€≈) [25..64, 40]

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
			case BRIDGE_SIMPLE: 	sprintf(str, message_template[msgcode], "BRIDGE SIMPLE"); break;
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
			case BRIDGE_SIMPLE: 	sprintf(str, message_template[msgcode], "BRIDGE SIMPLE", "RTU"); break;
			case MODBUS_PORT_ERROR: sprintf(str, message_template[msgcode], "PORT ERROR", "N/A"); break;
			default: 							sprintf(str, message_template[msgcode], "PORT OFF", "N/A");
			}

	if(msgcode==44) // PROGRAM TERMINATED (WORKTIME %d)
			sprintf(str, message_template[msgcode], prm1);

///--- CONNECTION (—≈“≈¬Œ≈ —Œ≈ƒ»Õ≈Õ»≈) [65..127, 63]

	if(msgcode==65) // SOCKET INITIALIZED STAGE %d TCPSERVER %s;
		if(prm2==DEFAULT_CLIENT) 	sprintf(str, message_template[msgcode], prm1, "N/A");
		  else  									sprintf(str, message_template[msgcode], prm1, "xxx.xxx.xxx.xxx");

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

///--- FORWARDING (œ≈–≈Õ¿œ–¿¬À≈Õ»≈) [128..147, 20]

///--- QUEUE (Œ◊≈–≈ƒ‹) [148..179, 32]

///--- ¬—≈ œ–Œ◊»≈ —ŒŒ¡Ÿ≈Õ»ﬂ ¡≈« œ¿–¿Ã≈“–Œ¬
  if(	msgcode==43 || msgcode==39 ||
		  (msgcode<=36 && msgcode>=29) ||
		  (msgcode<=27 && msgcode>=25) ||
		  (msgcode<=14 && msgcode>=1))
		sprintf(str, message_template[msgcode]);

	return;
	}

///-----------------------------------------------------------------------------------------------------------------
