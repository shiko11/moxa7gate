/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///*********************** МОДУЛЬ ИНТЕРФЕЙСОВ ШЛЮЗА ****************************

///=== INTERFACES_H MODULE IMPLEMENTATION

#include "interfaces.h"
#include "cli.h"
#include "messages.h"

///----------------------------------------------------------------------------
// условно конструктор
int init_interfaces_h()
  {
  unsigned int i, j;
  GW_Iface *iface;
	struct timezone tz;

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
      i=j - GATEWAY_T01;
      iface=&IfaceTCP[i];
    }

    // Security

    iface->Security.start_time=0;

    iface->Security.show_data_flow   =0;
    iface->Security.show_sys_messages=0;
    iface->Security.watchdog_timer   =0;
    iface->Security.use_buzzer       =0;

    iface->Security.halt=0;

    iface->Security.tcp_port=1000*(i+1)+502;

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
    
		iface->ethernet.active_connection=0;
    iface->ethernet.status=iface->ethernet.status2=TCPMSTCON_NOTMADE;
    iface->ethernet.csd=iface->ethernet.csd2=-1;
    
	  gettimeofday(&iface->ethernet.tv, &tz);

    // iface->queue;
    clear_stat(&iface->stat); // используется для подсчета запросов из очереди интерфейса
    clear_stat(&iface->Security.stat); // используется для подсчета запросов из таблицы опроса

  ///=== условно частные переменные (private members)

  // переменные режима IFACE_TCPSERVER
    iface->ssd=-1;
    // iface->serial_mutex;

  // переменные режима IFACE_RTUMASTER, IFACE_RTUSLAVE
    // iface->rc;
    // iface->tid_srvr;

		memset(iface->PQueryIndex,  0, sizeof(iface->PQueryIndex));

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

    ///!!! надо сделать проверку на наличие дублирующихся TCP-портов
	  if( (iface->Security.tcp_port < TCP_PORT_MIN) ||
        (iface->Security.tcp_port > TCP_PORT_MAX)
      ) return IFACE_RTUTCPPORT;

		/// Описание шлейфа (сети ModBus)
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

		// Modbus Address из карты адресов, вычисляемый
    //if(  (iface->ethernet.mb_slave < MODBUS_ADDRESS_MIN) ||
    //     (iface->ethernet.mb_slave > MODBUS_ADDRESS_MAX)
    //  ) return IFACE_TCPMBADDR;

		// timeout
    if( (iface->ethernet.timeout < TIMEOUT_MIN) ||
        (iface->ethernet.timeout > TIMEOUT_MAX)
      ) return IFACE_TCPTIMEOUT;

		// LAN2Address, TCP
    // допускаем ноль для резервного адреса:
    // if(iface->ethernet.ip2==0) return IFACE_CONF_TCPIP2;
    
	  if( (iface->ethernet.port2 < TCP_PORT_MIN) ||
        (iface->ethernet.port2 > TCP_PORT_MAX)
      ) return IFACE_TCPPORT2;

		/// Описание шлейфа (сети ModBus)
    // iface->description

    // основной и резервный адерса не должны совпадать
    if(iface->ethernet.ip == iface->ethernet.ip2) return IFACE_TCPIPEQUAL;
	  }

 	return 0;
  }

