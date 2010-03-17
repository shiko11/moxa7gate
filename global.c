/*
MOXA7GATE MODBUS GATEWAY SOFTWARE
SEM-ENGINEERING
                    BRYANSK 2009
*/

#include <string.h>
#include <stdlib.h>
#include "global.h"

#define DEBUG_CL //ЮЙУФП ДМС ПФМБДЛЙ БМЗПТЙФНБ ЙОФЕТЕРТЕФБГЙЙ ЛПНБОДОПК УФТПЛЙ
//#define DEBUG_WORK //ЮЙУФП ДМС ПФМБДЛЙ ТБВПФЩ Ж-ГЙЙ *run_work 

///-------------------------------------------------------------------------
void	help_print (void) //ЧЩЧПД УРТБЧЛЙ
{
printf("%s\n", "\
Usage: moxa7gate [OPTIONS] [PORTx mode speed parity timeout GATEWAY tcp_port]...\n\n\
Opens serial port(s) for transfer Modbus messages to communicate with modbus-enabled devices.\n\n\
OPTIONS:\n\n\
--help               - display this help and exit\n\
--version            - output version information and exit\n\
--show_data_flow     - prints Modbus requests and responses\n\
--show_sys_messages  - prints messages for debug purposes\n\
--single_gateway_port_502	- use only 502 tcp port\n\n\
PORT sections:\n\n\
x        - serial port number 1..8\n\
mode     - port mode (RS-232, RS-485 etc.)\n\
speed    - baud rate\n\
parity   - parity (none, even, odd)\n\
timeout  - communication timeout (500000...10000000)\n\
tcp_port - TCP port for accepting incoming\n\
           connections from Modbus-TCP clients\n\n\
Read the manual for details.\n\
");
return;
}

///-------------------------------------------------------------------------
void version_print (void) //ЧЩЧПД ЛТБФЛПЗП ПРЙУБОЙС РТПЗТБННЩ
{
printf("%s\n", "\
MOXA7GATE R1.00\n\
Modbus gateway\n\
     software\n\
SEM-ENGINEERING\n\
+7(4832)41-88-23\n\
www.semgroup.ru\n\
   Bryansk 2009\n\
");
	return;
}

