/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///*********************** ћќƒ”Ћ№ »Ќ“≈–‘≈…—ќ¬ ЎЋё«ј ****************************

///=== INTERFACES_H MODULE IMPLEMENTATION

#include "interfaces.h"
#include "cli.h"

///----------------------------------------------------------------------------
// условно конструктор
int init_interfaces_h()
  {
  unsigned int i, j;
  GW_Iface *iface;

	// memset(IfaceRTU,0,sizeof(IfaceRTU));
	// memset(IfaceTCP,0,sizeof(IfaceTCP));

	for(j=0; j<GATEWAY_ASSETS; j++) {
    
    if( ((j > GATEWAY_P8) && (j < GATEWAY_T01)) ||
        (j > GATEWAY_T32)
      ) continue;

    if(j<=GATEWAY_P8) {
      i=j;
      iface=&IfaceRTU[i];
    } else {
      i=j & GATEWAY_IFACE;
      iface=&IfaceTCP[i];
    }

    // Security

    iface->Security.start_time=0;

    iface->Security.show_data_flow   =0;
    iface->Security.show_sys_messages=0;
    iface->Security.watchdog_timer   =0;
    iface->Security.use_buzzer       =0;

    iface->Security.halt=0;

    iface->Security.tcp_port=1000*i+502;

    iface->Security.accepted_connections_number=0;
    iface->Security.current_connections_number =0;
    iface->Security.rejected_connections_number=0;

    // Common variables
    iface->modbus_mode=IFACE_OFF;
    strcpy(iface->bridge_status, "OFF");
    iface->description[0]=0;

    // Serial Line
    iface->serial.fd=-1;

    sprintf(iface->serial.p_name, "/dev/ttyM%d", i);
    strcpy (iface->serial.p_mode, "RS485_2w");
    strcpy (iface->serial.speed,  "9600");
    strcpy (iface->serial.parity, "none");
    iface->serial.timeout=1000000;

    // IfaceRTU[i].serial.ch_interval_timeout; ///!!!

    /// Ethernet Line
    iface->ethernet.ip      =0;
    iface->ethernet.port    =502;
    iface->ethernet.unit_id =MODBUS_ADDRESS_BROADCAST;
    iface->ethernet.offset  =0;
    iface->ethernet.mb_slave=MODBUS_ADDRESS_BROADCAST;
    iface->ethernet.ip2     =0;
    iface->ethernet.port2   =502;

    // iface->queue;
    // iface->stat;

  ///=== условно частные переменные (private members)

  // переменные режима GATEWAY_SIMPLE
    iface->ssd=-1;
    // iface->serial_mutex;

  // переменные режима IFACE_RTUMASTER, IFACE_RTUSLAVE
    // iface->rc;
    // iface->tid_srvr;

    // операции по инициализации очереди выполн€ем в специальном модуле:
    //IfaceRTU[i].queue.port_id=i;
    //memset(IfaceRTU[i].queue.queue_adu_len, 0, sizeof(IfaceRTU[i].queue.queue_adu_len));
    //IfaceRTU[i].queue.queue_start = IfaceRTU[i].queue.queue_len = 0;
    //pthread_mutex_init(&IfaceRTU[i].queue.queue_mutex, NULL);
    //IfaceRTU[i].queue.operations[0].sem_flg=0;
    //IfaceRTU[i].queue.operations[0].sem_num=i;

    // операции по инициализации очереди выполн€ем в специальном модуле:
    //IfaceTCP[i].queue.port_id=EVENT_SRC_TCPBRIDGE;
    //memset(IfaceTCP[i].queue.queue_adu_len, 0, sizeof(IfaceTCP[i].queue.queue_adu_len));
    //IfaceTCP[i].queue.queue_start = IfaceTCP[i].queue.queue_len = 0;
    //pthread_mutex_init(&IfaceTCP[i].queue.queue_mutex, NULL);
    //IfaceTCP[i].queue.operations[0].sem_flg=0;
    //IfaceTCP[i].queue.operations[0].sem_num=MAX_MOXA_PORTS*2+i;

    }

  return 0;
  }