///-----------------------------------------------------------------------------
int forward_response(int port_id, int client_id, u8 *req_adu, u16 req_adu_len, u8 *rsp_adu, u16 rsp_adu_len)
  {
	int status;
  GW_Iface *iface;

  if(port_id<=GATEWAY_P8) {
    iface=&IfaceRTU[port_id];
    } else if((port_id>=GATEWAY_T01)&&(port_id<=GATEWAY_T32)) {
      iface=&IfaceTCP[port_id - GATEWAY_T01];
      } else return 1;

	/// определяем тип клиента и соответственно функцию, используемую для отправки ответа
	if(Client[client_id].iface<=GATEWAY_P8) {
				
		status = crc(&rsp_adu[TCPADU_ADDRESS], 0, rsp_adu_len);
		rsp_adu[TCPADU_ADDRESS+rsp_adu_len+0] = status >> 8;
		rsp_adu[TCPADU_ADDRESS+rsp_adu_len+1] = status & 0x00FF;
		rsp_adu_len+=MB_SERIAL_CRC_LEN;

		if(Security.show_data_flow==1)
			show_traffic(TRAFFIC_RTU_SEND, port_id, client_id, &rsp_adu[TCPADU_ADDRESS], rsp_adu_len);

		///!!! необходимо сделать мьютексы для синхронизации работы нескольких потоков с одним портом

		status = mbcom_rtu_send(IfaceRTU[Client[client_id].iface].serial.fd,
														&rsp_adu[TCPADU_ADDRESS],
														rsp_adu_len);

		switch(status) {
		  case 0:
				iface->stat.sended++;
				Client[client_id].stat.sended++;
				func_res_ok(rsp_adu[TCPADU_FUNCTION], &iface->stat);
				func_res_ok(rsp_adu[TCPADU_FUNCTION], &Client[client_id].stat);
		  	break;

		  case MB_SERIAL_WRITE_ERR:
				iface->stat.errors++;
				Client[client_id].stat.errors++;
				func_res_err(rsp_adu[TCPADU_FUNCTION], &iface->stat);
				func_res_err(rsp_adu[TCPADU_FUNCTION], &Client[client_id].stat);
				stage_to_stat((MBCOM_RSP<<16) | (MBCOM_RTU_SEND<<8) | status, &iface->stat);
				stage_to_stat((MBCOM_RSP<<16) | (MBCOM_RTU_SEND<<8) | status, &Client[client_id].stat);

			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_WRN|port_id, POLL_RTU_SEND, (unsigned) status, client_id, 0, 0);

				return status;
		  	break;

		  default:;
		  }
				
		} else {

		// приводим в соответствие поля заголовка TCP-пакета
		rsp_adu[TCPADU_TRANS_HI]=req_adu[TCPADU_TRANS_HI];
		rsp_adu[TCPADU_TRANS_LO]=req_adu[TCPADU_TRANS_LO];
		rsp_adu[TCPADU_PROTO_HI]=req_adu[TCPADU_PROTO_HI];
		rsp_adu[TCPADU_PROTO_LO]=req_adu[TCPADU_PROTO_LO];
		rsp_adu[TCPADU_SIZE_HI] =((rsp_adu_len)>>8)&0xff;
		rsp_adu[TCPADU_SIZE_LO] = (rsp_adu_len)    &0xff;

		rsp_adu_len += MB_TCP_ADU_HEADER_LEN-1;

		if(Security.show_data_flow==1)
			show_traffic(TRAFFIC_TCP_SEND, port_id, client_id, rsp_adu, rsp_adu_len);

		status = mbcom_tcp_send(Client[client_id].csd,
														rsp_adu,
														rsp_adu_len);

		switch(status) {
		  case 0:
				iface->stat.sended++;
				Client[client_id].stat.sended++;
				func_res_ok(rsp_adu[TCPADU_FUNCTION], &iface->stat);
				func_res_ok(rsp_adu[TCPADU_FUNCTION], &Client[client_id].stat);
		  	break;

		  case TCP_COM_ERR_SEND:
				iface->stat.errors++;
				Client[client_id].stat.errors++;
				func_res_err(rsp_adu[TCPADU_FUNCTION], &iface->stat);
				func_res_err(rsp_adu[TCPADU_FUNCTION], &Client[client_id].stat);
				stage_to_stat((MBCOM_RSP<<16) | (MBCOM_TCP_SEND<<8) | status, &iface->stat);
				stage_to_stat((MBCOM_RSP<<16) | (MBCOM_TCP_SEND<<8) | status, &Client[client_id].stat);

			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_WRN|port_id, POLL_TCP_SEND, (unsigned) status, client_id, 0, 0);

				return status;
		  	break;

		  default:;
		  };
		}

	return status;
  }
///-----------------------------------------------------------------------------
