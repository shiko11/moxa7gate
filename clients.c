/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///****************** РАБОТА С КЛИЕНТСКИМИ СОЕДИНЕНИЯМИ ************************

///=== CLIENTS_H MODULE IMPLEMENTATION

#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>

#include "clients.h"
#include "interfaces.h"
#include "moxagate.h"
#include "messages.h"
#include "hmi_web.h"

///=== CLIENTS_H private variables

 	int current_client; // номер свободного соединения
	int ssd;						// системный идентификатор TCP-порта для приема входящих соединений

	// переменные мультиплексированного ввода
  // ввод мультиплексируется только от клиентов типа GW_CLIENT_TCP_502
	fd_set watchset;
	fd_set inset;
	
///=== CLIENTS_H private functions

  int gateway_single_port_processing();
  int query_translating(); ///!!! алгоритм перенаправления оформить отдельно и перенести в модуль FRWD_QUEUE_H

///----------------------------------------------------------------------------
// условно конструктор
int init_clients()
  {
	int j;

	memset(Client, 0, sizeof(Client));

	for(j=0; j<MOXAGATE_CLIENTS_NUMBER; j++) {
		Client[j].rc=1;
		Client[j].csd=-1;
		Client[j].status=GW_CLIENT_CLOSED;
	  }
								 
  // Security

	Security.start_time=0;

	Security.Object       [0]=0;
	Security.Location     [0]=0;
	Security.Label        [0]=0;
	Security.NetworkName  [0]=0;
	Security.LAN1Address     =0;
	Security.LAN2Address     =0;
	Security.VersionNumber[0]=0;
	Security.VersionTime  [0]=0;
	Security.Model        [0]=0;

  Security.show_data_flow   =0;
  Security.show_sys_messages=0;
  Security.watchdog_timer   =0;
  Security.use_buzzer       =0;

 	Security.halt=0;

	Security.tcp_port=502;

  Security.accepted_connections_number=0;
  Security.current_connections_number=0;
  Security.rejected_connections_number=0;

  for(j=0; j<MAX_TCP_SERVERS; j++)
    Security.TCPIndex[j]=GATEWAY_NONE;
  Security.TCPIndex[MAX_TCP_SERVERS]=0;

  // private variables
	ssd=-1;
  current_client=0;

  return 0;
  }

///----------------------------------------------------------------------------
// условно деструктор
int close_clients()
  {
  int i;

	for(i=0; i<MOXAGATE_CLIENTS_NUMBER; i++)
    clear_client(i);
	shutdown(ssd, SHUT_RDWR);
	close(ssd);

  return 0;
  }

///----------------------------------------------------------------------------
int check_Security()
  {
  // ОБЩАЯ ИНФОРМАЦИЯ

  // ПАРАМЕТРЫ РАБОТЫ ПРОГРАММЫ

	if( (Security.tcp_port < TCP_PORT_MIN) ||
      (Security.tcp_port > TCP_PORT_MAX)
    ) return SECURITY_TCPPORT;

  if(  (MoxaDevice.modbus_address < MODBUS_ADDRESS_MIN) ||
       (MoxaDevice.modbus_address > MODBUS_ADDRESS_MAX)
    ) return MOXAGATE_MBADDR;
	
	if((unsigned int)(MoxaDevice.status_info+GATE_STATUS_BLOCK_LENGTH) > MB_ADDRESS_LAST)
    return MOXAGATE_STATINFO;

  // Security.show_sys_messages
	// MoxaDevice.map2Xto4X

  // ОПЦИИ

	// Security.watchdog_timer
	// Security.show_data_flow
	// Security.use_buzzer

 	return 0;
  }

///-----------------------------------------------------------------------------
int clear_client(int client)
  {

  switch(Client[client].status) {

    case GW_CLIENT_CLOSED: break;
    case GW_CLIENT_ERROR: break;

    case GW_CLIENT_TCP_GWS:
		  close(Client[client].csd);
			Client[client].csd=-1;
			Client[client].status=GW_CLIENT_CLOSED;

		  Security.current_connections_number--;
		  IfaceRTU[Client[client].iface].Security.current_connections_number--;

			time(&Client[client].disconnection_time);
      break;

    case GW_CLIENT_TCP_502:

		  FD_CLR(Client[client].csd, &watchset);
		  shutdown(Client[client].csd, SHUT_RDWR);

		  close(Client[client].csd);
		  Client[client].csd=-1;
		  Client[client].status=GW_CLIENT_CLOSED;

			Security.current_connections_number--;

			time(&Client[client].disconnection_time);
      break;

    case GW_CLIENT_RTU_SLV: break;
    default:;
    }

  return 0;
  }
