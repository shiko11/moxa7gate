/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///*********************** МОДУЛЬ ИНТЕРФЕЙСОВ ШЛЮЗА ****************************
///*** МЕХАНИЗМ ОПРОСА MODBUS TCP MASTER

///=== INTERFACES_H MODULE IMPLEMENTATION

#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

#include <fcntl.h>
#include <errno.h>

#include <stdio.h>
#include <string.h>

#include "interfaces.h"
#include "moxagate.h"
#include "messages.h"
#include "modbus.h"
#include "tsd.h"

int do_data_exchange(GW_Iface *tcp_master, int port_id, int *timeout_counter,
	                   u8 *req_adu, u16  req_adu_len,
	                   u8 *rsp_adu, u16 *rsp_adu_len);

///-----------------------------------------------------------------------------------------------------------------
void *iface_tcp_master(void *arg)
  {
	u8  req_adu[MB_UNIVERSAL_ADU_LEN];
	u16 req_adu_len;
	u8  rsp_adu[MB_UNIVERSAL_ADU_LEN];
	u16 rsp_adu_len;

  int port_id=((unsigned)arg)>>8;
  int client_id, device_id;

	int status;
	unsigned i, j, Q;
	int connum, csd, active_connection=0, timeout_counter=0;
	int trans_id;

	GW_Iface	*tcp_master;
  HB_Header rcvhdr, lclhdr;

	struct timeval tv, tvchk, tv1, tv_wait;
	struct timezone tz;

	int queue_has_query;
	
	int first_scan=1;
  int eip_size_reg, buf_size_eip, max_read_eip, start_addr, length_reg;

  connum = port_id-GATEWAY_T01;
	tcp_master = &IfaceTCP[connum];
	
  memset(&rcvhdr, 0, sizeof(HB_Header));
  memset(&lclhdr, 0, sizeof(HB_Header));

  status=init_tsd_h(port_id); // инициализация конфигурационного массива параметров истории
  if(status!=0) printf("init_tsd_h error %d\n", status);
//  printf("metric %s\n", TSD_Param[connum][0].metric);
//  printf("adc_max %d\n", TSD_Param[connum][0].adc_max);
//  printf("eng_max %f\n", TSD_Param[connum][0].eng_max);

	//tcp_master->clients[client_id].stat.request_time_min=10000; // 10 seconds, must be "this->serial.timeout"
	//tcp_master->clients[client_id].stat.request_time_max=0;
	//tcp_master->clients[client_id].stat.request_time_average=0;
	//for(i=0; i<MAX_LATENCY_HISTORY_POINTS; i++) tcp_master->clients[client_id].stat.latency_history[i]=1000; //ms
	//tcp_master->clients[client_id].stat.clp=0;

	/// semaphore
	tcp_master->queue.operations[0].sem_op=-1;
	semop(semaphore_id, tcp_master->queue.operations, 1);
	tcp_master->queue.operations[0].sem_flg=IPC_NOWAIT;

  // THREAD STARTED
	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|GATEWAY_LANTCP, IFACE_THREAD_STARTED, port_id, client_id, 0, port_id);

	//reset_tcpmaster_conn(tcp_master, 1);

		gettimeofday(&tvchk, &tz);

