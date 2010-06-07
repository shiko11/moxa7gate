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
void	help_print (void) /* вывод справки */
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
void version_print (void) /* вывод информации о программе */
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
											input_cfg_502 *ptr_gate502,
											RT_Table_Entry *vslave,
											Query_Table_Entry *query_table,
											GW_TCP_Server *tcp_servers
											) //обработка параметров командной строки
{
	//проверка на пустую командную строку
	if (argc == 1) return CL_ERR_NONE_PARAM;

	u16		i;
	u16		j;
	unsigned		k;
	u16		p_cnt	= 0;
	u16		key_cnt	= 0;
	u16		id_p_argc[MAX_MOXA_PORTS];
	u16		id_key_argc[MAX_KEYS];
	
	memset(id_p_argc,0,sizeof(id_p_argc));
///	*single_gateway_port_502=0;
///	*single_address_space=0;
///	*proxy_mode=0;
	
	//-----------------------------------
	//поиск и обработка ключей командной строки, задающих глобальные параметры шлюза

	for (i=0; i<argc; i++) {
		if (strncmp(argv[i],"--",2)==0) {
			if(key_cnt==MAX_KEYS) return CL_ERR_NOT_ALLOWED;
			id_key_argc[key_cnt++] = i;
//			printf("key %d index %d\n", key_cnt-1, i);
		}
//  printf("%s\n", argv[i]);
	}

//  printf("stopping program...\n"); exit(1);
	for (i=0; i<key_cnt; i++) {
//		printf("%s\n", argv[id_key_argc[i]]);
		if (strcmp(argv[id_key_argc[i]],"--help")==0) {help_print(); return CL_INFO;}
		if (strcmp(argv[id_key_argc[i]],"--version")==0) {version_print(); return CL_INFO;}

		if (strcmp(argv[id_key_argc[i]],"--show_data_flow")==0) ptr_gate502->show_data_flow=1;
		if (strcmp(argv[id_key_argc[i]],"--show_sys_messages")==0) ptr_gate502->show_sys_messages=1;
		if (strcmp(argv[id_key_argc[i]],"--watchdog_timer")==0) ptr_gate502->watchdog_timer=1;
		if (strcmp(argv[id_key_argc[i]],"--use_buzzer")==0) ptr_gate502->use_buzzer=1;

//		if (strcmp(argv[id_key_argc[i]],"--single_gateway_port_502")==0) *single_gateway_port_502=1;
		if (strcmp(argv[id_key_argc[i]],"--single_gateway_port_502")==0) return CL_ERR_NOT_ALLOWED;
//		if (strcmp(argv[id_key_argc[i]],"--single_address_space")==0) *single_address_space=1;
		if (strcmp(argv[id_key_argc[i]],"--single_address_space")==0) return CL_ERR_NOT_ALLOWED;
//		if (strcmp(argv[id_key_argc[i]],"--proxy_mode")==0) *proxy_mode=1;
		if (strcmp(argv[id_key_argc[i]],"--proxy_mode")==0) return CL_ERR_NOT_ALLOWED;

		if (strcmp(argv[id_key_argc[i]],"--tcp_port")==0) {
			ptr_gate502->tcp_port=atoi(argv[id_key_argc[i]+1]);
			if(ptr_gate502->tcp_port==0) return CL_ERR_IN_MAP;					
			}
		if (strcmp(argv[id_key_argc[i]],"--modbus_address")==0) {
			ptr_gate502->modbus_address=atoi(argv[id_key_argc[i]+1]);
			if(ptr_gate502->modbus_address==0) return CL_ERR_IN_MAP;					
			}
		if (strcmp(argv[id_key_argc[i]],"--status_info")==0) {
			ptr_gate502->status_info=atoi(argv[id_key_argc[i]+1]);
			if(ptr_gate502->status_info==0) return CL_ERR_IN_MAP;					
			}

		if (strcmp(argv[id_key_argc[i]],"--Object")==0)
			strcpy(ptr_gate502->object, argv[id_key_argc[i]+1]);
		if (strcmp(argv[id_key_argc[i]],"--Location")==0)
			strcpy(ptr_gate502->location, argv[id_key_argc[i]+1]);
		if (strcmp(argv[id_key_argc[i]],"--confVersion")==0)
			strcpy(ptr_gate502->version, argv[id_key_argc[i]+1]);
		if (strcmp(argv[id_key_argc[i]],"--NetworkName")==0)
			strcpy(ptr_gate502->networkName, argv[id_key_argc[i]+1]);
		if (strcmp(argv[id_key_argc[i]],"--NetworkAddress")==0)
			get_ip_from_string(argv[id_key_argc[i]+1], &ptr_gate502->IPAddress, &k);
	}

	//------------------------------------
	//поиск ключевых слов PORTx (т.е. начала секций)
	char		dest[16];
	char		tmp[2];

	for (i=0; i<MAX_MOXA_PORTS; i++){
		memcpy(dest,NAME_MOXA_PORT,5);
		sprintf(tmp,"%d",i+1);
		strcat(dest,tmp);
		for (j=0;j<argc;j++)
			if (strncmp(argv[j],dest,5)==0){
				id_p_argc[p_cnt]=j;
				if(p_cnt!=0) if(id_p_argc[p_cnt]-id_p_argc[p_cnt-1]<6) return CL_ERR_MIN_PARAM;
//				printf("port %d position %d\n", p_cnt, j);
				p_cnt++;
				break;
			}
	}

//проверка на наличие и правильность ключевых слов командной строки
	if (p_cnt==0) return CL_ERR_PORT_WORD;

	/// читаем таблицу виртуральных устройств и таблицу опроса
	int id_vslaves=0, vslaves_num;
	int id_qt=0, qt_entries_num;
	int id_tcpsrv=0, tcpsrv_entries_num;

	for (j=0;j<argc;j++) {
		if (strcmp(argv[j],"RTM_TABLE")==0) {
			if(id_vslaves==0) id_vslaves=j;
				else return CL_ERR_VSLAVES_CFG;
			}
		if (strcmp(argv[j],"PROXY_TABLE")==0) {
			if(id_qt==0) id_qt=j;
				else return CL_ERR_QT_CFG;
			}
		if (strcmp(argv[j],"TCP_SERVERS")==0) {
			if(id_tcpsrv==0) id_tcpsrv=j;
				else return CL_ERR_TCPSRV_CFG;
			}
		}

	/// parsing serial port parameters
	int p_num;
	char *arg;
	int shift_counter;
		int temp;
		int port, client;


	for(i=0; i<p_cnt; i++) {

		//port number
		arg=argv[id_p_argc[i]+0];
		p_num=arg[4]-48;
		if((p_num<1)||(p_num>MAX_MOXA_PORTS)) continue;
//		strcpy(ptr_iDATA[p_num-1].bridge_status, "ERR");

//		printf("p_num %d\n", p_num);
		//port mode
		arg=argv[id_p_argc[i]+1];
///		strcpy(ptr_iDATA[p_num-1].serial.p_mode, arg);
		k=strlen(arg);
		for(j=0; j<k; j++) ptr_iDATA[p_num-1].serial.p_mode[j]=toupper(arg[j]);
		ptr_iDATA[p_num-1].serial.p_mode[j-1]=tolower(ptr_iDATA[p_num-1].serial.p_mode[j-1]);
		ptr_iDATA[p_num-1].serial.p_mode[j]=0;
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
		ptr_iDATA[p_num-1].serial.timeout*=1000;
//		printf("%d\n", ptr_iDATA[p_num-1].serial.timeout);

		// GATE MODE
		arg=argv[id_p_argc[i]+5];
		//strcpy(ptr_iDATA[p_num-1].serial.parity, arg);
		k=strlen(arg);
		for(j=0; j<k; j++) arg[j]=toupper(arg[j]);

		if(strcmp(arg, "GATEWAY_SIMPLE")==0) 	ptr_iDATA[p_num-1].modbus_mode=GATEWAY_SIMPLE;
		if(strcmp(arg, "GATEWAY_ATM")==0) 		ptr_iDATA[p_num-1].modbus_mode=GATEWAY_ATM;
		if(strcmp(arg, "GATEWAY_RTM")==0) 		ptr_iDATA[p_num-1].modbus_mode=GATEWAY_RTM;
		if(strcmp(arg, "GATEWAY_PROXY")==0) 	ptr_iDATA[p_num-1].modbus_mode=GATEWAY_PROXY;
		if(strcmp(arg, "BRIDGE_PROXY")==0) 		ptr_iDATA[p_num-1].modbus_mode=BRIDGE_PROXY;
		if(strcmp(arg, "BRIDGE_SIMPLE")==0) 	ptr_iDATA[p_num-1].modbus_mode=BRIDGE_SIMPLE;

		if(ptr_iDATA[p_num-1].modbus_mode==MODBUS_PORT_OFF) return CL_ERR_GATEWAY_MODE;
//		printf("%s\n", ptr_iDATA[p_num-1].serial.parity);

		k=check_gate_settings(&ptr_iDATA[p_num-1]);
//printf("NOOP2 p_num=%d, k=%d\n", p_num, k);
		if(k) return CL_ERR_IN_PORT_SETT;
//printf("NOOP3 key_cnt=%d\n", key_cnt);


		printf("parsing port %d, mode %d\n", p_num, ptr_iDATA[p_num-1].modbus_mode);

		switch(ptr_iDATA[p_num-1].modbus_mode) {
			case GATEWAY_SIMPLE:
				// tcp_port
				arg=argv[id_p_argc[i]+6];
				sscanf(arg, "%d", &temp);
				if((temp<502) || (temp>32000)) return CL_ERR_IN_MAP;
				ptr_iDATA[p_num-1].tcp_port=temp;
		//		printf("%d\n", ptr_iDATA[p_num-1].tcp_port);
				break;

 /*			case BRIDGE_SIMPLE:
				
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

					temp=get_ip_from_string(arg, &ptr_iDATA[p_num-1].clients[client].ip, &ptr_iDATA[p_num-1].clients[client].port);
					if(temp!=CL_OK) return CL_ERR_IN_MAP;
					
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
*/
			default:;
		}

		strcpy(ptr_iDATA[p_num-1].bridge_status, "INI");
	  }

	//------------------------------------
	/// читаем таблицу виртуральных устройств и таблицу опроса

	if(id_vslaves!=0) {

		sscanf(argv[id_vslaves+1], "%d", &vslaves_num);
		if((vslaves_num<1)||(vslaves_num>MAX_VIRTUAL_SLAVES)) return CL_ERR_VSLAVES_CFG;
		shift_counter=0;
	
		for(i=0; i<vslaves_num; i++) {
	
			arg=argv[id_vslaves+i*5+2+shift_counter];
			sscanf(arg, "%d", &vslave[i].start);
			vslave[i].start--;
	//		printf("%d\n", ptr_iDATA[p_num-1].serial.timeout);
	
			arg=argv[id_vslaves+i*5+3+shift_counter];
			sscanf(arg, "%d", &vslave[i].length);
	//		printf("%d\n", ptr_iDATA[p_num-1].serial.timeout);
	
			arg=argv[id_vslaves+i*5+4+shift_counter];
			vslave[i].port=arg[1]-48-1;
	
			arg=argv[id_vslaves+i*5+5+shift_counter];
			sscanf(arg, "%d", &vslave[i].device);
	//		printf("%d\n", ptr_iDATA[p_num-1].serial.timeout);

			arg=argv[id_vslaves+i*5+6+shift_counter];
			//strcpy(ptr_iDATA[p_num-1].serial.parity, arg);
			k=strlen(arg);
			for(j=0; j<k; j++) arg[j]=toupper(arg[j]);
			if(strcmp(arg, "COIL_STATUS")==0) vslave[i].modbus_table=COIL_STATUS_TABLE; else
			if(strcmp(arg, "INPUT_STATUS")==0) vslave[i].modbus_table=INPUT_STATUS_TABLE; else
			if(strcmp(arg, "HOLDING_REGISTER")==0) vslave[i].modbus_table=HOLDING_REGISTER_TABLE; else
			if(strcmp(arg, "INPUT_REGISTER")==0) vslave[i].modbus_table=INPUT_REGISTER_TABLE; else
				return CL_ERR_VSLAVES_CFG;
//		printf("%s\n", ptr_iDATA[p_num-1].serial.parity);

			arg=argv[id_vslaves+i*5+6+shift_counter+1];
			if(strcmp(arg, "--address_shift")==0) {
				vslave[i].address_shift=atoi(argv[id_vslaves+i*5+6+shift_counter+2]);
				shift_counter+=2;
				}

			if(argv[id_vslaves+i*5+6+shift_counter+1][0]=='-') shift_counter+=2;
		  }

	  int res=verify_vslaves();
	  if(res!=0) return CL_ERR_VSLAVES_CFG;
		}

	if(id_qt!=0) {
	
		sscanf(argv[id_qt+1], "%d", &qt_entries_num);
		if((qt_entries_num<1)||(qt_entries_num>MAX_QUERY_ENTRIES)) return CL_ERR_QT_CFG;
		shift_counter=0;
	
		for(i=0; i<qt_entries_num; i++) {
	
			arg=argv[id_qt+i*8+2+shift_counter];
			sscanf(arg, "%d", &query_table[i].start);
	//		printf("%d\n", ptr_iDATA[p_num-1].serial.timeout);
	
			arg=argv[id_qt+i*8+3+shift_counter];
			sscanf(arg, "%d", &query_table[i].length);
	//		printf("%d\n", ptr_iDATA[p_num-1].serial.timeout);
	
			arg=argv[id_qt+i*8+4+shift_counter];
			sscanf(arg, "%d", &query_table[i].offset);
	//		printf("%d\n", ptr_iDATA[p_num-1].serial.timeout);
	
			arg=argv[id_qt+i*8+5+shift_counter];
			query_table[i].port=arg[1]=='T'?PROXY_TCP:arg[1]-48-1;
	
			arg=argv[id_qt+i*8+6+shift_counter];
			sscanf(arg, "%d", &query_table[i].device);
	//		printf("%d\n", ptr_iDATA[p_num-1].serial.timeout);
	
		arg=argv[id_qt+i*8+7+shift_counter];
		//strcpy(ptr_iDATA[p_num-1].serial.parity, arg);
		k=strlen(arg);
		for(j=0; j<k; j++) arg[j]=toupper(arg[j]);
//sprintf("NOOP9 %s\n", arg);
		if(strcmp(arg, "COIL_STATUS")==0) query_table[i].mbf=MBF_READ_COILS; else
		if(strcmp(arg, "INPUT_STATUS")==0) query_table[i].mbf=MBF_READ_DECRETE_INPUTS; else
		if(strcmp(arg, "HOLDING_REGISTER")==0) query_table[i].mbf=MBF_READ_HOLDING_REGISTERS; else
		if(strcmp(arg, "INPUT_REGISTER")==0) query_table[i].mbf=MBF_READ_INPUT_REGISTERS; else return CL_ERR_QT_CFG;
//		printf("%s\n", ptr_iDATA[p_num-1].serial.parity);

			arg=argv[id_qt+i*8+8+shift_counter];
			sscanf(arg, "%d", &query_table[i].delay);
	
			arg=argv[id_qt+i*8+9+shift_counter];
			sscanf(arg, "%d", &query_table[i].critical);
	
//				printf("i=%d, %d(%d), %s\n", i, id_qt+i*9+10+shift_counter, argc, argv[id_qt+i*9+10+shift_counter]);
			if(argc>id_qt+i*8+10+shift_counter) {
				if(argv[id_qt+i*8+10+shift_counter][0]=='-') shift_counter+=2;
				}
		  }

	  int res=verify_proxy_queries();
	  if(res!=0) return CL_ERR_QT_CFG;
		}
	
/// ЧИТАЕМ ТАБЛИЦУ TCP_SERVERS

	if(id_tcpsrv!=0) {
	
		sscanf(argv[id_tcpsrv+1], "%d", &tcpsrv_entries_num);
		if((tcpsrv_entries_num<1)||(tcpsrv_entries_num>MAX_TCP_SERVERS)) return CL_ERR_TCPSRV_CFG;
		shift_counter=0;
	
		for(i=0; i<tcpsrv_entries_num; i++) {

					arg=argv[id_tcpsrv+i*4+2+shift_counter];

					temp=get_ip_from_string(arg, &tcp_servers[i].ip, &tcp_servers[i].port);
//					printf("noop temp=%d arg=\"%s\"\n", temp, arg);
					if(temp!=CL_OK) return CL_ERR_IN_MAP;
					
					printf("I:  %s\n", arg);
					printf("II: %X:%d\n", tcp_servers[i].ip, tcp_servers[i].port);

					/// читаем modbus-адрес устройства
					arg=argv[id_tcpsrv+i*4+3+shift_counter];
					temp=atoi(arg);
					if((temp<1)||(temp>247)) return CL_ERR_TCPSRV_CFG;
					tcp_servers[i].mb_slave=temp;
					
					/// читаем смещение адреса
					arg=argv[id_tcpsrv+i*4+4+shift_counter];
					tcp_servers[i].address_shift=atoi(arg);
					
					arg=argv[id_tcpsrv+i*4+5+shift_counter];
					tcp_servers[i].port=arg[1]=='T'?PROXY_TCP:arg[1]-48-1;
	
					/// Наименование устройства
					arg=argv[id_tcpsrv+i*4+6+shift_counter];
					//printf("%d-%d-%s\n", argc, id_p_argc[i]+6+2*client+2+shift_counter, arg);
//printf("desc=%s\n", arg);
					if(argc>id_tcpsrv+i*4+6+shift_counter)
					if(strcmp(arg, "--desc")==0) {
						arg=argv[id_tcpsrv+i*4+7+shift_counter];
						strcpy(tcp_servers[i].device_name, arg);
						shift_counter+=2;
						}
		  }

	  //int res=verify_tcp_servers();
	  //if(res!=0) return CL_TCPSRV_QT_CFG;
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

	if((data->serial.timeout<100000) || (data->serial.timeout>10000000)) return 4;

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

int checkDiapason(int function, int start_address, int length)
	{

	switch(function) {

		case MBF_READ_COILS:
			break;

		case MBF_READ_DECRETE_INPUTS:
			break;

		case MBF_READ_INPUT_REGISTERS:
			break;

		case MBF_READ_HOLDING_REGISTERS:
			if(start_address >= offset4xRegisters) {
				if(start_address + length <= offset4xRegisters + amount4xRegisters) return MOXA_DIAPASON_INSIDE;
				if(start_address >= offset4xRegisters + amount4xRegisters) return MOXA_DIAPASON_OUTSIDE;
				return MOXA_DIAPASON_OVERLAPPED;
				} else {
					if(start_address + length <= offset4xRegisters) return MOXA_DIAPASON_OUTSIDE;
					return MOXA_DIAPASON_OVERLAPPED;
					}
			break;

		default:;
		}

	return MOXA_DIAPASON_UNDEFINED;
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
			if(vslave[i].address_shift<0) return 6;

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
//  	if((query_table[i].length==0)||(query_table[i].length>70)) return 12; ///!!!
			if((query_table[i].length==0)||(query_table[i].length>125)) return 12;
			if((query_table[i].start+query_table[i].length)>65535) return 13;
			if(query_table[i].port>PROXY_TCP) return 14;
			if((query_table[i].device==0)||(query_table[i].device>247)) return 15;
			if((query_table[i].offset+query_table[i].length)>=65535) return 16;
			}
	return 0;
  }
///----------------------------------------------------------------------------------------------------------------
int get_ip_from_string(char *str, unsigned int *ip, unsigned int *port)
  {
	int dot, colon, digit, k, j;
	char addr[16];
	unsigned char ip1, ip2, ip3, ip4;

	//printf("arg:%s\n", arg);
	dot=colon=digit=0;
	k=strlen(str);
	for(j=0; j<k; j++) {
//		printf("%c", str[j]);
	  if((str[j]>=48)&&(str[j]<=57)) digit++; else
	    if(str[j]=='.') dot++; else
	      if(str[j]==':') {
				if(dot!=3) return CL_ERR_IN_MAP;
					colon++;
			} else return CL_ERR_IN_MAP;
		}
	if((dot!=3)||(colon>1))  return CL_ERR_IN_MAP;
	
	digit=0;
	for(j=digit; j<k; j++) if(str[j]=='.') {str[j]=0; break;}
	strcpy(addr, &str[digit]);
	ip4=(unsigned char) atoi(addr);
	digit=j+1;
	str[j]='.';
	
	for(j=digit; j<k; j++) if(str[j]=='.') {str[j]=0; break;}
	strcpy(addr, &str[digit]);
	ip3=(unsigned char) atoi(addr);
	digit=j+1;
	str[j]='.';
	
	for(j=digit; j<k; j++) if(str[j]=='.') {str[j]=0; break;}
	strcpy(addr, &str[digit]);
	ip2=(unsigned char) atoi(addr);
	digit=j+1;
	str[j]='.';
	
	for(j=digit; j<k; j++) if(str[j]==':') {str[j]=0; break;}
	strcpy(addr, &str[digit]);
	ip1=(unsigned char) atoi(addr);
	digit=j+1;
	str[j]=':';
	
	*ip=ip1+(ip2<<8)+(ip3<<16)+(ip4<<24);

  if(colon!=0) {
		strcpy(addr, &str[digit]);
		*port=atoi(addr);
		}

	return CL_OK;
  }
