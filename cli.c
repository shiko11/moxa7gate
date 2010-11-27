/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///**************** МОДУЛЬ РАЗБОРА ПАРАМЕТРОВ СТАРТОВОЙ КОНФИГУРАЦИИ ***********

///=== CLI_H MODULE IMPLEMENTATION

#include "cli.h"

///=== CLI_H private variables

///=== CLI_H private functions

void help_print(void);    /* вывод справки */
void version_print(void); /* вывод информации о программе */

int parse_Security();
int parse_IfaceRTU();
int parse_IfaceTCP();
int parse_QT();
int parse_RT();
int parse_Exceptions();
int parse_AT();

///----------------------------------------------------------------------------
void help_print (void) /* вывод справки */
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
//обработка параметров командной строки
int get_command_line(int argc, char *argv[])
  {
	// выводим короткую справку по умолчанию
	if( (argc==1) ||
      ((argc==2) && (strncmp(argv[1], "/?", 2)     ==0)) ||
      ((argc==2) && (strncmp(argv[1], "/h", 2)     ==0)) ||
      ((argc==2) && (strncmp(argv[1], "/help", 5)  ==0)) ||
      ((argc==2) && (strncmp(argv[1], "-?", 2)     ==0)) ||
      ((argc==2) && (strncmp(argv[1], "-h", 2)     ==0)) ||
      ((argc==2) && (strncmp(argv[1], "-help", 5)  ==0)) ||
      ((argc==2) && (strncmp(argv[1], "--h", 3)    ==0)) ||
      ((argc==2) && (strncmp(argv[1], "--help", 6) ==0)) ||
      ((argc==2) && (strncmp(argv[1], "help", 4)   ==0)) ||
      ((argc==2) && (strncmp(argv[1], "?", 1)      ==0)) ||
      ) {help_print(); return CL_INFO;}

	unsigned int i;
	unsigned int j;
	unsigned int k;

  //**** ЧТЕНИЕ ОБЩИХ ПАРАМЕТРОВ НАСТРОЙКИ ШЛЮЗА ****
  int parse_Security();
  int check_Security();
  //**** ЧТЕНИЕ ПАРАМЕТРОВ КОНФИГУРАЦИИ ПОСЛЕДОВАТЕЛЬНЫХ ИНТЕРФЕЙСОВ ****
  int parse_IfaceRTU();
  int check_Iface(GW_Iface *data);
	strcpy(IfaceRTU[p_num].bridge_status, "INI");
  //**** ЧТЕНИЕ ПАРАМЕТРОВ КОНФИГУРАЦИИ ЛОГИЧЕСКИХ TCP-ИНТЕРФЕЙСОВ ****
  int parse_IfaceTCP();
  int check_Iface(GW_Iface *data);
	strcpy(IfaceTCP[t_num].bridge_status, "INI");
  //**** ЧТЕНИЕ ТАБЛИЦЫ ОПРОСА ****
  int parse_QT();
  int check_QTEntry();
  //**** ЧТЕНИЕ ТАБЛИЦЫ ВИРТУАЛЬНЫХ УСТРОЙСТВ ****
  int parse_RT();
  int check_RTEntry();
  //**** ЗАПИСЬ ТАБЛИЦЫ ИСКЛЮЧЕНИЙ ****
  int parse_Exceptions();
  int check_Exception();

  int parse_AT();
  int check_AT();

	return CL_OK;
	}