///-----------------------------------------
	// в таблице опроса для данного порта д.б. ровно одна строка
	if(tcp_master->PQueryIndex[MAX_QUERY_ENTRIES]==1 && status==0) {
		
		Q=tcp_master->PQueryIndex[0];

		//create_proxy_request(Q, req_adu, &req_adu_len);
//  req_adu[TCPADU_ADDRESS]=	 PQuery[Q].device;
		req_adu[TCPADU_ADDRESS]=   tcp_master->ethernet.unit_id;
		req_adu[TCPADU_FUNCTION]=	 PQuery[Q].mbf;

		req_adu_len=12;

		client_id=GW_CLIENT_MOXAGATE;

	while (1) {

		gettimeofday(&tv, &tz);

		// status=get_query_from_queue(&tcp_master->queue, &client_id, &device_id, req_adu, &req_adu_len);

		// с установленной периодичностью проверяем состояние сетевых соединений
    if( (	(tv.tv_sec -tvchk.tv_sec )*1000000 +
    			(tv.tv_usec-tvchk.tv_usec)         ) >= TCP_RECONN_INTVL) {
    	if(tcp_master->ethernet.status ==TCPMSTCON_ESTABLISHED)
		    check_tcpmaster_conn(tcp_master, 1);
    	if(tcp_master->ethernet.status2==TCPMSTCON_ESTABLISHED)
		    check_tcpmaster_conn(tcp_master, 2);
			gettimeofday(&tvchk, &tz);
			}

    // выполняется выдержка времени для обеспечения заданной периодичности при опросе
		if(PQuery[Q].delay!=0) {
			// usleep(PQuery[Q].delay*1000);
			tv_wait.tv_sec  = PQuery[Q].delay / 1000;
			tv_wait.tv_usec =(PQuery[Q].delay % 1000) * 1000;
			select(0, NULL, NULL, NULL, &tv_wait);
		  }

		// проверка готовности tcp-соединения для связи с контроллером
		connum = tcp_master->ethernet.status==TCPMSTCON_ESTABLISHED ? 1:0;
		active_connection = tcp_master->ethernet.active_connection = connum;
		if(connum==0) continue;

	  // получение заголовка буфера истории

		req_adu[TCPADU_START_HI]=	(PQuery[Q].start  >>8)&0xff;
		req_adu[TCPADU_START_LO]=  PQuery[Q].start      &0xff;
		req_adu[TCPADU_LEN_HI]=		(HAGENT_HEADER_LEN>>8)&0xff;
		req_adu[TCPADU_LEN_LO]= 	 HAGENT_HEADER_LEN    &0xff;
																																
		status = do_data_exchange(tcp_master, port_id, &timeout_counter,
	                            req_adu,  req_adu_len,
	                            rsp_adu, &rsp_adu_len);

		if(status!=0) continue; // если обмен данными с контроллером не был выполнен, переход к следующей итерации
	                   
///---------------------

	rsp_adu_len-=TCPADU_ADDRESS; // получаем и сохраняем длину PDU+1
	if(client_id==GW_CLIENT_MOXAGATE) { // сохраняем локально полученные данные
		process_proxy_response(Q, rsp_adu, rsp_adu_len+MB_TCP_ADU_HEADER_LEN-1);
		}

  // получение значений из заголовка буфера истории
  status=check_header(rsp_adu, &rcvhdr);
	if(status!=0) {tcp_master->stat.frwd_p++; continue;}

  printf("\nhdr%d, min%d, hour%d, day%d, month%d, year%d, prmnum%d, msprec%d", 
          rcvhdr.header, rcvhdr.min, rcvhdr.hour, rcvhdr.day, rcvhdr.month, rcvhdr.year,
          rcvhdr.prmnum, rcvhdr.msprec);

  continue; /// отладка. завершение итерации

  if((rcvhdr.prmnum != lclhdr.prmnum) ||
     (rcvhdr.msprec != lclhdr.msprec)  ) first_scan=1;
											
  if(first_scan==1) { // инициализация переменных
    first_scan=0;
    copy_header(&rcvhdr, &lclhdr);    
    }

  // определение общих характеристик буфера истории
  eip_size_reg = lclhdr.prmnum + 1;                                     // размер блока ЕИП в регистрах
  buf_size_eip = (PQuery[Q].length - HAGENT_HEADER_LEN) / eip_size_reg; // размер буфера истории в блоках ЕИП
  max_read_eip = MBF_0x03_MAX_QUANTITY / eip_size_reg;                  // максимальный размер блока данных для
                                                                        // чтения в блоках ЕИП

  if(rcvhdr.header != lclhdr.header) { // в буфере истории появились новые данные

    if(rcvhdr.header < lclhdr.header) { // если достигнут конец буфера истории
      rcvhdr.header = buf_size_eip;			// данные вычитываются двумя запросами
      }

    // за один раз вычитывается блок данных размером не больше допустимого
    if((rcvhdr.header - lclhdr.header) > max_read_eip) {
    	rcvhdr.header = lclhdr.header + max_read_eip;
      }																						

    start_addr = PQuery[Q].start + HAGENT_HEADER_LEN + lclhdr.header * eip_size_reg;
    length_reg = (rcvhdr.header - lclhdr.header) * eip_size_reg;
      
    // формирование запроса на чтение данных
		req_adu[TCPADU_START_HI]=	(start_addr>>8)&0xff;
		req_adu[TCPADU_START_LO]=  start_addr    &0xff;
		req_adu[TCPADU_LEN_HI]=		(length_reg>>8)&0xff;
		req_adu[TCPADU_LEN_LO]= 	 length_reg    &0xff;
																																
		status = do_data_exchange(tcp_master, port_id, &timeout_counter,
	                            req_adu,  req_adu_len,
	                            rsp_adu, &rsp_adu_len);

		if(status!=0) continue; // если обмен данными с контроллером не был выполнен, переход к следующей итерации

    // полученные данные копируются в приемный буфер
  
    // завершение текущей итерации
    copy_header(&rcvhdr, &lclhdr);

    }

	//tcp_master->stat.proc_time=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
		///!!! статистику обновляем централизованно в функции int refresh_shm(void *arg) или подобной
	//gate502.wData4x[gate502.status_info+3*port_id+2]=tcp_master->stat.request_time_average;

	}
	}

	EndRun: ;

	tcp_master->rc=1;
	// THREAD STOPPED
 	sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|GATEWAY_LANTCP, IFACE_THREAD_STOPPED, port_id, client_id, 0, 0);
	pthread_exit (0);	
}