///-------------------------------------------------------------------------
int get_command_line (int 	argc,
											char	*argv[],
											input_cfg *ptr_iDATA,
											RT_Table_Entry *vslave,
											Query_Table_Entry *query_table,
											u8	*show_data_flow,
											u8	*show_sys_messages,
											u8	*single_gateway_port_502,
											u8  *single_address_space,
											u8  *proxy_mode
											) //ПВТБВПФЛБ РБТБНЕФТПЧ ЛПНБОДОПК УФТПЛЙ
{
	//РТПЧЕТЛБ ОБ РХУФХА ЛПНБОДОХА УФТПЛХ
	if (argc == 1) return CL_ERR_NONE_PARAM;

	u16		i;
	u16		j;
	u16		k;
	u16		p_cnt	= 0;
	u16		key_cnt	= 0;
	u16		id_p_argc[MAX_MOXA_PORTS];
	u16		id_key_argc[MAX_KEYS];
	
	memset(id_p_argc,0,sizeof(id_p_argc));
	*show_data_flow=0;
	*show_sys_messages=0;
	*single_gateway_port_502=0;
	*single_address_space=0;
	*proxy_mode=0;
	
	//-----------------------------------
	//РПЙУЛ Й ПВТБВПФЛБ ЛМАЮЕК ЛПНБОДОПК УФТПЛЙ
	for (i=0;i<argc;i++) {
		if (strncmp(argv[i],"--",2)==0) {
			id_key_argc[key_cnt++] = i;
//			printf("key %d index %d\n", key_cnt-1, i);
		}
	}
	
	for (i=0;i<key_cnt;i++) {
//		printf("%s\n", argv[id_key_argc[i]]);
		if (strcmp(argv[id_key_argc[i]],"--help")==0) {help_print(); return CL_INFO;}
		if (strcmp(argv[id_key_argc[i]],"--version")==0) {version_print(); return CL_INFO;}
		if (strcmp(argv[id_key_argc[i]],"--show_data_flow")==0) *show_data_flow=1;
		if (strcmp(argv[id_key_argc[i]],"--show_sys_messages")==0) *show_sys_messages=1;
		if (strcmp(argv[id_key_argc[i]],"--single_gateway_port_502")==0) *single_gateway_port_502=1;
		if (strcmp(argv[id_key_argc[i]],"--single_address_space")==0) *single_address_space=1;
		if (strcmp(argv[id_key_argc[i]],"--proxy_mode")==0) *proxy_mode=1;
	}

	if(
			((*single_gateway_port_502==1)&&(*single_address_space==1)) ||
			((*single_gateway_port_502==1)&&(*proxy_mode==1)) ||
			((*single_address_space==1)&&(*proxy_mode==1))
			) return CL_ERR_MUTEX_PARAM;

  if(*proxy_mode==1) *single_address_space=1; // используем пока уже готовые маханизмы в программе

	//printf("single_gateway_port_502=\n", (*single_gateway_port_502));
	//printf("single_address_space=\n", (*single_address_space));
	//printf("proxy_mode=\n", (*proxy_mode));

	//------------------------------------
	//РПЙУЛ ЛМАЮЕЧЩИ УМПЧ PORTx (Ф.Е. ОБЮБМБ УЕЛГЙК)
	char		dest[16];
	char		tmp[2];

	for (i=0;i<MAX_MOXA_PORTS;i++){
		memcpy(dest,NAME_MOXA_PORT,5);
		sprintf(tmp,"%d",i+1);
		strcat(dest,tmp);
		for (j=0;j<argc;j++)
			if (strncmp(argv[j],dest,5)==0){
				id_p_argc[p_cnt]=j;
				if(p_cnt!=0) if(id_p_argc[p_cnt]-id_p_argc[p_cnt-1]<7) return CL_ERR_MIN_PARAM;
//				printf("port %d position %d\n", p_cnt, j);
				p_cnt++;
				break;
			}
	}

//РТПЧЕТЛБ ОБ ОБМЙЮЙЕ Й РТБЧЙМШОПУФШ ЛМАЮЕЧЩИ УМПЧ ЛПНБОДОПК УФТПЛЙ
	if (p_cnt==0) return CL_ERR_PORT_WORD;

	/// читаем таблицу виртуральных устройств и таблицу опроса
	int id_vslaves=0, vslaves_num;
	int id_qt=0, qt_entries_num;

	for (j=0;j<argc;j++) {
		if (strcmp(argv[j],"VSLAVES")==0) {
			if(id_vslaves==0) id_vslaves=j;
				else return CL_ERR_VSLAVES_CFG;
			}
		if (strcmp(argv[j],"QUERY_TABLE")==0) {
			if(id_qt==0) id_qt=j;
				else return CL_ERR_QT_CFG;
			}
		}

	/// parsing serial port parameters
	int p_num;
	char *arg;
	for(i=0; i<p_cnt; i++) {

		//port number
		arg=argv[id_p_argc[i]+0];
		p_num=arg[4]-48;
		if((p_num<1)||(p_num>MAX_MOXA_PORTS)) continue;
//		strcpy(ptr_iDATA[p_num-1].bridge_status, "ERR");

//		printf("p_num %d\n", p_num);
		//port mode
		arg=argv[id_p_argc[i]+1];
		strcpy(ptr_iDATA[p_num-1].serial.p_mode, arg);
//		printf("%s\n", ptr_iDATA[p_num-1].serial.p_mode);
		//port speed
		arg=argv[id_p_argc[i]+2];
		strcpy(ptr_iDATA[p_num-1].serial.speed, arg);
//		printf("%s\n", ptr_iDATA[p_num-1].serial.speed);
		// parity		
		arg=argv[id_p_argc[i]+3];
		//strcpy(ptr_iDATA[p_num-1].serial.parity, arg);
		k=strlen(arg);
		for(j=0; j<k; j++) ptr_iDATA[p_num-1].serial.parity[j]=tolower(arg[j]);
		ptr_iDATA[p_num-1].serial.parity[j]=0;
//		printf("%s\n", ptr_iDATA[p_num-1].serial.parity);
		// timeout
		arg=argv[id_p_argc[i]+4];
		sscanf(arg, "%d", &ptr_iDATA[p_num-1].serial.timeout);
//		printf("%d\n", ptr_iDATA[p_num-1].serial.timeout);

		// GATE MODE
		arg=argv[id_p_argc[i]+5];
		//strcpy(ptr_iDATA[p_num-1].serial.parity, arg);
		k=strlen(arg);
		for(j=0; j<k; j++) arg[j]=toupper(arg[j]);
		if(strcmp(arg, "GATEWAY")==0) ptr_iDATA[p_num-1].modbus_mode=MODBUS_GATEWAY_MODE; else
		if(strcmp(arg, "BRIDGE")==0) ptr_iDATA[p_num-1].modbus_mode=MODBUS_BRIDGE_MODE; else
		if(strcmp(arg, "MASTER")==0) ptr_iDATA[p_num-1].modbus_mode=MODBUS_PROXY_MODE; else return CL_ERR_GATEWAY_MODE;
//		printf("%s\n", ptr_iDATA[p_num-1].serial.parity);

		int temp;
		int dot, colon, digit, port, client;
		unsigned char ip1, ip2, ip3, ip4;
		char addr[16];
		int shift_counter;

		if(check_gate_settings(&ptr_iDATA[p_num-1])) return CL_ERR_IN_PORT_SETT;

		printf("parsing port %d, mode %d\n", p_num, ptr_iDATA[p_num-1].modbus_mode);

		switch(ptr_iDATA[p_num-1].modbus_mode) {
			case MODBUS_GATEWAY_MODE:
				// tcp_port
				arg=argv[id_p_argc[i]+6];
				sscanf(arg, "%d", &temp);
				if((temp<502) || (temp>32000)) return CL_ERR_IN_MAP;
				ptr_iDATA[p_num-1].tcp_port=temp;
		//		printf("%d\n", ptr_iDATA[p_num-1].tcp_port);
				break;

			case MODBUS_BRIDGE_MODE:
				
				shift_counter=0;
				for(client=0; client<MAX_TCP_CLIENTS_PER_PORT; client++) {
					
//					printf("id_p_argc[i]+6+client=%d\n", id_p_argc[i]+6+client);

					if(i<p_cnt-1) {if((id_p_argc[i]+6+2*client+1+shift_counter)>=id_p_argc[i+1]) continue;}
					  else if(((id_vslaves!=0) && (id_p_argc[i]+6+2*client+1+shift_counter)>=id_vslaves) ||
										((id_qt!=0) && (id_p_argc[i]+6+2*client+1+shift_counter)>=id_qt) ||
										((id_p_argc[i]+6+2*client+1+shift_counter)>=argc)
										) continue;
				
					//#include <arpa/inet.h> // стр. 350, "разработка приложений в Linux.djvu"
					//const char * inet_ntop(...);
					//int inet_pton(...);

					arg=argv[id_p_argc[i]+6+2*client+shift_counter];
					//printf("arg:%s\n", arg);
					dot=colon=digit=0;
					k=strlen(arg);
					for(j=0; j<k; j++)
					  if((arg[j]>=48)&&(arg[j]<=57)) digit++; else
					    if(arg[j]=='.') dot++; else
					      if(arg[j]==':') colon++; else return CL_ERR_IN_MAP;
					if((dot!=3)||(colon!=1))  return CL_ERR_IN_MAP;
					
					digit=0;
					for(j=digit; j<k; j++) if(arg[j]=='.') {arg[j]=0; break;}
					strcpy(addr, &arg[digit]);
					ip4=(unsigned char) atoi(addr);
					digit=j+1;
					arg[j]='.';
					
					for(j=digit; j<k; j++) if(arg[j]=='.') {arg[j]=0; break;}
					strcpy(addr, &arg[digit]);
					ip3=(unsigned char) atoi(addr);
					digit=j+1;
					arg[j]='.';
					
					for(j=digit; j<k; j++) if(arg[j]=='.') {arg[j]=0; break;}
					strcpy(addr, &arg[digit]);
					ip2=(unsigned char) atoi(addr);
					digit=j+1;
					arg[j]='.';
					
					for(j=digit; j<k; j++) if(arg[j]==':') {arg[j]=0; break;}
					strcpy(addr, &arg[digit]);
					ip1=(unsigned char) atoi(addr);
					digit=j+1;
					arg[j]=':';
					
					strcpy(addr, &arg[digit]);
					port=atoi(addr);
	
					ptr_iDATA[p_num-1].clients[client].ip=ip1+(ip2<<8)+(ip3<<16)+(ip4<<24);
					ptr_iDATA[p_num-1].clients[client].port=port;
					
					printf("I:  %s\n", arg);
					printf("II: %X:%d\n", ptr_iDATA[p_num-1].clients[client].ip, ptr_iDATA[p_num-1].clients[client].port);

					/// читаем modbus-адрес устройства
					arg=argv[id_p_argc[i]+6+2*client+1+shift_counter];
					temp=atoi(arg);
					if((temp<1)||(temp>247)) return CL_ERR_IN_MAP;
					ptr_iDATA[p_num-1].clients[client].mb_slave=temp;
					
					/// сдвиг адресного пространства
					arg=argv[id_p_argc[i]+6+2*client+2+shift_counter];
					//printf("%d-%d-%s\n", argc, id_p_argc[i]+6+2*client+2+shift_counter, arg);
					if(argc>id_p_argc[i]+6+2*client+2+shift_counter)
					if(strcmp(arg, "--address_shift")==0) {
						arg=argv[id_p_argc[i]+6+2*client+3+shift_counter];
						temp=atoi(arg);
						//if((temp<1)||(temp>247)) return CL_ERR_IN_MAP;
						ptr_iDATA[p_num-1].clients[client].address_shift=temp;
						shift_counter+=2;
						}
					
					/// общее количество сетевых соединений
	  			ptr_iDATA[p_num-1].accepted_connections_number++;
					}
				
				break;

			case MODBUS_PROXY_MODE: ///!!!
			default: ptr_iDATA[p_num-1].modbus_mode=MODBUS_PORT_OFF;
		}

		strcpy(ptr_iDATA[p_num-1].bridge_status, "INI");
	  }

	//------------------------------------
	/// читаем таблицу виртуральных устройств и таблицу опроса

	if(id_vslaves!=0) {
	
		sscanf(argv[id_vslaves+1], "%d", &vslaves_num);
		if((vslaves_num<1)||(vslaves_num>MAX_VIRTUAL_SLAVES)) return CL_ERR_VSLAVES_CFG;
	
		for(i=0; i<vslaves_num; i++) {
	
			arg=argv[id_vslaves+i*4+2];
			sscanf(arg, "%d", &vslave[i].start);
	//		printf("%d\n", ptr_iDATA[p_num-1].serial.timeout);
	
			arg=argv[id_vslaves+i*4+3];
			sscanf(arg, "%d", &vslave[i].length);
	//		printf("%d\n", ptr_iDATA[p_num-1].serial.timeout);
	
			arg=argv[id_vslaves+i*4+4];
			vslave[i].port=arg[1]-48-1;
	
			arg=argv[id_vslaves+i*4+5];
			sscanf(arg, "%d", &vslave[i].device);
	//		printf("%d\n", ptr_iDATA[p_num-1].serial.timeout);
	
		  }

	  int res=verify_vslaves();
	  if(res!=0) return CL_ERR_VSLAVES_CFG;
		}

	if(id_qt!=0) {
	
		sscanf(argv[id_qt+1], "%d", &qt_entries_num);
		if((qt_entries_num<1)||(qt_entries_num>MAX_QUERY_ENTRIES)) return CL_ERR_QT_CFG;
	
		for(i=0; i<qt_entries_num; i++) {
	
			arg=argv[id_qt+i*9+2];
			sscanf(arg, "%d", &query_table[i].start);
	//		printf("%d\n", ptr_iDATA[p_num-1].serial.timeout);
	
			arg=argv[id_qt+i*9+3];
			sscanf(arg, "%d", &query_table[i].length);
	//		printf("%d\n", ptr_iDATA[p_num-1].serial.timeout);
	
			arg=argv[id_qt+i*9+4];
			sscanf(arg, "%d", &query_table[i].offset);
	//		printf("%d\n", ptr_iDATA[p_num-1].serial.timeout);
	
			arg=argv[id_qt+i*9+5];
			query_table[i].port=arg[1]-48-1;
	
			arg=argv[id_qt+i*9+6];
			sscanf(arg, "%d", &query_table[i].device);
	//		printf("%d\n", ptr_iDATA[p_num-1].serial.timeout);
	
		arg=argv[id_qt+i*9+7];
		//strcpy(ptr_iDATA[p_num-1].serial.parity, arg);
		k=strlen(arg);
		for(j=0; j<k; j++) arg[j]=toupper(arg[j]);
		if(strcmp(arg, "COIL_STATUS")==0) query_table[i].mbf=MBF_READ_COILS; else
		if(strcmp(arg, "INPUT_STATUS")==0) query_table[i].mbf=MBF_READ_DECRETE_INPUTS; else
		if(strcmp(arg, "HOLDING_REGISTER")==0) query_table[i].mbf=MBF_READ_HOLDING_REGISTERS; else
		if(strcmp(arg, "INPUT_REGISTER")==0) query_table[i].mbf=MBF_READ_INPUT_REGISTERS; else return CL_ERR_QT_CFG;
//		printf("%s\n", ptr_iDATA[p_num-1].serial.parity);

			arg=argv[id_qt+i*9+8];
			sscanf(arg, "%d", &query_table[i].delay);
	//		printf("%d\n", ptr_iDATA[p_num-1].serial.timeout);
	
			arg=argv[id_qt+i*9+9];
			sscanf(arg, "%d", &query_table[i].status_register);
	//		printf("%d\n", ptr_iDATA[p_num-1].serial.timeout);
	
			arg=argv[id_qt+i*9+10];
			sscanf(arg, "%d", &query_table[i].status_bit);
	//		printf("%d\n", ptr_iDATA[p_num-1].serial.timeout);
	
		  }

	  int res=verify_proxy_queries();
	  if(res!=0) return CL_ERR_QT_CFG;
		}

	return CL_OK;
	}