///-----------------------------------------------------------------------------
//**** ЧТЕНИЕ ОБЩИХ ПАРАМЕТРОВ НАСТРОЙКИ ШЛЮЗА ****
int parse_Security()
  {
	unsigned int i;
	unsigned int j;
	unsigned int k;

	unsigned char key_cnt	= 0;
	unsigned char id_key_argc[MAX_COMMON_KEYS];
	memset(id_key_argc, 0, sizeof(id_key_argc));
	unsigned char id_key_valset[MAX_COMMON_KEYS];
	memset(id_key_valset, 0, sizeof(id_key_valset));

	for(i=0; i<argc; i++)
		if(strncmp(argv[i], "--", 2)==0) {
			if(key_cnt==MAX_COMMON_KEYS) return CL_ERR_NOT_ALLOWED; // считаем есть дубликат
			id_key_argc[key_cnt++] = i;
		  }

	for (i=0; i<key_cnt; i++) {

    j=0;								

    // ОБЩАЯ ИНФОРМАЦИЯ

		if(strcmp(argv[id_key_argc[i]],"--Object")==0) {
      if(id_key_valset[0]==1) return CL_ERR_NOT_ALLOWED;
			strcpy(Security.Object, argv[id_key_argc[i]+1]);
      id_key_valset[0]=1;
      j=1;
      }

		if(strcmp(argv[id_key_argc[i]],"--Location")==0) {
      if(id_key_valset[1]==1) return CL_ERR_NOT_ALLOWED;
			strcpy(Security.Location, argv[id_key_argc[i]+1]);
      id_key_valset[1]=1;
      j=1;
      }

		if(strcmp(argv[id_key_argc[i]],"--Label")==0) {
      if(id_key_valset[2]==1) return CL_ERR_NOT_ALLOWED;
			strcpy(Security.Label, argv[id_key_argc[i]+1]);
      id_key_valset[2]=1;
      j=1;
      }

		if(strcmp(argv[id_key_argc[i]],"--NetworkName")==0) {
      if(id_key_valset[3]==1) return CL_ERR_NOT_ALLOWED;
			strcpy(Security.NetworkName, argv[id_key_argc[i]+1]);
      id_key_valset[3]=1;
      j=1;
      }

		if(strcmp(argv[id_key_argc[i]],"--LAN1Address")==0) {
      if(id_key_valset[4]==1) return CL_ERR_NOT_ALLOWED;
			get_ip_from_string(argv[id_key_argc[i]+1], &Security.LAN1Address, &k);
      id_key_valset[4]=1;
      j=1;
      }

		if(strcmp(argv[id_key_argc[i]],"--LAN2Address")==0) {
      if(id_key_valset[5]==1) return CL_ERR_NOT_ALLOWED;
			get_ip_from_string(argv[id_key_argc[i]+1], &Security.LAN2Address, &k);
      id_key_valset[5]=1;
      j=1;
      }

		if(strcmp(argv[id_key_argc[i]],"--VersionNumber")==0) {
      if(id_key_valset[6]==1) return CL_ERR_NOT_ALLOWED;
			strcpy(Security.VersionNumber, argv[id_key_argc[i]+1]);
      id_key_valset[6]=1;
      j=1;
      }

		if(strcmp(argv[id_key_argc[i]],"--VersionTime")==0) {
      if(id_key_valset[7]==1) return CL_ERR_NOT_ALLOWED;
			strcpy(Security.VersionTime, argv[id_key_argc[i]+1]);
      id_key_valset[7]=1;
      j=1;
      }

		if(strcmp(argv[id_key_argc[i]],"--Model")==0) {
      if(id_key_valset[8]==1) return CL_ERR_NOT_ALLOWED;
			strcpy(Security.Model, argv[id_key_argc[i]+1]);
      id_key_valset[8]=1;
      j=1;
      }

    // ПАРАМЕТРЫ РАБОТЫ ПРОГРАММЫ

		if(strcmp(argv[id_key_argc[i]],"--tcp_port")==0) {
      if(id_key_valset[9]==1) return CL_ERR_NOT_ALLOWED;
			Security.tcp_port=atoi(argv[id_key_argc[i]+1]);
      id_key_valset[9]=1;
      j=1;
			}

		if(strcmp(argv[id_key_argc[i]],"--modbus_address")==0) {
      if(id_key_valset[10]==1) return CL_ERR_NOT_ALLOWED;
			ptr_gate502->modbus_address=atoi(argv[id_key_argc[i]+1]);
      id_key_valset[10]=1;
      j=1;
			}

		if(strcmp(argv[id_key_argc[i]],"--status_info")==0) {
      if(id_key_valset[11]==1) return CL_ERR_NOT_ALLOWED;
			ptr_gate502->status_info=atoi(argv[id_key_argc[i]+1]);
      id_key_valset[11]=1;
      j=1;
			}

		if(strcmp(argv[id_key_argc[i]],"--show_sys_messages")==0) {j=1; Security.show_sys_messages=1;}
		if(strcmp(argv[id_key_argc[i]],"--map2Xto4X")==0)         {j=1; ptr_gate502->map2Xto4X=1;}

    // ОПЦИИ

		if(strcmp(argv[id_key_argc[i]],"--watchdog_timer")==0)    {j=1; Security.watchdog_timer=1;}
		if(strcmp(argv[id_key_argc[i]],"--show_data_flow")==0)    {j=1; Security.show_data_flow=1;}
		if(strcmp(argv[id_key_argc[i]],"--use_buzzer")==0)        {j=1; Security.use_buzzer=1;}

    if(j==0) return CL_ERR_NOT_ALLOWED; // считаем встретилась ошибка в написании ключа
	  }

 	return 0;
  }