///-----------------------------------------------------------------------------------------------------------------
int reset_tcpmaster_conn(GW_Iface *tcp_master, int connum)
	{

	int csd, status;
	
	struct sockaddr_in server;
	
	struct timeval tv;
	int optlen=sizeof(tv);

	int res=0;

  int optval = 1;
  
	csd=    connum==1? tcp_master->ethernet.csd :    tcp_master->ethernet.csd2;
	status= connum==1? tcp_master->ethernet.status : tcp_master->ethernet.status2;
	
	// Определяем семейство протоколов
	server.sin_family = AF_INET;

	// определяем IP-адрес сервера
	//server.sin_addr.s_addr = 0x0a00006f; // 10.0.0.240
  //server.sin_addr.s_addr = 0xc00000fc; // 192.0.0.252
	server.sin_addr.s_addr = connum==1? htonl(tcp_master->ethernet.ip) : htonl(tcp_master->ethernet.ip2);

	// Определяем порт сервера
	server.sin_port =  connum==1? htons(tcp_master->ethernet.port) : htons(tcp_master->ethernet.port2);

	if(status==TCPMSTCON_NOTMADE) {

		csd = socket(AF_INET, SOCK_STREAM, 0);
	
		if(csd < 0) {
			perror("tcp_master socket");
			return 1;
			}
	
		tv.tv_sec = tcp_master->ethernet.timeout / 1000000;
		tv.tv_usec= tcp_master->ethernet.timeout % 1000000;
		
		// устанавливаем значение таймаута на операции записи и чтения для сокета
		if(setsockopt(csd, SOL_SOCKET, SO_SNDTIMEO, &tv, optlen)!=0) res++;
		if(setsockopt(csd, SOL_SOCKET, SO_RCVTIMEO, &tv, optlen)!=0) res++;
		
    /* Set the KEEPALIVE option active */
    optlen = sizeof(optval);
    if(setsockopt(csd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen)!=0) res++;

    optval = 2; // the number of unacknowledged probes to send before considering the connection dead and notifying the application layer
    if(setsockopt(csd, SOL_TCP, TCP_KEEPCNT, &optval, optlen)!=0) res++;

    optval = TCP_RECONN_INTVL/1000000; // the interval between the last data packet sent and the first keepalive probe
    if(setsockopt(csd, SOL_TCP, TCP_KEEPIDLE, &optval, optlen)!=0) res++;

    optval = TCP_RECONN_INTVL/1000000; // the interval between subsequential keepalive probes
    if(setsockopt(csd, SOL_TCP, TCP_KEEPINTVL, &optval, optlen)!=0) res++;

		if(res!=0) { // SOCKET NOT INITIALIZED
			sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_LANTCP, TCPCON_INITIALIZED, 0, 0, 0, tcp_master->queue.port_id);
			return 3;
		  }

		// для обеспечения немедленного возврата из функции connect()
		fcntl(csd, F_SETFL, fcntl(csd, F_GETFL, 0) | O_NONBLOCK);

		status=TCPMSTCON_INPROGRESS;
		if(connum==1) {
			tcp_master->ethernet.csd=csd;
			tcp_master->ethernet.status=TCPMSTCON_INPROGRESS;
			} else {
			tcp_master->ethernet.csd2=csd;
			tcp_master->ethernet.status2=TCPMSTCON_INPROGRESS;
			}
	  }

	if (status==TCPMSTCON_INPROGRESS) {
		if(connect(csd, (struct sockaddr *)&server, sizeof(server))==-1) {
			// perror("perror: ");
			// CONNECTION FAILED
		 	// sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|GATEWAY_LANTCP, TCPCON_FAILED, server.sin_addr.s_addr, 0, 0, tcp_master->queue.port_id);
			return 2;
			} 

		// CONNECTION ESTABLISHED
		sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_INF|GATEWAY_LANTCP, TCPCON_ESTABLISHED, server.sin_addr.s_addr, 0, 0, tcp_master->queue.port_id);
	
		// для обеспечения нормальной работы потоковой функции TCP-интерфейса
		fcntl(csd, F_SETFL, fcntl(csd, F_GETFL, 0) & (~O_NONBLOCK));

		if(connum==1) {
			tcp_master->ethernet.status=TCPMSTCON_ESTABLISHED;
			time(&tcp_master->ethernet.connection_time);
			} else {
			tcp_master->ethernet.status2=TCPMSTCON_ESTABLISHED;
			time(&tcp_master->ethernet.connection_time2);
			}
    }

	return 0;
	}