///----------------------------------------------------------------------------
int init_main_socket()
  {																																							 
	struct sockaddr_in	addr;
//	int csd;

	// SOCKET INITIALIZED
	ssd = socket(AF_INET, SOCK_STREAM, 0);
	if (ssd < 0) {
		perror("csdet");
		sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SECURITY, 65, 1, MOXAGATE_CLIENTS_NUMBER, 0, 0);
		return 1;
		}
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(Security.tcp_port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	//РТЙЧСЪЩЧБЕН УПЛЕФ 
	if (bind(ssd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		close(ssd);
		sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SECURITY, 65, 2, MOXAGATE_CLIENTS_NUMBER, 0, 0);
		return 2;
		}

	listen(ssd, MOXAGATE_CLIENTS_NUMBER);
	fcntl(ssd, F_SETFL, fcntl(ssd, F_GETFL, 0) | O_NONBLOCK);

	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|GATEWAY_SECURITY, 65, 3, MOXAGATE_CLIENTS_NUMBER, 0, 0);

  return 0;
  }

///----------------------------------------------------------------------------
int get_current_client()
  {
  static int j;

  // алгоритм выбора свободного клиентского слота реализует циклическое размещение клиентов
  // в буфере для обеспечения возможности накопления информации об истории подключений

  for(j=0; j<MOXAGATE_CLIENTS_NUMBER; j++)
    if(Client[j].status==GW_CLIENT_CLOSED) break;
//    if(Client[j].rc!=0) break;
//    if(gate502.clients[j].csd==-1) break;

  if(j==MOXAGATE_CLIENTS_NUMBER)
    for(j=0; j<MOXAGATE_CLIENTS_NUMBER; j++)
      if(Client[j].status==GW_CLIENT_ERROR) break;

  return j;
  }

