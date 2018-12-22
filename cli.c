/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///**************** ������ ������� ���������� ��������� ������������ ***********

///=== CLI_H MODULE IMPLEMENTATION

#include "cli.h"

///=== CLI_H private variables

int qt_current=0;
int rt_current=0;
int expt_current=0;

int argc_counter=0;

///=== CLI_H private functions

void help_print(void);    /* ����� ���������� � ��������� */

int parse_Security();
int parse_IfacesRTU();
int parse_IfacesTCP();
int parse_AddressMap();
int parse_Vslaves();
int parse_ProxyQueries();
int parse_Exceptions();

///----------------------------------------------------------------------------
void help_print (void) /* ����� ���������� � ��������� */
{
printf("%s\n", "\
Usage: moxa7gate [OPTIONS] [PORTx mode speed parity timeout GATEWAY tcp_port]...\n\n\
Opens serial port(s) for transfer Modbus messages to communicate with modbus-enabled devices.\n\n\
Read the manual for details.\n\
");

//printf("%s\n", "\
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
//��������� ���������� ��������� ������
int get_command_line(int argc, char *argv[])
  {
	// ������� ���������� � ��������� �� ���������
	if( (argc==1) ||
      ((argc>1) && (strncmp(argv[1], "/?", 2)     ==0)) ||
      ((argc>1) && (strncmp(argv[1], "/h", 2)     ==0)) ||
      ((argc>1) && (strncmp(argv[1], "/help", 5)  ==0)) ||
      ((argc>1) && (strncmp(argv[1], "-?", 2)     ==0)) ||
      ((argc>1) && (strncmp(argv[1], "-h", 2)     ==0)) ||
      ((argc>1) && (strncmp(argv[1], "-help", 5)  ==0)) ||
      ((argc>1) && (strncmp(argv[1], "--h", 3)    ==0)) ||
      ((argc>1) && (strncmp(argv[1], "--help", 6) ==0)) ||
      ((argc>1) && (strncmp(argv[1], "help", 4)   ==0)) ||
      ((argc>1) && (strncmp(argv[1], "?", 1)      ==0)) ||
      ) {help_print(); return COMMAND_LINE_INFO;}

	int i=COMMAND_LINE_OK;
	unsigned int j;
	unsigned int k;

  //**** ������ ����� ���������� ��������� ����� ****
  k=parse_Security();
  if(k!=COMMAND_LINE_OK) {
    i=COMMAND_LINE_ERROR;
		sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|EVENT_SRC_SYSTEM, k, 0, 0, 0, 0);
    } else {
    k=check_Security();
    if(k!=COMMAND_LINE_OK) {
      i=COMMAND_LINE_ERROR;
      sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|EVENT_SRC_SYSTEM, k, 0, 0, 0, 0);
      }
    }

  //**** ������ ���������� ������������ ���������������� ����������� ****
  k=parse_IfacesRTU();
  if(k!=COMMAND_LINE_OK) {
    i=COMMAND_LINE_ERROR;
    sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|EVENT_SRC_SYSTEM, k, 0, 0, 0, 0);
    } else {
      for(j=GATEWAY_P1; j<GATEWAY_P8; j++) {
        if(IfaceRTU[j].modbus_mode==IFACE_OFF) continue;
        k=check_Iface(&IfaceRTU[j]);
        if(k!=COMMAND_LINE_OK) {
          i=COMMAND_LINE_ERROR;
          sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|EVENT_SRC_SYSTEM, k, 0, 0, 0, 0);
          break; // ��������������� ����� ������ ������
          } else strcpy(IfaceRTU[j].bridge_status, "INI");
        }
      }

  //**** ������ ���������� ������������ ���������� TCP-����������� ****
  k=parse_IfacesTCP();
  if(k!=COMMAND_LINE_OK) {
    i=COMMAND_LINE_ERROR;
    sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|EVENT_SRC_SYSTEM, k, 0, 0, 0, 0);
    } else {
      for(j=GATEWAY_T01; j<GATEWAY_T32; j++) {
        if(IfaceTCP[j].modbus_mode==IFACE_OFF) continue;
        k=check_Iface(&IfaceTCP[j]);
        if(k!=COMMAND_LINE_OK) {
          i=COMMAND_LINE_ERROR;	
          sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|EVENT_SRC_SYSTEM, k, 0, 0, 0, 0);
          break; // ��������������� ����� ������ ������
          } else strcpy(IfaceTCP[j].bridge_status, "INI");
        }
      }

  //**** ������ ������� ���������� ������� ***
  k=parse_AddressMap();
  if(k!=COMMAND_LINE_OK) {
    i=COMMAND_LINE_ERROR;
    sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|EVENT_SRC_SYSTEM, k, 0, 0, 0, 0);
    } else {
      for(j=MODBUS_ADDRESS_MIN; j<=MODBUS_ADDRESS_MAX; j++) {
        if(AddressMap[j].iface==GATEWAY_NONE) continue;
        k=check_AddressMap_Entry(j);
        if(k!=COMMAND_LINE_OK) {
          i=COMMAND_LINE_ERROR;	
          sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|EVENT_SRC_SYSTEM, k, 0, 0, 0, 0);
          break; // ��������������� ����� ������ ������
          }
        }
      }

  //**** ������ ������� ����������� ��������� ****
  k=parse_Vslaves();
  if(k!=COMMAND_LINE_OK) {
    i=COMMAND_LINE_ERROR;
    sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|EVENT_SRC_SYSTEM, k, 0, 0, 0, 0);
    } else {
      for(j=0; j<MAX_VIRTUAL_SLAVES; j++) {
        if(vslave[j].iface==GATEWAY_NONE) continue;
        k=check_Vslave_Entry(j);
        if(k!=COMMAND_LINE_OK) {
          i=COMMAND_LINE_ERROR;	
          sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|EVENT_SRC_SYSTEM, k, 0, 0, 0, 0);
          break; // ��������������� ����� ������ ������
          }
        }
      }

  //**** ������ ������� ������ ****
  k=parse_ProxyQueries();
  if(k!=COMMAND_LINE_OK) {
    i=COMMAND_LINE_ERROR;
    sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|EVENT_SRC_SYSTEM, k, 0, 0, 0, 0);
    } else {
      for(j=0; j<MAX_QUERY_ENTRIES; j++) {
        if(query_table[j].iface==GATEWAY_NONE) continue;
        k=check_ProxyQuery_Entry(j);
        if(k!=COMMAND_LINE_OK) {
          i=COMMAND_LINE_ERROR;	
          sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|EVENT_SRC_SYSTEM, k, 0, 0, 0, 0);
          break; // ��������������� ����� ������ ������
          }
        }
      }

  //**** ������ ������� ���������� ****
  k=parse_Exceptions();
  if(k!=COMMAND_LINE_OK) {
    i=COMMAND_LINE_ERROR;
    sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|EVENT_SRC_SYSTEM, k, 0, 0, 0, 0);
    } else {
      for(j=0; j<MOXAGATE_EXCEPTIONS_NUMBER; j++) {
        if(Exceptions[j].stage==EXPT_STAGE_UNDEFINED) continue;
        k=check_Exception(j);
        if(k!=COMMAND_LINE_OK) {
          i=COMMAND_LINE_ERROR;	
          sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|EVENT_SRC_SYSTEM, k, 0, 0, 0, 0);
          break; // ��������������� ����� ������ ������
          }
        }
      }

  // ��������� ���������� ������������ ���������������� ����������
  if((argc_counter+1)!=argc) return COMMAND_LINE_ARGC;

	return i;
	}
