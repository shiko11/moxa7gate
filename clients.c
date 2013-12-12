/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///****************** РАБОТА С КЛИЕНТСКИМИ СОЕДИНЕНИЯМИ ************************

///=== CLIENTS_H MODULE IMPLEMENTATION

#include <string.h>
#include <stdio.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>

#include "interfaces.h"
#include "clients.h"
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
		Client[j].iface=GATEWAY_NONE;
		Client[j].rc=1;
		Client[j].csd=-1;
		Client[j].status=GW_CLIENT_IDLE;
		clear_stat(&Client[j].stat);
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

  ///!!!
	Security.timestamp=0;  // время последнего обновления данных для web-интерфейса
 	Security.halt=0;
  Security.back_light=1;

	Security.tcp_port=502;

  Security.accepted_connections_number=0;
  Security.current_connections_number=0;
  Security.rejected_connections_number=0;
  
  clear_stat(&Security.stat);
  
  Security.scan_counter=0;

  for(j=0; j<MAX_TCP_SERVERS; j++)
    Security.TCPIndex[j]=GATEWAY_NONE;
  Security.TCPIndex[MAX_TCP_SERVERS]=0;

  for(j=0; j<MAX_MOXA_PORTS; j++)
    Security.TCPSRVIndex[j]=GATEWAY_NONE;
  Security.TCPSRVIndex[MAX_MOXA_PORTS]=0;

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

	//fcntl(ssd, F_SETFL, fcntl(ssd, F_GETFL, 0) & (~O_NONBLOCK));
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

    case GW_CLIENT_IDLE: break;
    case GW_CLIENT_ERROR: break;

    case GW_CLIENT_TCP_GWS:
		  close(Client[client].csd);
			Client[client].csd=-1;
			Client[client].status=GW_CLIENT_IDLE;

		  IfaceRTU[Client[client].iface].Security.current_connections_number--;

			time(&Client[client].disconnection_time);
      break;

    case GW_CLIENT_TCP_502:

		  FD_CLR(Client[client].csd, &watchset);
		  shutdown(Client[client].csd, SHUT_RDWR);

		  close(Client[client].csd);
		  Client[client].csd=-1;
		  Client[client].status=GW_CLIENT_IDLE;

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

	// SOCKET INITIALIZED
	ssd = socket(AF_INET, SOCK_STREAM, 0);
	if (ssd < 0) {
		perror("csdet");
		sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SECURITY, TCPCON_INITIALIZED, 1, 0, 0, 0);
		return 1;
		}
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(Security.tcp_port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	//РТЙЧСЪЩЧБЕН УПЛЕФ 
	if (bind(ssd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		close(ssd);
		sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SECURITY, TCPCON_INITIALIZED, 2, 0, 0, 0);
		return 2;
		}

	listen(ssd, MOXAGATE_CLIENTS_NUMBER);
	fcntl(ssd, F_SETFL, fcntl(ssd, F_GETFL, 0) | O_NONBLOCK);

	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|GATEWAY_SECURITY, TCPCON_INITIALIZED, 3, 0, 0, 0);

  return 0;
  }