///----------------------------------------------------------------------------
int gateway_common_processing()
  {
	struct sockaddr_in	addr; ///!!! static
  int i, j, csd, rc; ///!!! static

	FD_ZERO(&watchset);

	while (1) {
    // прием входящих соединений к портам в режиме GATEWAY_SIMPLE и создание клиентских потоков:
	  for(i=0; i<MAX_MOXA_PORTS; i++) {

	  	///!!! создать массив с индексами портов в этом режиме
		  if(IfaceRTU[i].modbus_mode!=GATEWAY_SIMPLE) continue;
	  	
		rc=sizeof(addr);
		if(IfaceRTU[i].ssd>=0) csd = accept(IfaceRTU[i].ssd, (struct sockaddr *)&addr, &rc);
		  else continue;
			
		if((csd<0)&&(errno==EAGAIN)) {
				//printf("noop\n");
				continue;
			  }
			  
			if (csd < 0) {
				perror("accept");
				close(IfaceRTU[i].ssd);
				IfaceRTU[i].ssd=-1;
				IfaceRTU[i].modbus_mode=MODBUS_PORT_ERROR;
	      strcpy(IfaceRTU[i].bridge_status, "ERR");
				// CONNECTION ACCEPTED
				sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|i, 67, 0, 0, 0, 0);
			  }

     /// ищем свободный слот для нового соединения
		 current_client=get_current_client();

	   if((IfaceRTU[i].Security.current_connections_number==MAX_TCP_CLIENTS_PER_PORT) ||
	      (current_client==MOXAGATE_CLIENTS_NUMBER)) {

			 Security.rejected_connections_number++;
			 IfaceRTU[i].Security.rejected_connections_number++;
			 
			 // CONNECTION REJECTED
			 sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|i, 70, addr.sin_addr.s_addr, 0, 0, 0);
			 
			 close(csd);
	   	 continue;
	     }
	  Client[current_client].csd=csd;
	  	
		time(&Client[current_client].connection_time);
    Client[current_client].disconnection_time=0;
		Client[current_client].ip=addr.sin_addr.s_addr;
		Client[current_client].port=addr.sin_port;
		Client[current_client].status=GW_CLIENT_TCP_GWS;
		clear_stat(&Client[current_client].stat);			 
		sprintf(Client[current_client].device_name, "%d.%d.%d.%d", \
			addr.sin_addr.s_addr >> 24, (addr.sin_addr.s_addr >> 16) & 0xff, \
			(addr.sin_addr.s_addr >> 8) & 0xff, addr.sin_addr.s_addr & 0xff);
    Client[current_client].iface=i;

		//printf("ip%X\n", addr.sin_addr.s_addr);
			
		  Security.accepted_connections_number++;
		  Security.current_connections_number++;
		  IfaceRTU[i].Security.accepted_connections_number++;
		  IfaceRTU[i].Security.current_connections_number++;

			// CONNECTION ACCEPTED
			sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|i, 67, addr.sin_addr.s_addr, current_client, 0, 0);
			
			int arg=(i<<8)|(current_client&0xff);
			Client[current_client].rc = pthread_create(
				&Client[current_client].tid_srvr,
				NULL,
				iface_rtu_gws,
				(void *) arg);
			
			if(Client[current_client].rc!=0) {
				close(IfaceRTU[i].ssd);
				IfaceRTU[i].ssd=-1;
				IfaceRTU[i].modbus_mode=MODBUS_PORT_ERROR;
	      strcpy(IfaceRTU[i].bridge_status, "ERR");

				clear_client(current_client);
				Client[current_client].status=GW_CLIENT_ERROR;

	      // THREAD INITIALIZED
				sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_ERR|i, 41, Client[current_client].rc, 0, 0, 0);
			  }
	    }

	  ///usleep(750000); ///!!!

		/// обработка входящих соединений на порт 502
		gateway_single_port_processing();
		/// механизм трансляции запросов
		query_translating();
	  
		// останов программы по внешней команде
		if(Security.halt==1) break;
		}

  // usleep(4000000); /// перед выходом из программы ожидаем, пока cgi-скрипт освободит разделяемую память
	return 0;
  }
///-----------------------------------------------------------------------------------------------------------------
	//pthread_join(csd, NULL);
int gateway_single_port_processing()
	{
	struct sockaddr_in	addr; ///!!! static
  int i, j, k, csd, rc; ///!!! static

			/// блок обработки входящих TCP-соединений

		 rc=sizeof(addr);
		 csd=-1;
		 if(ssd>=0) csd=accept(ssd, (struct sockaddr *)&addr, &rc);
       else return 1;
			
			if(!((csd<0)&&(errno==EAGAIN))) // если есть новое подключение
			if (csd < 0) {
				perror("accept");
				// CONNECTION ACCEPTED
				sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SECURITY, 67, 0, 0, 0, 0);
			} else {

		 /// ищем свободный слот для нового соединения
		 current_client=get_current_client();

	   if(current_client==MOXAGATE_CLIENTS_NUMBER) { // если нет свободных клиентских слотов

			 Security.rejected_connections_number++;
			 // CONNECTION REJECTED
			 sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|GATEWAY_SECURITY, 70, addr.sin_addr.s_addr, 0, 0, 0);
			 close(csd);

	     } else {
	   		  Client[current_client].csd=csd;
				  FD_SET(Client[current_client].csd, &watchset);
	  	
				 // CONNECTION ACCEPTED
				 sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|GATEWAY_SECURITY, 67, addr.sin_addr.s_addr, current_client, 0, 0);
			
				 time(&Client[current_client].connection_time);
				 Client[current_client].disconnection_time=0;
				 time(&Client[current_client].last_activity_time);
			   Client[current_client].ip=addr.sin_addr.s_addr;
			   Client[current_client].port=addr.sin_port;
			   Client[current_client].status=GW_CLIENT_TCP_502;
			   clear_stat(&Client[current_client].stat);
				 sprintf(Client[current_client].device_name, "%d.%d.%d.%d", \
					 addr.sin_addr.s_addr >> 24, (addr.sin_addr.s_addr >> 16) & 0xff, \
					 (addr.sin_addr.s_addr >> 8) & 0xff, addr.sin_addr.s_addr & 0xff);
			//printf("ip%X\n", addr.sin_addr.s_addr);
         Client[current_client].iface=GATEWAY_SECURITY;
			
		  	Security.accepted_connections_number++;
		  	Security.current_connections_number++;
				}
		  }


	  //usleep(500000); // proverit' zaderzhku na obrabotku vseh klientov v rezhime MASTER

	return 0;
	}