///-----------------------------------------------------------------------------
//**** ������ ����� ���������� ��������� ����� ****
int parse_Security()
  {
	unsigned int i;
	unsigned int j;
	unsigned int k;

  char *arg;

	unsigned char key_cnt	= 0;
	unsigned char id_key_argc[MAX_COMMON_KEYS];
	memset(id_key_argc, 0, sizeof(id_key_argc));
	unsigned char id_key_valset[MAX_COMMON_KEYS];
	memset(id_key_valset, 0, sizeof(id_key_valset));

	for(i=0; i<argc; i++)
		if(strncmp(argv[i], "--", 2)==0) {
			if(key_cnt==MAX_COMMON_KEYS) return SECURITY_CONF_STRUCT; // ������� ������ � ��������� ����������
			id_key_argc[key_cnt++] = i;
		  }

	for (i=0; i<key_cnt; i++) {

    j=0;								

    // ����� ����������

		if(strcmp(argv[id_key_argc[i]],"--Object")==0) {
      if(id_key_valset[0]==1) return SECURITY_CONF_DUPLICATE;
      arg=argv[id_key_argc[i]+1];
      if(strlen(arg)>=(DEVICE_NAME_LENGTH-1))
        arg[DEVICE_NAME_LENGTH-1]=0;
			strcpy(Security.Object, arg);
      id_key_valset[0]=1;
      j=2;
      }

		if(strcmp(argv[id_key_argc[i]],"--Location")==0) {
      if(id_key_valset[1]==1) return SECURITY_CONF_DUPLICATE;
      arg=argv[id_key_argc[i]+1];
      if(strlen(arg)>=(DEVICE_NAME_LENGTH-1))
        arg[DEVICE_NAME_LENGTH-1]=0;
			strcpy(Security.Location, arg);
      id_key_valset[1]=1;
      j=2;
      }

		if(strcmp(argv[id_key_argc[i]],"--Label")==0) {
      if(id_key_valset[2]==1) return SECURITY_CONF_DUPLICATE;
      arg=argv[id_key_argc[i]+1];
      if(strlen(arg)>=(DEVICE_NAME_LENGTH-1))
        arg[DEVICE_NAME_LENGTH-1]=0;
			strcpy(Security.Label, arg);
      id_key_valset[2]=1;
      j=2;
      }

		if(strcmp(argv[id_key_argc[i]],"--NetworkName")==0) {
      if(id_key_valset[3]==1) return SECURITY_CONF_DUPLICATE;
      arg=argv[id_key_argc[i]+1];
      if(strlen(arg)>=(DEVICE_NAME_LENGTH-1))
        arg[DEVICE_NAME_LENGTH-1]=0;
			strcpy(Security.NetworkName, arg);
      id_key_valset[3]=1;
      j=2;
      }

		if(strcmp(argv[id_key_argc[i]],"--LAN1Address")==0) {
      if(id_key_valset[4]==1) return SECURITY_CONF_DUPLICATE;
			get_ip_from_string(argv[id_key_argc[i]+1], &Security.LAN1Address, &k);
      id_key_valset[4]=1;
      j=2;
      }

		if(strcmp(argv[id_key_argc[i]],"--LAN2Address")==0) {
      if(id_key_valset[5]==1) return SECURITY_CONF_DUPLICATE;
			get_ip_from_string(argv[id_key_argc[i]+1], &Security.LAN2Address, &k);
      id_key_valset[5]=1;
      j=2;
      }

		if(strcmp(argv[id_key_argc[i]],"--VersionNumber")==0) {
      if(id_key_valset[6]==1) return SECURITY_CONF_DUPLICATE;
      arg=argv[id_key_argc[i]+1];
      if(strlen(arg)>=(DEVICE_NAME_LENGTH-1))
        arg[DEVICE_NAME_LENGTH-1]=0;
			strcpy(Security.VersionNumber, arg);
      id_key_valset[6]=1;
      j=2;
      }

		if(strcmp(argv[id_key_argc[i]],"--VersionTime")==0) {
      if(id_key_valset[7]==1) return SECURITY_CONF_DUPLICATE;
      arg=argv[id_key_argc[i]+1];
      if(strlen(arg)>=(DEVICE_NAME_LENGTH-1))
        arg[DEVICE_NAME_LENGTH-1]=0;
			strcpy(Security.VersionTime, arg);
      id_key_valset[7]=1;
      j=2;
      }

		if(strcmp(argv[id_key_argc[i]],"--Model")==0) {
      if(id_key_valset[8]==1) return SECURITY_CONF_DUPLICATE;
      arg=argv[id_key_argc[i]+1];
      if(strlen(arg)>=(DEVICE_NAME_LENGTH-1))
        arg[DEVICE_NAME_LENGTH-1]=0;
			strcpy(Security.Model, arg);
      id_key_valset[8]=1;
      j=2;
      }

    // ��������� ������ ���������

		if(strcmp(argv[id_key_argc[i]],"--tcp_port")==0) {
      if(id_key_valset[9]==1) return SECURITY_CONF_DUPLICATE;
			Security.tcp_port=atoi(argv[id_key_argc[i]+1]);
      id_key_valset[9]=1;
      j=2;
			}

		if(strcmp(argv[id_key_argc[i]],"--modbus_address")==0) {
      if(id_key_valset[10]==1) return SECURITY_CONF_DUPLICATE;
			MoxaDevice.modbus_address=atoi(argv[id_key_argc[i]+1]);
      id_key_valset[10]=1;
      j=2;
			}

		if(strcmp(argv[id_key_argc[i]],"--status_info")==0) {
      if(id_key_valset[11]==1) return SECURITY_CONF_DUPLICATE;
			MoxaDevice.status_info=atoi(argv[id_key_argc[i]+1]);
      ///!!! ������ ������������������ �����, �.�. ��� ������. � ��������� ����� ���������������� ��� �������������:
      //MoxaDevice.status_info--;
      id_key_valset[11]=1;
      j=2;
			}

		if(strcmp(argv[id_key_argc[i]],"--show_sys_messages")==0) {j=1; Security.show_sys_messages=1;}
		if(strcmp(argv[id_key_argc[i]],"--map2Xto4X")==0)         {j=1; MoxaDevice.map2Xto4X=1;}

    // �����

		if(strcmp(argv[id_key_argc[i]],"--watchdog_timer")==0)    {j=1; Security.watchdog_timer=1;}
		if(strcmp(argv[id_key_argc[i]],"--show_data_flow")==0)    {j=1; Security.show_data_flow=1;}
		if(strcmp(argv[id_key_argc[i]],"--use_buzzer")==0)        {j=1; Security.use_buzzer=1;}

    if(j==0) return SECURITY_CONF_SPELLING; // ������� ����������� ������ � ��������� �����
    argc_counter+=j;
	  }

 	return COMMAND_LINE_OK;
  }