///----------------------------------------------------------------------------
int get_current_client()
  {
  static int j;

  // алгоритм выбора свободного клиентского слота реализует циклическое размещение клиентов
  // в буфере для обеспечения возможности накопления информации об истории подключений

  for(j=0; j<MOXAGATE_CLIENTS_NUMBER; j++)
    if(Client[j].status==GW_CLIENT_IDLE) break;
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
	struct sockaddr_in	addr;
  int i, j, P, T, csd, rc;

	struct timeval tv;
	struct timezone tz;

	int arg;

	FD_ZERO(&watchset);

	while (1) {

		gettimeofday(&tv, &tz);

    // прием входящих соединений к интерфейсам IFACE_TCPSERVER и создание клиентских потоков:
	  for(i=0; i<Security.TCPSRVIndex[MAX_MOXA_PORTS]; i++) {

		  P=Security.TCPSRVIndex[i];
		  if(IfaceRTU[P].modbus_mode!=IFACE_TCPSERVER) continue;
	  	
		  rc=sizeof(addr);
		  if(IfaceRTU[P].ssd>=0) csd = accept(IfaceRTU[P].ssd, (struct sockaddr *)&addr, &rc);
		    else continue;
			
			// запросов на подключение от клиентов нет
		  if((csd<0)&&(errno==EAGAIN)) continue;
			  
			if (csd < 0) {
				perror("accept");
				close(IfaceRTU[P].ssd);
				IfaceRTU[P].ssd=-1;
				IfaceRTU[P].modbus_mode=IFACE_ERROR;
	      strcpy(IfaceRTU[P].bridge_status, "ERR");
				// CONNECTION ACCEPTED
				sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|P, TCPCON_ACCEPTED, 0, 0, 0, 0);
			  }

     /// ищем свободный слот для нового соединения
		 current_client=get_current_client();

	   if((IfaceRTU[P].Security.current_connections_number==MAX_TCP_CLIENTS_PER_PORT) ||
	      (current_client==MOXAGATE_CLIENTS_NUMBER)) {

			 IfaceRTU[P].Security.rejected_connections_number++;
			 
			 // CONNECTION REJECTED
			 if(current_client==MOXAGATE_CLIENTS_NUMBER)
			   sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|P, CLIENT_NOTAVAIL, addr.sin_addr.s_addr, 0, 0, 0);
			   else
			   sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|P, TCPCON_REJECTED, addr.sin_addr.s_addr, 0, 0, 0);
			 
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
		sprintf(Client[current_client].device_name, "%d.%d.%d.%d",
			addr.sin_addr.s_addr >> 24, (addr.sin_addr.s_addr >> 16) & 0xff,
			(addr.sin_addr.s_addr >> 8) & 0xff, addr.sin_addr.s_addr & 0xff);
    Client[current_client].iface=P;

		//printf("ip%X\n", addr.sin_addr.s_addr);
			
		  IfaceRTU[P].Security.accepted_connections_number++;
		  IfaceRTU[P].Security.current_connections_number++;

			// CONNECTION ACCEPTED
			sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|P, TCPCON_ACCEPTED, addr.sin_addr.s_addr, current_client, 0, 0);
			
			arg=(P<<8)|(current_client&0xff);
			Client[current_client].rc = pthread_create(
				&Client[current_client].tid_srvr,
				NULL,
				iface_tcp_server,
				(void *) arg);
			
			if(Client[current_client].rc!=0) {
				close(IfaceRTU[P].ssd);
				IfaceRTU[P].ssd=-1;
				IfaceRTU[P].modbus_mode=IFACE_ERROR;
	      strcpy(IfaceRTU[P].bridge_status, "ERR");

				clear_client(current_client);
				Client[current_client].status=GW_CLIENT_ERROR;

	      // THREAD INITIALIZED
				sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|P, IFACE_THREAD_INIT, Client[current_client].rc, 0, 0, 0);
			  }
	    }

		/// обслуживание TCP-соединений TCP MASTER интерфейсов
	  for(i=0; i<Security.TCPIndex[MAX_TCP_SERVERS]; i++) {
	
		  T=Security.TCPIndex[i];
		  if(IfaceTCP[T].modbus_mode!=IFACE_TCPMASTER) continue;
	
			if( // перенести в модуль HMI_WEB
			  ((IfaceTCP[T].ethernet.status==TCPMSTCON_ESTABLISHED)&&(IfaceTCP[T].ethernet.status2==TCPMSTCON_ESTABLISHED))||
			  ((IfaceTCP[T].ethernet.status==TCPMSTCON_ESTABLISHED)&&(IfaceTCP[T].ethernet.status2==TCPMSTCON_NOTUSED))
			  ) strcpy(IfaceTCP[T].bridge_status, " OK");
			  
			// с установленной периодичностью проверяем состояние сетевых соединений
	    if( (	(tv.tv_sec- IfaceTCP[T].ethernet.tv.tv_sec ) * 1000000 +
	    			(tv.tv_usec-IfaceTCP[T].ethernet.tv.tv_usec) ) >= TCP_RECONN_INTVL) {

				if(IfaceTCP[T].ethernet.status!=TCPMSTCON_ESTABLISHED) {
				  strcpy(IfaceTCP[T].bridge_status, "MRG");
				  reset_tcpmaster_conn(&IfaceTCP[T], 1);
				  }
				  
				if( (IfaceTCP[T].ethernet.status2!=TCPMSTCON_ESTABLISHED) &&
				    (IfaceTCP[T].ethernet.status2!=TCPMSTCON_NOTUSED)     ){
				  strcpy(IfaceTCP[T].bridge_status, "MRG");
				  reset_tcpmaster_conn(&IfaceTCP[T], 2);
				  }

			  gettimeofday(&IfaceTCP[T].ethernet.tv, &tz);
				}
			}

		/// обработка входящих соединений на порт 502
		gateway_single_port_processing();
		/// трансляция запросов от клиентов GW_CLIENT_TCP_502
		query_translating();
	  
    // обновляем динамические данные для web-интерфейса
    refresh_shm();
    // обновляем динамические данные блока диагностики шлюза
    refresh_status_info();

		// останов программы по внешней команде
		if(Security.halt==1) break;

		// инкрементируем значение счетчика циклов сканирования главного потока программы
		Security.scan_counter++;
		
		// выполняем сброс watch-dog таймера
		if(Security.watchdog_timer==1) mxwdg_refresh(MoxaDevice.mxwdt_handle);
		}

  // usleep(4000000); /// перед выходом из программы ожидаем, пока cgi-скрипт освободит разделяемую память
	return 0;
  }