///-----------------------------------------------------------------------------------------------------------------
int query_translating()
  {
  int i, j, k;
	struct sockaddr_in	addr;

	struct timeval stv;
	stv.tv_sec=0; stv.tv_usec=0;

		//printf("input data processing...\n");

		///### блок обработки входящих ModbusTCP запросов

		u8			tcp_adu[MB_TCP_MAX_ADU_LENGTH];// TCP ADU
		u16			tcp_adu_len;
		GW_StaticData tmpstat;
		int port_id, device_id;
		int status;

		unsigned temp;
		int maxfd;
		maxfd=0;

    int diff;
	  time_t moment;
    time(&moment);

	  for(i=0; i<MOXAGATE_CLIENTS_NUMBER; i++) { // принимаем данные от клиентов

			if(Client[i].status!=GW_CLIENT_TCP_502) continue;

			if(Client[i].csd!=-1)
			  if(FD_ISSET(Client[i].csd, &watchset)) {
					diff=difftime(moment, Client[i].last_activity_time);
	    		if(diff>=MAX_CLIENT_ACTIVITY_TIMEOUT) {
            clear_client(i);
			 			// CONNECTION CLOSED (TIMEOUT)
			 			sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|GATEWAY_SECURITY, 72, i, 0, 0, 0);
						};
      }

	    if(maxfd<Client[i].csd) maxfd=Client[i].csd;
	    }

			clear_stat(&tmpstat);
			
			inset=watchset;

			if((temp=select(maxfd+1, &inset, NULL, NULL, &stv))<0) {
				perror("select");
				return 0;
				}
		if(temp==0) return 0;
																												 
	  for(i=0; i<MOXAGATE_CLIENTS_NUMBER; i++) 
			if (Client[i].status==GW_CLIENT_TCP_502)
			if (FD_ISSET(Client[i].csd, &inset)) {
	  	
			status = mb_tcp_receive_adu(Client[i].csd, &tmpstat, tcp_adu, &tcp_adu_len);
	
			if(Security.show_data_flow==1)
				show_traffic(TRAFFIC_TCP_RECV, GATEWAY_SECURITY, i, tcp_adu, tcp_adu_len);

			switch(status) {
			  case 0:
			  	break;
			  case TCP_COM_ERR_NULL:
			  case TCP_ADU_ERR_MIN:
			  case TCP_ADU_ERR_MAX:
			  case TCP_ADU_ERR_PROTOCOL:
			  case TCP_ADU_ERR_LEN:
			  case TCP_ADU_ERR_UID:
			  case TCP_PDU_ERR:
			  	//tmpstat.errors++;
	  			// POLLING: TCP RECV
				 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_ERR|GATEWAY_SECURITY, 184, (unsigned) status, i, 0, 0);

					//update_stat(&inputDATA->clients[client_id].stat, &tmpstat);
					//update_stat(&IfaceRTU[port_id].stat, &tmpstat);
				  if(status==TCP_COM_ERR_NULL) {
						clear_client(i);
			 			// CONNECTION CLOSED (LINK DOWN)
			 			sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|GATEWAY_SECURITY, 071, i, 0, 0, 0);
						}
					return 0;
			  	break;
			  default:; ///!!! continue;
			  };

			time(&Client[i].last_activity_time);