///-----------------------------------------------------------------------------
//**** ЧТЕНИЕ ПАРАМЕТРОВ КОНФИГУРАЦИИ ПОСЛЕДОВАТЕЛЬНЫХ ИНТЕРФЕЙСОВ ****
int parse_IfaceRTU()
  {
	unsigned int i;
	unsigned int j;
	unsigned int k;

  char *arg;
  int shift_counter;

  int p_num;
	unsigned char p_cnt	= 0;
	unsigned char id_p_argc[MAX_MOXA_PORTS];
	memset(id_p_argc, 0, sizeof(id_p_argc));
	
	char		dest[16];
	char		tmp[3];

	for(i=0; i<MAX_MOXA_PORTS; i++) {
		memcpy(dest, NAME_MOXA_PORT, 5);
		sprintf(tmp, "%d", i+1);
		strcat(dest, tmp);
		for(j=0; j<argc; j++)
			if(strncmp(argv[j], dest, 5)==0) {
        if(p_cnt==MAX_MOXA_PORTS) return CL_ERR_MIN_PARAM; // считаем есть дубликаты
				id_p_argc[p_cnt++]=j;						
        }
    }

  for(i=0; i<p_cnt; i++) {

    if(argc - id_p_argc[i] < SERIAL_PARAMETERS) return CL_ERR_MIN_PARAM; // считаем ошибка в количестве параметров

		// port number
		arg=argv[id_p_argc[i]+0];
		p_num=arg[4]-48;
    // ошибка в номере интерфейса исключена, т.к. массив id_p_argc формируется заведомо корректными ключами:
		// if((p_num<1)||(p_num>MAX_MOXA_PORTS)) return CL_ERR_MIN_PARAM;
    p_num--;

		// port mode
		arg=argv[id_p_argc[i]+1];
		k=strlen(arg);
		for(j=0; j<k; j++) IfaceRTU[p_num].serial.p_mode[j]=toupper(arg[j]);
		IfaceRTU[p_num].serial.p_mode[j-1]=tolower(IfaceRTU[p_num].serial.p_mode[j-1]);
		IfaceRTU[p_num].serial.p_mode[j]=0;

		// port speed
		arg=argv[id_p_argc[i]+2];
		strcpy(IfaceRTU[p_num].serial.speed, arg);

		// parity		
		arg=argv[id_p_argc[i]+3];
		k=strlen(arg);
		for(j=0; j<k; j++) IfaceRTU[p_num].serial.parity[j]=tolower(arg[j]);
		IfaceRTU[p_num].serial.parity[j]=0;

		// timeout
		arg=argv[id_p_argc[i]+4];
		sscanf(arg, "%d", &IfaceRTU[p_num].serial.timeout);
		IfaceRTU[p_num].serial.timeout*=1000;

		// GATE MODE
		arg=argv[id_p_argc[i]+5];
		k=strlen(arg);
		for(j=0; j<k; j++) arg[j]=toupper(arg[j]);

		if(IfaceRTU[p_num].modbus_mode!=IFACE_OFF) return CL_ERR_GATEWAY_MODE; // считаем есть дубликаты

		if(strcmp(arg, "TCP_SERVER")==0) IfaceRTU[p_num].modbus_mode=IFACE_TCPSERVER;
		if(strcmp(arg, "RTU_MASTER")==0) IfaceRTU[p_num].modbus_mode=IFACE_RTUMASTER;
		if(strcmp(arg, "RTU_SLAVE")==0)  IfaceRTU[p_num].modbus_mode=IFACE_RTUSLAVE;

		if(IfaceRTU[p_num].modbus_mode==IFACE_OFF) return CL_ERR_GATEWAY_MODE; // считаем ошибка в названии режима работы

		shift_counter=0;

    // tcp port
		if(IfaceRTU[p_num].modbus_mode==IFACE_TCPSERVER)
		  if(argc > id_p_argc[i]+6+shift_counter) {
			  arg=argv[id_p_argc[i]+6+shift_counter];
			  sscanf(arg, "%d", &IfaceRTU[p_num].Security.tcp_port);
			  shift_counter++;
			  }

		/// Описание шлейфа (сети ModBus)
    if(argc > id_p_argc[i]+6+shift_counter+1) {
		  arg=argv[id_p_argc[i]+6+shift_counter];
		  if(strcmp(arg, "--desc")==0) {
			  shift_counter++;
			  arg=argv[id_p_argc[i]+6+shift_counter];
			  if(strlen(arg)>=(DEVICE_NAME_LENGTH-1))
				  arg[DEVICE_NAME_LENGTH-1]=0;
			  strcpy(IfaceRTU[p_num].description, arg);
			  shift_counter++;
			  }
      }
	  }

 	return 0;
  }