///-----------------------------------------------------------------------------
//**** ������ ���������� ������������ ���������������� ����������� ****
int parse_IfacesRTU()
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
        if(p_cnt==MAX_MOXA_PORTS) return IFACE_CONF_RTUDUPLICATE; // ������� ���� ���������
				id_p_argc[p_cnt++]=j;						
        }
    }

  for(i=0; i<p_cnt; i++) {

    if(argc - id_p_argc[i] < SERIAL_PARAMETERS) return IFACE_CONF_RTUSTRUCT; // ������� ������ � ��������� ����������

		// port number
		arg=argv[id_p_argc[i]+0];
		p_num=arg[4]-48;
    // ������ � ������ ���������� ���������, �.�. ������ id_p_argc ����������� �������� ����������� �������:
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

		if(IfaceRTU[p_num].modbus_mode!=IFACE_OFF) return IFACE_CONF_RTUDUPLICATE; // ������� ���� ���������

		if(strcmp(arg, "TCP_SERVER")==0) IfaceRTU[p_num].modbus_mode=IFACE_TCPSERVER;
		if(strcmp(arg, "RTU_MASTER")==0) IfaceRTU[p_num].modbus_mode=IFACE_RTUMASTER;
		if(strcmp(arg, "RTU_SLAVE")==0)  IfaceRTU[p_num].modbus_mode=IFACE_RTUSLAVE;

    // ������� ������ � �������� ������ ������
		if(IfaceRTU[p_num].modbus_mode==IFACE_OFF) return IFACE_CONF_MBMODE;

		shift_counter=0;

    // tcp port
		if(IfaceRTU[p_num].modbus_mode==IFACE_TCPSERVER)
		  if(argc > id_p_argc[i]+6+shift_counter) {
			  arg=argv[id_p_argc[i]+6+shift_counter];
			  sscanf(arg, "%d", &IfaceRTU[p_num].Security.tcp_port);
			  shift_counter++;
			  }

		/// �������� ������ (���� ModBus)
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

    argc_counter+=SERIAL_PARAMETERS+shift_counter;
	  }

 	return COMMAND_LINE_OK;
  }