/* разбор входящего запроса происходит в несколько этапов:
	- проверяем modbus адрес, указанный в запросе;
	- проверяем диапазон регистров, указанный в запросе;
	- если и адрес и диапазон принадлежат устройству Moxa, ставим запрос в очередь внутренних запросов;
	- если только адрес принадлежит устройству Moxa, ставим запрос в очередь одного из портов режима RTM;
	- если адрес не принадлежит устройству Moxa, ставим запрос в очередь одного из портов режима ATM.
 */

			k=tcp_adu[TCPADU_ADDRESS]==MoxaDevice.modbus_address?GATEWAY_SIMPLE:GATEWAY_ATM;

			switch(tcp_adu[TCPADU_FUNCTION]) { 	// определяем диапазон регистров, требуемых в запросе,
																					// а также фильтруем входящие запросы по функциям ModBus
				case MBF_READ_COILS:
				case MBF_READ_DECRETE_INPUTS:
				case MBF_READ_HOLDING_REGISTERS:
				case MBF_READ_INPUT_REGISTERS:
		
				case MBF_WRITE_MULTIPLE_COILS:
				case MBF_WRITE_MULTIPLE_REGISTERS:
		
					j=(tcp_adu[TCPADU_LEN_HI]<<8)|tcp_adu[TCPADU_LEN_LO];
					break;

				case MBF_WRITE_SINGLE_COIL:
				case MBF_WRITE_SINGLE_REGISTER:
					j=1;
					break;
		
				default: ///!!! функция не поддерживается шлюзом, добавить счетчик статистики
					// POLLING: FUNC NOT SUPPORTED
			 		sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_WRN|GATEWAY_SECURITY, 180, i, (unsigned) tcp_adu[TCPADU_FUNCTION], 0, 0);
					continue;
				}
	
			if(k!=GATEWAY_ATM) {
				switch(
							checkDiapason(	tcp_adu[TCPADU_FUNCTION],
															(tcp_adu[TCPADU_START_HI]<<8)|tcp_adu[TCPADU_START_LO],
															j)) {
	
					case 	MOXA_DIAPASON_INSIDE:
						k=GATEWAY_PROXY;
						/// ставим запрос в очередь MOXA MODBUS DEVICE
						if((status=enqueue_query_ex(&MoxaDevice.queue, i, device_id, tcp_adu, tcp_adu_len))!=0) continue;
//						printf("GATE502 GATEWAY_PROXY\n");
						break;
					
					case MOXA_DIAPASON_OUTSIDE:
						k=GATEWAY_RTM;
	
						status=translateRegisters(
							(tcp_adu[TCPADU_START_HI]<<8)|tcp_adu[TCPADU_START_LO],
							j, &port_id, &device_id);

					  if(status) {
		  			// FRWD: REGISTERS TRANSLATION
			 			sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_ERR|GATEWAY_SECURITY, 131, i, (unsigned) (tcp_adu[TCPADU_START_HI]<<8)|tcp_adu[TCPADU_START_LO], (unsigned) j, 0);
							continue;
							}

						/// ставим запрос в очередь последовательного порта
//						if((status=enqueue_query(port_id, i, device_id, tcp_adu, tcp_adu_len))!=0) continue;
						if((status=enqueue_query_ex(&IfaceRTU[port_id].queue, i, device_id, tcp_adu, tcp_adu_len))!=0) continue;
//						printf("GATE502 GATEWAY_RTM\n");
						break;
					
					case MOXA_DIAPASON_OVERLAPPED:
		  			// FRWD: BLOCK OVERLAPS
			 			sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_ERR|GATEWAY_SECURITY, 129, i, (unsigned) (tcp_adu[TCPADU_START_HI]<<8)|tcp_adu[TCPADU_START_LO], (unsigned) j, 0);
						break;
	
					default:;
					}
				} else { /// обработка запроса в режиме ATM

					status=translateAddress(tcp_adu[TCPADU_ADDRESS], &port_id, &device_id);

				  if(status) {
		  			// FRWD: ADDRESS TRANSLATION
			 			sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_ERR|GATEWAY_SECURITY, 128, i, (unsigned) tcp_adu[TCPADU_ADDRESS], 0, 0);
						continue;
						}

					tcp_adu[TCPADU_ADDRESS]=device_id;

					/// ставим запрос в очередь последовательного порта
//printf("enqueue_query port_id=%d, client_id=%d, device_id=%d\n", port_id, i, device_id);
//					if((status=enqueue_query(port_id, i, device_id, tcp_adu, tcp_adu_len))!=0) continue;
						if((status=enqueue_query_ex(&IfaceRTU[port_id].queue, i, device_id, tcp_adu, tcp_adu_len))!=0) continue;
					}

	    }

  return 0;
  }
///-----------------------------------------------------------------------------------------------------------------