///-----------------------------------------------------------------------------
//**** ЧТЕНИЕ ПАРАМЕТРОВ КОНФИГУРАЦИИ ЛОГИЧЕСКИХ TCP-ИНТЕРФЕЙСОВ ****
int parse_IfaceTCP()
  {
	unsigned int i;
	unsigned int j;

  char *arg;
  int shift_counter;

  int t_num;
	unsigned char t_cnt	= 0;
	unsigned char id_t_argc[MAX_TCP_SERVERS];
	memset(id_t_argc, 0, sizeof(id_t_argc));
	
	char		dest[16];
	char		tmp[3];

	for(i=0; i<MAX_TCP_SERVERS; i++) {
		sprintf(tmp, "%d", i+1);
    if(i<9) strcpy(dest, "TCP0");
      else  strcpy(dest, "TCP");
		strcat(dest, tmp);
		for(j=0; j<argc; j++)
			if(strncmp(argv[j], dest, 5)==0) {
        if(t_cnt==MAX_TCP_SERVERS) return CL_ERR_MIN_PARAM; // считаем есть дубликаты
				id_t_argc[t_cnt++]=j;
        }
    }

  for(i=0; i<t_cnt; i++) {

    if(argc - id_t_argc[i] < LANTCP_PARAMETERS) return CL_ERR_MIN_PARAM; // считаем ошибка в количестве параметров

		// port number
		arg=argv[id_t_argc[i]+0];
		t_num=10*(arg[3]-48) + arg[4]-48;
    // ошибка в номере интерфейса исключена, т.к. массив id_t_argc формируется заведомо корректными ключами:
		// if((t_num<1)||(t_num > MAX_TCP_SERVERS)) return CL_ERR_MIN_PARAM;
    t_num--;

		// LAN1Address, TCP
		arg=argv[id_t_argc[i]+1];
		get_ip_from_string(arg, &IfaceTCP[t_num].ethernet.ip, &IfaceTCP[t_num].ethernet.port);

		// Unit ID
		arg=argv[id_t_argc[i]+2];
		sscanf(arg, "%d", &IfaceTCP[t_num].ethernet.unit_id);

		// Offset
		arg=argv[id_t_argc[i]+3];
		sscanf(arg, "%d", &IfaceTCP[t_num].ethernet.offset);

		// Modbus Address for ATM
		arg=argv[id_t_argc[i]+4];
		sscanf(arg, "%d", &IfaceTCP[t_num].ethernet.mb_slave);

		// LAN2Address, TCP
		arg=argv[id_t_argc[i]+5];
		get_ip_from_string(arg, &IfaceTCP[t_num].ethernet.ip2, &IfaceTCP[t_num].ethernet.port2);

		if(IfaceTCP[t_num].modbus_mode!=IFACE_OFF) return CL_ERR_GATEWAY_MODE; // считаем есть дубликаты
		IfaceTCP[t_num].modbus_mode=IFACE_TCPMASTER;

		shift_counter=0;

		/// Описание шлейфа (сети ModBus)
    if(argc > id_t_argc[i]+6+shift_counter+1) {
		  arg=argv[id_t_argc[i]+6+shift_counter];
		  if(strcmp(arg, "--desc")==0) {
			  shift_counter++;
			  arg=argv[id_t_argc[i]+6+shift_counter];
			  if(strlen(arg)>=(DEVICE_NAME_LENGTH-1))
				  arg[DEVICE_NAME_LENGTH-1]=0;
			  strcpy(IfaceTCP[t_num].description, arg);
			  shift_counter++;
			  }
      }
	  }

 	return 0;
  }