///-------------------------------------------------------------------------
int check_gate_settings(input_cfg *data)
	{
	
	if(!(
			strcmp(data->serial.p_mode, "RS232")==0 ||
			strcmp(data->serial.p_mode, "RS422")==0 ||
			strcmp(data->serial.p_mode, "RS485_2w")==0 ||
			strcmp(data->serial.p_mode, "RS485_4w")==0
		)) return 1;
	
	if(!(
			strcmp(data->serial.speed, "2400")==0 ||
			strcmp(data->serial.speed, "4800")==0 ||
			strcmp(data->serial.speed, "9600")==0 ||
			strcmp(data->serial.speed, "14400")==0 ||
			strcmp(data->serial.speed, "19200")==0 ||
			strcmp(data->serial.speed, "38400")==0 ||
			strcmp(data->serial.speed, "56000")==0 ||
			strcmp(data->serial.speed, "57600")==0 ||
			strcmp(data->serial.speed, "115200")==0
		)) return 2;

	if(!(
			strcmp(data->serial.parity, "none")==0 ||
			strcmp(data->serial.parity, "even")==0 ||
			strcmp(data->serial.parity, "odd")==0
		)) return 3;

	if((data->serial.timeout<200000) || (data->serial.timeout>10000000)) return 4;

	return 0;
	}
