/*

*/
#include <string.h>
#include <stdlib.h>
#include "global.h"

#define DEBUG_CL //чисто для отладки алгоритма интерепретации командной строки
//#define DEBUG_WORK //чисто для отладки работы ф-ции *run_work 

///-------------------------------------------------------------------------
void	help_print (void) //вывод справки
{
printf("%s\n", "\
Usage: moxa7gate [OPTIONS] [PORTx mode speed parity timeout GATEWAY tcp_port]...\n\n\
Opens serial port(s) for transfer Modbus messages to communicate with modbus-enabled devices.\n\n\
OPTIONS:\n\n\
--help               - display this help and exit\n\
--version            - output version information and exit\n\
--show_data_flow     - prints Modbus requests and responses\n\
--show_sys_messages  - prints messages for debug purposes\n\n\
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
void version_print (void) //вывод краткого описания программы
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
											u8	*show_data_flow,
											u8	*show_sys_messages
											) //обработка параметров командной строки
{
	//проверка на пустую командную строку
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
	
	//-----------------------------------
	//поиск и обработка ключей командной строки
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
	}
	//------------------------------------
	//поиск ключевых слов PORTx (т.е. начала секций)
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

//проверка на наличие и правильность ключевых слов командной строки
	if (p_cnt==0) return CL_ERR_PORT_WORD;

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
		if(strcmp(arg, "MASTER")==0) ptr_iDATA[p_num-1].modbus_mode=MODBUS_MASTER_MODE; else return CL_ERR_GATEWAY_MODE;
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
					  else if((id_p_argc[i]+6+2*client+1+shift_counter)>=argc) continue;
				
					//#include <arpa/inet.h> // ЯРП. 350, "ПЮГПЮАНРЙЮ ОПХКНФЕМХИ Б Linux.djvu"
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

					/// ВХРЮЕЛ modbus-ЮДПЕЯ СЯРПНИЯРБЮ
					arg=argv[id_p_argc[i]+6+2*client+1+shift_counter];
					temp=atoi(arg);
					if((temp<1)||(temp>247)) return CL_ERR_IN_MAP;
					ptr_iDATA[p_num-1].clients[client].mb_slave=temp;
					
					/// ЯДБХЦ ЮДПЕЯМНЦН ОПНЯРПЮМЯРБЮ
					arg=argv[id_p_argc[i]+6+2*client+2+shift_counter];
					if(strcmp(arg, "--address_shift")==0) {
						arg=argv[id_p_argc[i]+6+2*client+3+shift_counter];
						temp=atoi(arg);
						//if((temp<1)||(temp>247)) return CL_ERR_IN_MAP;
						ptr_iDATA[p_num-1].clients[client].address_shift=temp;
						shift_counter+=2;
						}
					
					/// НАЫЕЕ ЙНКХВЕЯРБН ЯЕРЕБШУ ЯНЕДХМЕМХИ
	  			ptr_iDATA[p_num-1].accepted_connections_number++;
					}
				
				if(i<p_cnt-1) {if((id_p_argc[i]+6+2*client+shift_counter)<id_p_argc[i+1]) return CL_ERR_IN_MAP;}
				  else if((id_p_argc[i]+6+2*client+shift_counter)<argc) return CL_ERR_IN_MAP;

				break;

			case MODBUS_MASTER_MODE: ///!!!
			default: ptr_iDATA[p_num-1].modbus_mode=MODBUS_PORT_OFF;
		}

		strcpy(ptr_iDATA[p_num-1].bridge_status, "INI");
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

	if((data->serial.timeout<500000) || (data->serial.timeout>10000000)) return 4;

	return 0;
	}
///-------------------------------------------------------------------------