///-----------------------------------------------------------------------------
//**** ЧТЕНИЕ ТАБЛИЦЫ ОПРОСА ****
int parse_QT()
  {
	unsigned int i;
	unsigned int j;
	unsigned int k;

  char *arg;
  int shift_counter;

	int id_qt=0, qt_entries_num;

	for(j=0; j<argc; j++)
		if(strcmp(argv[j], "PROXY_TABLE")==0) {
			if(id_qt==0) id_qt=j;
				else return CL_ERR_QT_CFG;
			}

	if(id_qt!=0) {
	
		sscanf(argv[id_qt+1], "%d", &qt_entries_num);
    // это значение носит справочный характер и в алгоритме не участвует:
		// if((qt_entries_num<1)||(qt_entries_num>MAX_QUERY_ENTRIES)) return CL_ERR_QT_CFG;
		shift_counter=2;
	
    i=0;
		do {
							 
      if(argc - (id_qt+shift_counter) < PROXY_TABLE_PARAMETERS) return CL_ERR_MIN_PARAM;

      // интерфейс
			arg=argv[id_qt+shift_counter+0];
      if(((arg[0]=='P') || (arg[0]=='T')) && (arg[1]>47) && (arg[1]<58))
			  query_table[i].iface=\
          arg[0]=='P'?arg[1]-48-1:\
          10*(arg[1]-48-1)+(arg[2]-48-1);
        else break; // читаем записи таблицы до первой ошибки в названии интерфейса
	
      // адрес устройства
			arg=argv[id_qt+shift_counter+1];
			sscanf(arg, "%d", &query_table[i].device);
	
      // таблица MODBUS
      arg=argv[id_qt+shift_counter+2];
      k=strlen(arg);
      for(j=0; j<k; j++) arg[j]=toupper(arg[j]);
           if(strcmp(arg, "COIL_STATUS")==0)      query_table[i].mbf=MBF_READ_COILS; 
      else if(strcmp(arg, "INPUT_STATUS")==0)     query_table[i].mbf=MBF_READ_DECRETE_INPUTS;
      else if(strcmp(arg, "HOLDING_REGISTER")==0) query_table[i].mbf=MBF_READ_HOLDING_REGISTERS;
      else if(strcmp(arg, "INPUT_REGISTER")==0)   query_table[i].mbf=MBF_READ_INPUT_REGISTERS;
      // else return CL_ERR_QT_CFG;

      // режим доступа
      arg=argv[id_qt+shift_counter+3];
      k=strlen(arg);
      for(j=0; j<k; j++) arg[j]=toupper(arg[j]);
           if(strcmp(arg, "RW")==0)  query_table[i].access=QT_ACCESS_READWRITE;
      else if(strcmp(arg, "R")==0)   query_table[i].access=QT_ACCESS_READONLY;
      else if(strcmp(arg, "OFF")==0) query_table[i].access=QT_ACCESS_DISABLED;
      // else return CL_ERR_QT_CFG;

			arg=argv[id_qt+shift_counter+4];
			sscanf(arg, "%d", &query_table[i].start);
	
			arg=argv[id_qt+shift_counter+5];
			sscanf(arg, "%d", &query_table[i].length);
	
			/// стартовый регистр области записи (номер регистра)
			arg=argv[id_qt+shift_counter+6];
			sscanf(arg, "%d", &query_table[i].offset);
	
			arg=argv[id_qt+shift_counter+7];
			sscanf(arg, "%d", &query_table[i].delay);
	
			arg=argv[id_qt+shift_counter+8];
			sscanf(arg, "%d", &query_table[i].critical);
	
      /// Наименование устройства
      if(argc > id_qt+shift_counter+9+1 ) {
        arg=argv[id_qt+shift_counter+9];
        if(strcmp(arg, "--desc")==0) {
          shift_counter++;
          arg=argv[id_qt+shift_counter+9];
          if(strlen(arg)>=(DEVICE_NAME_LENGTH-1))
            arg[DEVICE_NAME_LENGTH-1]=0;
          strcpy(query_table[i].device_name, arg);
          shift_counter++;
          }
        }

      shift_counter+=PROXY_TABLE_PARAMETERS;
      i++;
		  } while (id_qt+shift_counter < argc);

		}
	
 	return 0;
  }