///----------------------------------------------------------------------------------------------------------------
void sigpipe_handler()
{
printf("SIGPIPE! \n");
return;
}

void sigio_handler()
{
printf("SIGIO! \n");
return;
}
///----------------------------------------------------------------------------------------------------------------
int translateAddress(u8 unit_id, int *port_id, int *device_id)
  {

	*port_id=unit_id/30;
	*device_id=unit_id-*port_id*30;
	
  return 1;
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

	*port_id=vslave[i].port;
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

	*port_id=query_table[i].port;
	*device_id=i; // индекс блока адресов виртуального устройства
	
  return 0;
  }

///----------------------------------------------------------------------------------------------------------------
///----------------------------------------------------------------------------------------------------------------
int verify_vslaves()
  {
	int i;

  for(i=0; i<MAX_VIRTUAL_SLAVES; i++) {
    if(
//			(vslave[i].start==0) ||
			(vslave[i].length==0) ||
//			(vslave[i].port==SERIAL_STUB) ||
			(vslave[i].device==0)
			) continue;

			if(vslave[i].start>65534) return 1;
			if((vslave[i].length==0)||(vslave[i].length>65535)) return 2;
			if((vslave[i].start+vslave[i].length)>65535) return 3;
			if(vslave[i].port>SERIAL_P8) return 4;
			if((vslave[i].device==0)||(vslave[i].device>247)) return 5;

			}
  

	return 0;
  }
///----------------------------------------------------------------------------------------------------------------
int verify_proxy_queries()
  {
	int i;

  for(i=0; i<MAX_QUERY_ENTRIES; i++) {
    if(
			(query_table[i].length==0) ||
			(query_table[i].mbf==0) ||
			(query_table[i].device==0)
			) continue;

			if(query_table[i].start>65534) return 11;
			if((query_table[i].length==0)||(query_table[i].length>70)) return 12; ///!!!
			if((query_table[i].start+query_table[i].length)>65535) return 13;
			if(query_table[i].port>SERIAL_P8) return 14;
			if((query_table[i].device==0)||(query_table[i].device>247)) return 15;

			if((query_table[i].offset+query_table[i].length)>=PROXY_MODE_REGISTERS) return 16;
			}
	return 0;
  }
///----------------------------------------------------------------------------------------------------------------