///-----------------------------------------------------------------------------
//**** ������ ���������� ������������ ���������� TCP-����������� ****
int parse_IfacesTCP()
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
        if(t_cnt==MAX_TCP_SERVERS) return IFACE_CONF_TCPDUPLICATE; // ������� ���� ���������
				id_t_argc[t_cnt++]=j;
        }
    }

  for(i=0; i<t_cnt; i++) {

    if(argc - id_t_argc[i] < LANTCP_PARAMETERS) return IFACE_CONF_TCPSTRUCT; // ������� ������ � ��������� ����������

		// port number
		arg=argv[id_t_argc[i]+0];
		t_num=10*(arg[3]-48) + arg[4]-48;
    // ������ � ������ ���������� ���������, �.�. ������ id_t_argc ����������� �������� ����������� �������:
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

		if(IfaceTCP[t_num].modbus_mode!=IFACE_OFF) return IFACE_CONF_TCPDUPLICATE; // ������� ���� ���������
		IfaceTCP[t_num].modbus_mode=IFACE_TCPMASTER;

		shift_counter=0;

		/// �������� ������ (���� ModBus)
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

    argc_counter+=LANTCP_PARAMETERS+shift_counter;
	  }

 	return COMMAND_LINE_OK;
  }

///-----------------------------------------------------------------------------
//**** ������ ������� ���������� ������� ***
int parse_AddressMap()
  {
	unsigned int i;
	unsigned int j;
	unsigned int k;

  char *arg;
	unsigned int val;

	int id_at;

  for(i=0; i<argc; i++)
  if(strncmp(argv[i], "AT", 2)==0) {
    id_at=i;
	
    arg=argv[id_at+0];
    if(strlen(arg)!=3) return ATM_CONF_SPELLING;
    k=arg[2]-48;
    if((k<1) || (k>8)) return ATM_CONF_SPELLING;
    if(argc - id_at < ADDRESSMAP_PARAMETERS+1) return ATM_CONF_STRUCT;
    k--;

    for(j=1; j<=ADDRESSMAP_PARAMETERS; j++) {
      arg=argv[id_at+j];
      val=atoi(arg);															 
      if(val==0) continue; // �������� ��� ������ ��������� �������
      AddressMap[ADDRESSMAP_PARAMETERS * k].iface=val >> 8;
      // �� ������ ������� ��������� ������ ��������� ���������� ��� ����������:
      if(AddressMap[ADDRESSMAP_PARAMETERS * k].iface > GATEWAY_P8) return ATM_CONF_IFACE;
      AddressMap[ADDRESSMAP_PARAMETERS * k].address=val & 0xff;
      }

    argc_counter+=ADDRESSMAP_PARAMETERS+1;
		}

 	return COMMAND_LINE_OK;
  }