///-----------------------------------------------------------------------------
//**** ЧТЕНИЕ ТАБЛИЦЫ ВИРТУАЛЬНЫХ УСТРОЙСТВ ****
int parse_RT()
  {
	unsigned int i;
	unsigned int j;
	unsigned int k;

  char *arg;
  int shift_counter;

	int id_vslaves=0, vslaves_num;

	for(j=0; j<argc; j++)
		if(strcmp(argv[j], "RTM_TABLE")==0) {
			if(id_vslaves==0) id_vslaves=j;
				else return CL_ERR_VSLAVES_CFG;
			}

	if(id_vslaves!=0) {

		sscanf(argv[id_vslaves+1], "%d", &vslaves_num);
    // это значение носит справочный характер и в алгоритме не участвует:
    // if((vslaves_num<1)||(vslaves_num>MAX_VIRTUAL_SLAVES)) return CL_ERR_VSLAVES_CFG;
		shift_counter=2;
	
		i=0;
    do {
	
      if(argc - (id_vslaves+shift_counter) < RTM_TABLE_PARAMETERS) return CL_ERR_MIN_PARAM;

      // интерфейс
			arg=argv[id_vslaves+shift_counter+0];
      if(((arg[0]=='P') || (arg[0]=='T')) && (arg[1]>47) && (arg[1]<58))
			  vslave[i].iface=\
          arg[0]=='P'?arg[1]-48-1:\
          10*(arg[1]-48-1)+(arg[2]-48-1);
        else break; // читаем записи таблицы до первой ошибки в названии интерфейса
	
			arg=argv[id_vslaves+shift_counter+1];
			sscanf(arg, "%d", &vslave[i].device);

			arg=argv[id_vslaves+shift_counter+2];
			k=strlen(arg);
			for(j=0; j<k; j++) arg[j]=toupper(arg[j]);
			     if(strcmp(arg, "COIL_STATUS")==0)      vslave[i].modbus_table=COIL_STATUS_TABLE;
      else if(strcmp(arg, "INPUT_STATUS")==0)     vslave[i].modbus_table=INPUT_STATUS_TABLE;
      else if(strcmp(arg, "HOLDING_REGISTER")==0) vslave[i].modbus_table=HOLDING_REGISTER_TABLE;
      else if(strcmp(arg, "INPUT_REGISTER")==0)   vslave[i].modbus_table=INPUT_REGISTER_TABLE;
      // else return CL_ERR_VSLAVES_CFG;

			arg=argv[id_vslaves+shift_counter+3];
			sscanf(arg, "%d", &vslave[i].offset);
	
			arg=argv[id_vslaves+shift_counter+4];
			sscanf(arg, "%d", &vslave[i].start);
			vslave[i].start--;
	
			arg=argv[id_vslaves+shift_counter+5];
			sscanf(arg, "%d", &vslave[i].length);
	
			/// Наименование устройства
			if(argc>id_vslaves+shift_counter+6+1) {
			  arg=argv[id_vslaves+shift_counter+6];
			  if(strcmp(arg, "--desc")==0) {
				  shift_counter++;
				  arg=argv[id_vslaves+shift_counter+6];
				  if(strlen(arg)>=(DEVICE_NAME_LENGTH-1))
					  arg[DEVICE_NAME_LENGTH-1]=0;
				  strcpy(vslave[i].device_name, arg);
				  shift_counter++;
				  }
        }

      shift_counter+=RTM_TABLE_PARAMETERS;
      i++;
		  } while(id_vslaves+shift_counter < argc);

		}

 	return 0;
  }