///-----------------------------------------------------------------------------------------------------------------
	//pthread_join(csd, NULL);
int gateway_single_port_processing()
	{
	struct sockaddr_in	addr;
  int i, j, k, csd, rc;

			/// блок обработки входящих TCP-соединений

		 rc=sizeof(addr);
		 csd=-1;
		 if(ssd>=0) csd=accept(ssd, (struct sockaddr *)&addr, &rc);
       else return 1;
			
			if(!((csd<0)&&(errno==EAGAIN))) // если есть новое подключение
			if (csd < 0) {
				perror("accept");
				// CONNECTION ACCEPTED
				sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_SECURITY, TCPCON_ACCEPTED, 0, 0, 0, 0);
			} else {

		 /// ищем свободный слот для нового соединения
		 current_client=get_current_client();

	   if(current_client==MOXAGATE_CLIENTS_NUMBER) { // если нет свободных клиентских слотов

			 Security.rejected_connections_number++;
			 // CONNECTION REJECTED
			 sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|GATEWAY_SECURITY, CLIENT_NOTAVAIL, addr.sin_addr.s_addr, 0, 0, 0);
			 close(csd);

	     } else {
	   		  Client[current_client].csd=csd;
				  FD_SET(Client[current_client].csd, &watchset);
	  	
				 // CONNECTION ACCEPTED
				 sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|GATEWAY_SECURITY, TCPCON_ACCEPTED, addr.sin_addr.s_addr, current_client, 0, 0);
			
				 time(&Client[current_client].connection_time);
				 Client[current_client].disconnection_time=0;
				 time(&Client[current_client].last_activity_time);
			   Client[current_client].ip=addr.sin_addr.s_addr;
			   Client[current_client].port=addr.sin_port;
			   Client[current_client].status=GW_CLIENT_TCP_502;
			   clear_stat(&Client[current_client].stat);
				 sprintf(Client[current_client].device_name, "%d.%d.%d.%d",
					 addr.sin_addr.s_addr >> 24, (addr.sin_addr.s_addr >> 16) & 0xff,
					 (addr.sin_addr.s_addr >> 8) & 0xff, addr.sin_addr.s_addr & 0xff);
			//printf("ip%X\n", addr.sin_addr.s_addr);
         Client[current_client].iface=GATEWAY_SECURITY;
			
		  	Security.accepted_connections_number++;
		  	Security.current_connections_number++;
				}
		  }

	return 0;
	}