///-----------------------------------------------------------------------------
//**** ������ ������� ����������� ��������� ****
int parse_Vslaves()
  {
	unsigned int i;
	unsigned int j;
	unsigned int k;

  char *arg;
  int shift_counter;

	int id_rt;

  for(i=0; i<argc; i++)
  if(strcmp(argv[i], "RT")==0) {
    id_rt=i;
	
    if(argc - id_rt < RTM_TABLE_PARAMETERS) return VSLAVE_CONF_STRUCT; // ������� ������ � ��������� ����������

    // ���������
    arg=argv[id_rt+1];
    if(  ((arg[0]=='P') && (arg[1]>48) && (arg[1]<57)) ||
         ((arg[0]=='T') && (arg[1]>47) && (arg[1]<52) && (arg[2]>47) && (arg[2]<58))
      )
      vslave[rt_current].iface=\
      arg[0]=='P'?arg[1]-48-1:\
      10*(arg[1]-48)+(arg[2]-48)-1+GATEWAY_T01;
      else return VSLAVE_CONF_IFACE; // ������� ������ � �������� ����������
	
    arg=argv[id_rt+2];
    sscanf(arg, "%d", &vslave[rt_current].device);

    arg=argv[id_rt+3];
    k=strlen(arg);
    for(j=0; j<k; j++) arg[j]=toupper(arg[j]);
         if(strcmp(arg, "COIL_STATUS")==0)      vslave[rt_current].modbus_table=COIL_STATUS_TABLE;
    else if(strcmp(arg, "INPUT_STATUS")==0)     vslave[rt_current].modbus_table=INPUT_STATUS_TABLE;
    else if(strcmp(arg, "HOLDING_REGISTER")==0) vslave[rt_current].modbus_table=HOLDING_REGISTER_TABLE;
    else if(strcmp(arg, "INPUT_REGISTER")==0)   vslave[rt_current].modbus_table=INPUT_REGISTER_TABLE;
    // else return CL_ERR_VSLAVES_CFG;

    arg=argv[id_rt+4];
    sscanf(arg, "%d", &vslave[rt_current].offset);
	
    arg=argv[id_rt+5];
    sscanf(arg, "%d", &vslave[rt_current].start);
    vslave[rt_current].start--;
	
    arg=argv[id_rt+6];
    sscanf(arg, "%d", &vslave[rt_current].length);

    shift_counter=0;
	
    /// ������������ ����������
    if(argc>id_rt+7+1) {
      arg=argv[id_rt+7];
      if(strcmp(arg, "--desc")==0) {
        shift_counter++;
        arg=argv[id_rt+7+1];
        if(strlen(arg)>=(DEVICE_NAME_LENGTH-1))
          arg[DEVICE_NAME_LENGTH-1]=0;
        strcpy(vslave[rt_current].device_name, arg);
        shift_counter++;
        }
      }

    if(rt_current < MAX_VIRTUAL_SLAVES) rt_current++;
      else return VSLAVE_CONF_OVERFLOW; // ������� ������ � ���������� ������� �������

    argc_counter+=RTM_TABLE_PARAMETERS+shift_counter;
		}

 	return COMMAND_LINE_OK;
  }