///-----------------------------------------------------------------------------------------------------------------
int check_tcpmaster_conn(GW_Iface *tcp_master, int connum)
	{
	int csd, res, buf;

	csd = connum==1? tcp_master->ethernet.csd : tcp_master->ethernet.csd2;
	
	res = recv(csd, &buf, 1, MSG_PEEK | MSG_DONTWAIT);

  // возможны следующие ситуации при вызове функции recv в нашей ситуации:
  // соединение установлено, данные есть:	res > 0
  // таймаут при попытке чтения:					res==-1, errno==EAGAIN (11)
  // соединение разорвано:								res==-1, errno==ENOTCONN (107)
  // соединение закрыто сервером:					res==0

	if( (res==0) || ((res==-1)&&(errno!=EAGAIN)) ) {
		shutdown(csd, SHUT_RDWR);
		close(csd);
		if(connum==1) {
			tcp_master->ethernet.csd=-1;
			tcp_master->ethernet.status=TCPMSTCON_NOTMADE;
			} else {
				tcp_master->ethernet.csd2=-1;
				tcp_master->ethernet.status2=TCPMSTCON_NOTMADE;
				}
	  sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|GATEWAY_LANTCP, (res==0?TCPCON_CLOSED_REMSD:TCPCON_CLOSED_KPALV), 
	    (connum==1? tcp_master->ethernet.ip : tcp_master->ethernet.ip2), 0, 0, tcp_master->queue.port_id);

		return TCPMSTCON_NOTMADE;
	  }
	
	return TCPMSTCON_ESTABLISHED;
	}