///-----------------------------------------------------------------------------
//**** ЗАПИСЬ ТАБЛИЦЫ ИСКЛЮЧЕНИЙ ****
int parse_Exceptions()
  {
	unsigned int i;
	unsigned int j;
	unsigned int k;

  char *arg;
  int shift_counter;

	int id_expt=0, expt_num;

	for(j=0; j<argc; j++)
		if(strcmp(argv[j], "EXCEPTIONS")==0) {
			if(id_expt==0) id_expt=j;
				else return CL_ERR_EXPT_CFG;
			}

	if(id_expt!=0) {

		sscanf(argv[id_expt+1], "%d", &expt_num);
    // это значение носит справочный характер и в алгоритме не участвует:
    // if((expt_num<1)||(expt_num>MOXAGATE_EXCEPTIONS_NUMBER)) return CL_ERR_EXPT_CFG;
		shift_counter=2;
	
		i=0;
    do {
	
      if(argc - (id_expt+shift_counter) < EXCEPTION_PARAMETERS) return CL_ERR_MIN_PARAM;

			arg=argv[id_expt+shift_counter+0];
			k=strlen(arg);
			for(j=0; j<k; j++) arg[j]=toupper(arg[j]);
			     if(strcmp(arg, "RESPONSE_RECV_RAW")==0)  Exception[i].stage=EXPT_STAGE_RESPONSE_RECV_RAW;
      else if(strcmp(arg, "QUERY_RECV_RAW")==0)     Exception[i].stage=EXPT_STAGE_QUERY_RECV_RAW;
      else if(strcmp(arg, "QUERY_RECV")==0)         Exception[i].stage=EXPT_STAGE_QUERY_RECV;
      else if(strcmp(arg, "QUERY_FRWD")==0)         Exception[i].stage=EXPT_STAGE_QUERY_FRWD;
      else if(strcmp(arg, "RESPONSE_RECV")==0)      Exception[i].stage=EXPT_STAGE_RESPONSE_RECV;
      else if(strcmp(arg, "RESPONSE_SEND")==0)      Exception[i].stage=EXPT_STAGE_RESPONSE_SEND;
      // else return CL_ERR_VSLAVES_CFG;


			arg=argv[id_expt+shift_counter+1];
			k=strlen(arg);
			for(j=0; j<k; j++) arg[j]=toupper(arg[j]);
			     if(strcmp(arg, "SKS07_DIOGEN")==0)      Exception[i].action=EXCP_SKS07_DIOGEN;
      // else if(strcmp(arg, "INPUT_STATUS")==0)     vslave[i].modbus_table=INPUT_STATUS_TABLE;
      // else return CL_ERR_VSLAVES_CFG;

			arg=argv[id_expt+shift_counter+2];
			sscanf(arg, "%d", &Exception[i].prm1);
	
			arg=argv[id_expt+shift_counter+3];
			sscanf(arg, "%d", &Exception[i].prm2);
	
			arg=argv[id_expt+shift_counter+4];
			sscanf(arg, "%d", &Exception[i].prm3);
	
			arg=argv[id_expt+shift_counter+5];
			sscanf(arg, "%d", &Exception[i].prm4);
	
			/// Комментарий
			if(argc > id_expt+shift_counter+6+1) {
			  arg=argv[id_expt+shift_counter+6];
			  if(strcmp(arg, "--desc")==0) {
				  shift_counter++;
				  arg=argv[id_expt+shift_counter+6];
				  if(strlen(arg)>=(DEVICE_NAME_LENGTH-1))
					  arg[DEVICE_NAME_LENGTH-1]=0;
				  strcpy(Exception[i].comment, arg);
				  shift_counter++;
				  }
        }

      shift_counter+=EXCEPTION_PARAMETERS;
      i++;
		  } while(id_expt+shift_counter < argc);

		}

 	return 0;
  }
///-----------------------------------------------------------------------------

int parse_AT()
  {
	unsigned int i;
	unsigned int j;
	unsigned int k;
  
  char *arg;
  int shift_counter;

 	return 0;
  }