///-----------------------------------------------------------------------------
//**** ������ ������� ������ ****
int parse_ProxyQueries()
  {
	unsigned int i;
	unsigned int j;
	unsigned int k;

  char *arg;
  int shift_counter;

	int id_qt;

	for(i=1; i<argc; i++)
	if(strcmp(argv[i], "QT")==0) {
    id_qt=i;
	
    if(argc - id_qt < PROXY_TABLE_PARAMETERS) return PQUERY_CONF_STRUCT; // ������� ������ � ��������� ����������

    // ���������
    arg=argv[id_qt+1];
    if(  ((arg[0]=='P') && (arg[1]>48) && (arg[1]<57)) ||
         ((arg[0]=='T') && (arg[1]>47) && (arg[1]<52) && (arg[2]>47) && (arg[2]<58))
      )
      query_table[qt_current].iface=\
      arg[0]=='P'?arg[1]-48-1:\
      10*(arg[1]-48)+(arg[2]-48)-1+GATEWAY_T01;
      else return PQUERY_CONF_IFACE; // ������� ������ � �������� ����������
	
    // ����� ����������
    arg=argv[id_qt+2];
    sscanf(arg, "%d", &query_table[qt_current].device);
	
    // ������� MODBUS
    arg=argv[id_qt+3];
    k=strlen(arg);
    for(j=0; j<k; j++) arg[j]=toupper(arg[j]);
         if(strcmp(arg, "COIL_STATUS")==0)      query_table[qt_current].mbf=MBF_READ_COILS; 
    else if(strcmp(arg, "INPUT_STATUS")==0)     query_table[qt_current].mbf=MBF_READ_DECRETE_INPUTS;
    else if(strcmp(arg, "HOLDING_REGISTER")==0) query_table[qt_current].mbf=MBF_READ_HOLDING_REGISTERS;
    else if(strcmp(arg, "INPUT_REGISTER")==0)   query_table[qt_current].mbf=MBF_READ_INPUT_REGISTERS;
    // else return CL_ERR_QT_CFG;

    // ����� �������
    arg=argv[id_qt+4];
    k=strlen(arg);
    for(j=0; j<k; j++) arg[j]=toupper(arg[j]);
         if(strcmp(arg, "RW")==0)  query_table[qt_current].access=QT_ACCESS_READWRITE;
    else if(strcmp(arg, "R")==0)   query_table[qt_current].access=QT_ACCESS_READONLY;
    else if(strcmp(arg, "OFF")==0) query_table[qt_current].access=QT_ACCESS_DISABLED;
    // else return CL_ERR_QT_CFG;

    arg=argv[id_qt+5];
    sscanf(arg, "%d", &query_table[qt_current].start);
    query_table[qt_current].start--;
	
    arg=argv[id_qt+6];
    sscanf(arg, "%d", &query_table[qt_current].length);
	
    /// ��������� ������� ������� ������ (����� ��������)
    arg=argv[id_qt+7];
    sscanf(arg, "%d", &query_table[qt_current].offset);
    query_table[qt_current].offset--;
	
    arg=argv[id_qt+8];
    sscanf(arg, "%d", &query_table[qt_current].delay);
	
    arg=argv[id_qt+9];
    sscanf(arg, "%d", &query_table[qt_current].critical);

    shift_counter=0;
	
    /// ������������ ����������
    if(argc > id_qt+10+1 ) {
      arg=argv[id_qt+10];
      if(strcmp(arg, "--desc")==0) {
        shift_counter++;
        arg=argv[id_qt+10+1];
        if(strlen(arg)>=(DEVICE_NAME_LENGTH-1))
          arg[DEVICE_NAME_LENGTH-1]=0;
        strcpy(query_table[qt_current].device_name, arg);
        shift_counter++;
        }
      }

    if(qt_current < MAX_QUERY_ENTRIES) qt_current++;
      else return PQUERY_CONF_OVERFLOW; // ������� ������ � ���������� ������� �������

    argc_counter+=PROXY_TABLE_PARAMETERS+shift_counter;
		}
	
 	return COMMAND_LINE_OK;
  }