///----------------------------------------------------------------------------
int check_Iface(GW_Iface *iface)
  {
																 
  if(!(
       iface->modbus_mode==IFACE_TCPSERVER ||
       iface->modbus_mode==IFACE_RTUMASTER ||
       iface->modbus_mode==IFACE_RTUSLAVE  ||
       iface->modbus_mode==IFACE_TCPMASTER
    )) return IFACE_MBMODE;

	if(iface->modbus_mode!=IFACE_TCPMASTER) {

	   if(!( // port mode
		    strcmp(iface->serial.p_mode, "RS232")==0    ||
		    strcmp(iface->serial.p_mode, "RS422")==0    ||
		    strcmp(iface->serial.p_mode, "RS485_2w")==0 ||
		    strcmp(iface->serial.p_mode, "RS485_4w")==0
		  )) return IFACE_RTUPHYSPROT;

    if(!( // port speed
		    strcmp(iface->serial.speed, "2400")==0  ||
		    strcmp(iface->serial.speed, "4800")==0  ||
		    strcmp(iface->serial.speed, "9600")==0  ||
		    strcmp(iface->serial.speed, "14400")==0 ||
		    strcmp(iface->serial.speed, "19200")==0 ||
		    strcmp(iface->serial.speed, "38400")==0 ||
		    strcmp(iface->serial.speed, "56000")==0 ||
		    strcmp(iface->serial.speed, "57600")==0 ||
		    strcmp(iface->serial.speed, "115200")==0
		  )) return IFACE_RTUSPEED;

    if(!( // parity
		    strcmp(iface->serial.parity, "none")==0 ||
		    strcmp(iface->serial.parity, "even")==0 ||
		    strcmp(iface->serial.parity, "odd")==0
		  )) return IFACE_RTUPARITY;

		// timeout
    if( (iface->serial.timeout < TIMEOUT_MIN) ||
        (iface->serial.timeout > TIMEOUT_MAX)
      ) return IFACE_RTUTIMEOUT;

    ///!!! надо сделать проверку на наличие дублирующихс€ TCP-портов
	  if( (iface->Security.tcp_port < TCP_PORT_MIN) ||
        (iface->Security.tcp_port > TCP_PORT_MAX)
      ) return IFACE_RTUTCPPORT;

		/// ќписание шлейфа (сети ModBus)
    // iface->description

	  } else {

		// LAN1Address, TCP
    if(iface->ethernet.ip==0) return IFACE_TCPIP1;
    
	  if( (iface->ethernet.port < TCP_PORT_MIN) ||
        (iface->ethernet.port > TCP_PORT_MAX)
      ) return IFACE_TCPPORT1;

		// Unit ID
    if(  (iface->ethernet.unit_id < MODBUS_ADDRESS_MIN) ||
         (iface->ethernet.unit_id > MODBUS_ADDRESS_MAX)
      ) return IFACE_TCPUNITID;

		// Offset
    // iface->ethernet.offset

		// Modbus Address for ATM
    if(  (iface->ethernet.mb_slave < MODBUS_ADDRESS_MIN) ||
         (iface->ethernet.mb_slave > MODBUS_ADDRESS_MAX)
      ) return IFACE_TCPMBADDR;

		// LAN2Address, TCP
    // допускаем ноль дл€ резервного адреса:
    // if(iface->ethernet.ip2==0) return IFACE_CONF_TCPIP2;
    
	  if( (iface->ethernet.port2 < TCP_PORT_MIN) ||
        (iface->ethernet.port2 > TCP_PORT_MAX)
      ) return IFACE_TCPPORT2;

		/// ќписание шлейфа (сети ModBus)
    // iface->description

    // основной и резервный адерса не должны совпадать
    if(iface->ethernet.ip == iface->ethernet.ip2) return IFACE_TCPIPEQUAL;
	  }

 	return 0;
  }

///-----------------------------------------------------------------------------