///-----------------------------------------------------------------------------------------------------------------
int do_data_exchange(GW_Iface *tcp_master, int port_id, int *timeout_counter,
	                   u8 *req_adu, u16  req_adu_len,
	                   u8 *rsp_adu, u16 *rsp_adu_len)
  {
	int status;
	u16 adu_len;
  	
		if(tcp_master->ethernet.status != TCPMSTCON_ESTABLISHED) return 1;

		tcp_master->stat.accepted++;

		make_tcp_adu(req_adu, req_adu_len-TCPADU_ADDRESS);

		if(Security.show_data_flow==1)
			show_traffic(TRAFFIC_TCP_SEND, port_id, GW_CLIENT_MOXAGATE, req_adu, req_adu_len);

		status = mbcom_tcp_send(tcp_master->ethernet.csd, req_adu, req_adu_len);

		switch(status) {
		  case 0:
		  	break;
		  case TCP_COM_ERR_SEND:

				tcp_master->stat.errors++;

			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_WRN|port_id, POLL_TCP_SEND, (unsigned) status, GW_CLIENT_MOXAGATE, 0, 0);

				shutdown(tcp_master->ethernet.csd, SHUT_RDWR);
				close(tcp_master->ethernet.csd);
				tcp_master->ethernet.csd =-1;
				tcp_master->ethernet.status =TCPMSTCON_NOTMADE;

				//if(client_id==GW_CLIENT_MOXAGATE) {}
				//PQuery[Q].status_bit=0;

				return 2;
		  	break;

		  default:;
		  };

		status = mbcom_tcp_recv(tcp_master->ethernet.csd, rsp_adu, &adu_len);
		*rsp_adu_len=adu_len;
		
		if(Security.show_data_flow==1)
			show_traffic(TRAFFIC_TCP_RECV, port_id, GW_CLIENT_MOXAGATE, rsp_adu, *rsp_adu_len);

		switch(status) {
		  case 0:
		  	break;

		  case TCP_COM_ERR_NULL:
		  case TCP_COM_ERR_TIMEOUT:
		  	
		  	*timeout_counter++;
		  	if((status==TCP_COM_ERR_NULL) || (*timeout_counter>=4)) {
					shutdown(tcp_master->ethernet.csd, SHUT_RDWR);
					close(tcp_master->ethernet.csd);
					tcp_master->ethernet.csd =-1;
					tcp_master->ethernet.status =TCPMSTCON_NOTMADE;

					*timeout_counter=0;

				  sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|GATEWAY_LANTCP, (status==TCP_COM_ERR_NULL?TCPCON_CLOSED_REMSD:TCPCON_CLOSED_TMOUT), 
				    tcp_master->ethernet.ip, 0, 0, 0);

		  	  }
		  	
		  case TCP_ADU_ERR_MIN:
		  case TCP_ADU_ERR_MAX:
		  case TCP_ADU_ERR_PROTOCOL:
		  case TCP_ADU_ERR_LEN:
		  case TCP_ADU_ERR_UID:
		  case TCP_PDU_ERR:

				tcp_master->stat.errors++;

			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_WRN|port_id, POLL_TCP_RECV, (unsigned) status, GW_CLIENT_MOXAGATE, 0, 0);

				return 3;
		  	break;

		  default:;
		  };
			 
	if((rsp_adu[TCPADU_FUNCTION]&0x80)>0) { // получено исключение
		tcp_master->stat.errors++;
		return 4;
	  }
	tcp_master->stat.sended++;

  return 0;
	}
///-----------------------------------------------------------------------------------------------------------------