///-----------------------------------------------------------------------------------------------------------------
int query_translating()
  {
  int i, j, k;
	struct sockaddr_in	addr;

	struct timeval stv;

  //printf("input data processing...\n");

  ///### блок обработки входящих ModbusTCP запросов

  u8			tcp_adu[MB_TCP_MAX_ADU_LENGTH];// TCP ADU
  u16			tcp_adu_len;

  int status;

  unsigned temp;
  int maxfd;

  int diff;
  time_t moment;

	stv.tv_sec=0; stv.tv_usec=0;

  maxfd=0;

  time(&moment);

	  for(i=0; i<MOXAGATE_CLIENTS_NUMBER; i++) { // принимаем данные от клиентов

			if(Client[i].status!=GW_CLIENT_TCP_502) continue;

			if(Client[i].csd!=-1)
			  if(FD_ISSET(Client[i].csd, &watchset)) {
					diff=difftime(moment, Client[i].last_activity_time);
	    		if(diff>=MAX_CLIENT_ACTIVITY_TIMEOUT) {
            clear_client(i);
			 			// CONNECTION CLOSED (TIMEOUT)
			 			sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|GATEWAY_SECURITY, TCPCON_CLOSED_TIME, i, 0, 0, 0);
						};
      }

	    if(maxfd<Client[i].csd) maxfd=Client[i].csd;
	    }

			inset=watchset;

			if((temp=select(maxfd+1, &inset, NULL, NULL, &stv))<0) {
				perror("select");
				return 0;
				}
		if(temp==0) return 0;
																												 
	  for(i=0; i<MOXAGATE_CLIENTS_NUMBER; i++) 
			if (Client[i].status==GW_CLIENT_TCP_502)
			if (FD_ISSET(Client[i].csd, &inset)) {
	  	
			status = mbcom_tcp_recv(Client[i].csd,
															tcp_adu,
															&tcp_adu_len);

			Security.stat.accepted++;
			Client[i].stat.accepted++;
	
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

					Security.stat.errors++;
					Client[i].stat.errors++;
					func_res_err(tcp_adu[TCPADU_FUNCTION], &Security.stat);
					func_res_err(tcp_adu[TCPADU_FUNCTION], &Client[i].stat);
					stage_to_stat((MBCOM_REQ<<16) | (MBCOM_TCP_RECV<<8) | status, &Security.stat);
					stage_to_stat((MBCOM_REQ<<16) | (MBCOM_TCP_RECV<<8) | status, &Client[i].stat);

				 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_WRN|GATEWAY_SECURITY, POLL_TCP_RECV, (unsigned) status, i, 0, 0);

				  if(status==TCP_COM_ERR_NULL) {
						clear_client(i);
			 			// CONNECTION CLOSED (LINK DOWN)
			 			sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|GATEWAY_SECURITY, TCPCON_CLOSED_LINK, i, 0, 0, 0);
						}

					continue;
			  	break;

			  default:;
			  }

			time(&Client[i].last_activity_time);

			status=forward_query(i, tcp_adu, tcp_adu_len);

			if((status & FRWD_RESULT_MASK)!=FRWD_RESULT_OK ) { // запрос завершился ошибкой

				Security.stat.errors++;
				Client[i].stat.errors++;
				func_res_err(tcp_adu[TCPADU_FUNCTION], &Security.stat);
				func_res_err(tcp_adu[TCPADU_FUNCTION], &Client[i].stat);
				stage_to_stat((MBCOM_REQ<<16) | (MBCOM_FRWD_REQ<<8) | status, &Security.stat);
				stage_to_stat((MBCOM_REQ<<16) | (MBCOM_FRWD_REQ<<8) | status, &Client[i].stat);

				continue;
			  }
			  
			// запрос был перенаправлен

			switch(status&FRWD_TYPE_MASK) {

				case FRWD_TYPE_PROXY: 
					Security.stat.frwd_p++;
					Client[i].stat.frwd_p++;
					break;

				case FRWD_TYPE_REGISTER: 
					Security.stat.frwd_r++;
					Client[i].stat.frwd_r++;
					break;

				case FRWD_TYPE_ADDRESS:    
					Security.stat.frwd_a++;
					Client[i].stat.frwd_a++;
					break;

				default:;
				}

			func_res_ok(tcp_adu[TCPADU_FUNCTION], &Security.stat);
			//func_res_ok(tcp_adu[TCPADU_FUNCTION], &Client[i].stat); // запрос пошел дальше

	    }

  return 0;
  }
///-----------------------------------------------------------------------------------------------------------------