///-----------------------------------------------------------------------------
//**** ������ ������� ���������� ****
int parse_Exceptions()
  {
	unsigned int i;
	unsigned int j;
	unsigned int k;

  char *arg;
  int shift_counter;

	int id_expt;

  for(i=0; i<argc; i++)
  if(strcmp(argv[i], "EXPT")==0) {
    id_expt=i;
	
      if(argc - id_expt < EXCEPTION_PARAMETERS) return EXPT_CONF_STRUCT;

			arg=argv[id_expt+1];
			k=strlen(arg);
			for(j=0; j<k; j++) arg[j]=toupper(arg[j]);
			     if(strcmp(arg, "RESPONSE_RECV_RAW")==0)  Exception[expt_current].stage=EXPT_STAGE_RESPONSE_RECV_RAW;
      else if(strcmp(arg, "QUERY_RECV_RAW")==0)     Exception[expt_current].stage=EXPT_STAGE_QUERY_RECV_RAW;
      else if(strcmp(arg, "QUERY_RECV")==0)         Exception[expt_current].stage=EXPT_STAGE_QUERY_RECV;
      else if(strcmp(arg, "QUERY_FRWD")==0)         Exception[expt_current].stage=EXPT_STAGE_QUERY_FRWD;
      else if(strcmp(arg, "RESPONSE_RECV")==0)      Exception[expt_current].stage=EXPT_STAGE_RESPONSE_RECV;
      else if(strcmp(arg, "RESPONSE_SEND")==0)      Exception[expt_current].stage=EXPT_STAGE_RESPONSE_SEND;
      else return EXPT_CONF_STAGE;


			arg=argv[id_expt+2];
			k=strlen(arg);
			for(j=0; j<k; j++) arg[j]=toupper(arg[j]);
			     if(strcmp(arg, "SKS07_DIOGEN")==0)      Exception[expt_current].action=EXPT_ACT_SKS07_DIOGEN;
      // else if(strcmp(arg, "INPUT_STATUS")==0)     vslave[i].modbus_table=INPUT_STATUS_TABLE;
      // else return CL_ERR_VSLAVES_CFG;

			arg=argv[id_expt+3];
			sscanf(arg, "%d", &Exception[expt_current].prm1);
	
			arg=argv[id_expt+4];
			sscanf(arg, "%d", &Exception[expt_current].prm2);
	
			arg=argv[id_expt+5];
			sscanf(arg, "%d", &Exception[expt_current].prm3);
	
			arg=argv[id_expt+6];
			sscanf(arg, "%d", &Exception[expt_current].prm4);
																										 
      shift_counter=0;

			/// �����������
			if(argc > id_expt+7+1) {
			  arg=argv[id_expt+7];
			  if(strcmp(arg, "--desc")==0) {
          shift_counter++;
				  arg=argv[id_expt+7+1];
				  if(strlen(arg)>=(DEVICE_NAME_LENGTH-1))
					  arg[DEVICE_NAME_LENGTH-1]=0;
				  strcpy(Exception[expt_current].comment, arg);
          shift_counter++;
				  }
        }

    if(expt_current < MOXAGATE_EXCEPTIONS_NUMBER) expt_current++;
      else return EXPT_CONF_OVERFLOW; // ������� ������ � ���������� ������� �������

    argc_counter+=EXCEPTION_PARAMETERS+shift_counter;
		}

 	return COMMAND_LINE_OK;
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

	return COMMAND_LINE_OK;
  }

///-----------------------------------------------------------------------------