///-----------------------------------------------------------------------------

int check_gate_settings(GW_Iface *data)
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
/*//----------------------------------------------------------------------------------------------------------------
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
*///----------------------------------------------------------------------------------------------------------------
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
			if(vslave[i].length==0) return 2;
			if((vslave[i].start+vslave[i].length)>65535) return 3;
			if((vslave[i].iface>GATEWAY_T32) ||
        ((vslave[i].iface>GATEWAY_P8) && (vslave[i].iface<GATEWAY_T01))
         ) return 4;
			if((vslave[i].device==0)||(vslave[i].device>247)) return 5;
			// if(vslave[i].offset<0) return 6; // comparison is always false due to limited range of data type

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

		switch(query_table[i].mbf) {
			case MBF_READ_COILS: 
				if(	(query_table[i].start-1 < MIN_STARTING_ADDRESS_1X)||
						(query_table[i].start-1 > MAX_STARTING_ADDRESS_1X)) return 11;
				if(	(query_table[i].length < MBF_0x01_MIN_QUANTITY)||
						(query_table[i].length > MBF_0x01_MAX_QUANTITY)) return 12;
				if((query_table[i].start-1 + query_table[i].length) > MAX_STARTING_ADDRESS_1X) return 13;
				if((query_table[i].offset-1+ query_table[i].length) > MAX_STARTING_ADDRESS_1X) return 16;
				break;
	
			case MBF_READ_DECRETE_INPUTS: 
				if(	(query_table[i].start-1 < MIN_STARTING_ADDRESS_2X)||
						(query_table[i].start-1 > MAX_STARTING_ADDRESS_2X)) return 11;
				if(	(query_table[i].length < MBF_0x02_MIN_QUANTITY)||
						(query_table[i].length > MBF_0x02_MAX_QUANTITY)) return 12;
				if((query_table[i].start-1 + query_table[i].length) > MAX_STARTING_ADDRESS_2X) return 13;
				if((query_table[i].offset-1+ query_table[i].length) > MAX_STARTING_ADDRESS_2X) return 16;
				break;
	
			case MBF_READ_HOLDING_REGISTERS: 
				if(	(query_table[i].start-1 < MIN_STARTING_ADDRESS_4X)||
						(query_table[i].start-1 > MAX_STARTING_ADDRESS_4X)) return 11;
				if(	(query_table[i].length < MBF_0x03_MIN_QUANTITY)||
						(query_table[i].length > MBF_0x03_MAX_QUANTITY)) return 12;
				if((query_table[i].start-1 + query_table[i].length) > MAX_STARTING_ADDRESS_4X) return 13;
				if((query_table[i].offset-1+ query_table[i].length) > MAX_STARTING_ADDRESS_4X) return 16;
				break;
	
			case MBF_READ_INPUT_REGISTERS: 
				if(	(query_table[i].start-1 < MIN_STARTING_ADDRESS_3X)||
						(query_table[i].start-1 > MAX_STARTING_ADDRESS_3X)) return 11;
				if(	(query_table[i].length < MBF_0x04_MIN_QUANTITY)||
						(query_table[i].length > MBF_0x04_MAX_QUANTITY)) return 12;
				if((query_table[i].start-1 + query_table[i].length) > MAX_STARTING_ADDRESS_3X) return 13;
				if((query_table[i].offset-1+ query_table[i].length) > MAX_STARTING_ADDRESS_3X) return 16;
				break;
	
			default: return 17;
			}
	
//		if(query_table[i].port>PROXY_TCP) return 14;
		if((query_table[i].device<MODBUS_ADDRESS_MIN)||(query_table[i].device>MODBUS_ADDRESS_MAX)) return 15;
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
	if(j<k) str[j]=':';
	
	*ip=ip1+(ip2<<8)+(ip3<<16)+(ip4<<24);
//	printf("str=%s, digit=%d, addr=%s\n", str, digit, &str[digit]);
  if(colon!=0) {
		strcpy(addr, &str[digit]);
		*port=atoi(addr);
		}

	return CL_OK;
  }

///-----------------------------------------------------------------------------------------------------------------
