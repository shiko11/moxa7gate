/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///*********************** МОДУЛЬ ИНТЕРФЕЙСОВ ШЛЮЗА ****************************
///*** МЕХАНИЗМ ОПРОСА MODBUS TCP MASTER

///=== INTERFACES_H MODULE IMPLEMENTATION

#include <string.h>
#include <stdio.h>

#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <unistd.h>

#include <fcntl.h>
#include <errno.h>

#include "interfaces.h"
#include "moxagate.h"
#include "messages.h"
#include "modbus.h"

///-----------------------------------------------------------------------------------------------------------------
void *iface_tcp_master(void *arg)
  {
	u8  req_adu[MB_UNIVERSAL_ADU_LEN];
	u16 req_adu_len;
	u8  rsp_adu[MB_UNIVERSAL_ADU_LEN];
	u16 rsp_adu_len;

  int port_id=((long)arg)>>8;
  int client_id, device_id;

	int status, rnum;
	unsigned i, j, Q;
	int connum, csd, active_connection=0, timeout_counter=0;
	int trans_id;

	GW_Iface	*tcp_master;

	struct timeval tv, tvchk, tv1, tv_wait;
	struct timezone tz;

	int queue_has_query;

  connum = port_id-GATEWAY_T01;
	tcp_master = &IfaceTCP[connum];
	
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

	while (1) for(i=0; i<=tcp_master->PQueryIndex[MAX_QUERY_ENTRIES]; i++) {

		gettimeofday(&tv, &tz);

		Q=tcp_master->PQueryIndex[i];
		j=i;

		status=get_query_from_queue(&tcp_master->queue, &client_id, &device_id, req_adu, &req_adu_len);

		if(status==1) { // внутренняя ошибка в программе
			tcp_master->stat.accepted++;
			tcp_master->stat.errors++;
			tcp_master->stat.frwd_queue_out++;
		  }

		queue_has_query=status==-1?0:1;
		if(queue_has_query==1) i=tcp_master->PQueryIndex[MAX_QUERY_ENTRIES];

		// с установленной периодичностью проверяем состояние сетевых соединений
    if( (	(tv.tv_sec -tvchk.tv_sec )*1000000 +
    			(tv.tv_usec-tvchk.tv_usec)         ) >= TCP_RECONN_INTVL) {
    	if(tcp_master->ethernet.status ==TCPMSTCON_ESTABLISHED)
		    check_tcpmaster_conn(tcp_master, 1);
    	if(tcp_master->ethernet.status2==TCPMSTCON_ESTABLISHED)
		    check_tcpmaster_conn(tcp_master, 2);
			gettimeofday(&tvchk, &tz);
			}

		// эта итерация цикла сканирования нужна для проверки очереди порта при пустой таблице опроса
		if( ((queue_has_query!=1) || (status==1)) &&
		     (i==tcp_master->PQueryIndex[MAX_QUERY_ENTRIES])) continue;

	if(queue_has_query!=1) {//формируем следующий запрос из таблицы опроса

		if(PQuery[Q].delay!=0) usleep(PQuery[Q].delay*1000);

    // если текущая операция - операция записи, то определяем парный интерфейс
    // и используем его мьютекс при формировании запроса для синхронизации с ним при доступе к общей памяти
    if(PQuery[Q].access == QT_ACCESS_WRITEONLY) { // MBF_WRITE_MULTIPLE_REGISTERS 
      connum = ((port_id-GATEWAY_T01)%2)==0 ? port_id-GATEWAY_T01+1: port_id-GATEWAY_T01-1;
      pthread_mutex_lock  (&IfaceTCP[connum].serial_mutex);
      }

    create_proxy_request(Q, req_adu, &req_adu_len);

    if(PQuery[Q].access == QT_ACCESS_WRITEONLY) { // MBF_WRITE_MULTIPLE_REGISTERS 
      pthread_mutex_unlock(&IfaceTCP[connum].serial_mutex);
      }


	    client_id=GW_CLIENT_MOXAGATE;

		tcp_master->Security.stat.accepted++;

		} else { //обрабатываем запрос из очереди последовательного порта

			i=j; // после обработки запроса из очереди будет обработан прерванный запрос из таблицы опроса

			prepare_request (device_id, req_adu, req_adu_len);

			}

	// устанавливаем актуальный адрес modbus tcp сервера
	req_adu[TCPADU_ADDRESS]=tcp_master->ethernet.unit_id;
	
	// сохраняем идентификатор транзакции
	trans_id = (req_adu[TCPADU_TRANS_HI]<<8) | req_adu[TCPADU_TRANS_LO];

///-----------------------------------
//		gettimeofday(&tv2, &tz);
//		tcp_master->clients[client_id].stat.scan_rate=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
//		if(tcp_master->clients[client_id].stat.scan_rate>MB_SCAN_RATE_INFINITE)
//		  tcp_master->clients[client_id].stat.scan_rate=MB_SCAN_RATE_INFINITE;
//	tcp_master->clients[client_id].stat.request_time_average=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
//	printf("cycle begins after %d msec\n", tcp_master->clients[client_id].stat.request_time_average);
//		gettimeofday(&tv1, &tz);

//	  pthread_mutex_unlock(&tcp_master->serial_mutex);
	  
///###-----------------------------
//			status = mb_tcp_make_adu(&OPC,oDATA,mb_tcp_adu);


		// процедура выбора tcp-соединения для связи с сервером:
		// если оба соединения установлены и активны, выбираем то из них,
		// которое было использовано для связи последним.
		
		connum=0;

		if(tcp_master->ethernet.status2==TCPMSTCON_ESTABLISHED) connum=2;
		if(tcp_master->ethernet.status ==TCPMSTCON_ESTABLISHED) connum=1;

		if((tcp_master->ethernet.status==TCPMSTCON_ESTABLISHED)&&(tcp_master->ethernet.status2==TCPMSTCON_ESTABLISHED))
			connum = active_connection==0? 1 : active_connection;

		active_connection = tcp_master->ethernet.active_connection = connum;

		switch(connum) {

			case 0: // если клиентское соединение не готово, завершаем итерацию
				//usleep(4000*1000);
				//reset_tcpmaster_conn(tcp_master);
				continue;
				break;

			case 1: csd=tcp_master->ethernet.csd ; break;
			case 2: csd=tcp_master->ethernet.csd2; break;
			
			default:;
			}

	  ///---------------------
		tcp_master->stat.accepted++;

		make_tcp_adu(req_adu, req_adu_len-TCPADU_ADDRESS);

		if(Security.show_data_flow==1)
			show_traffic(TRAFFIC_TCP_SEND, port_id, client_id, req_adu, req_adu_len);

//  gettimeofday(&tv1, NULL);
//  printf("send %lu; ", tv1.tv_sec*1000 + tv1.tv_usec/1000);

		status = mbcom_tcp_send(csd,
														req_adu,
														req_adu_len);

		switch(status) {
		  case 0:
		  	break;
		  case TCP_COM_ERR_SEND:

				tcp_master->stat.errors++;
				func_res_err(req_adu[TCPADU_FUNCTION], &tcp_master->stat);
				stage_to_stat((MBCOM_REQ<<16) | (MBCOM_TCP_SEND<<8) | status, &tcp_master->stat);

			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_WRN|port_id, POLL_TCP_SEND, (unsigned) status, client_id, 0, 0);

				shutdown(csd, SHUT_RDWR);
				close(csd);
				if(connum==1) tcp_master->ethernet.csd =-1;
				         else tcp_master->ethernet.csd2=-1;
				if(connum==1) tcp_master->ethernet.status =TCPMSTCON_NOTMADE;
				         else tcp_master->ethernet.status2=TCPMSTCON_NOTMADE;

				if(client_id==GW_CLIENT_MOXAGATE) {

					tcp_master->Security.stat.errors++;
					func_res_err(req_adu[TCPADU_FUNCTION], &tcp_master->Security.stat);
					stage_to_stat((MBCOM_REQ<<16) | (MBCOM_TCP_SEND<<8) | status, &tcp_master->Security.stat);

					PQuery[Q].status_bit=0;
          VSlave[device_id&0xff].status_bit=0;
				  } else if(client_id==GW_CLIENT_KM400) {

//            VSlave[device_id&0xff].err_counter++;
//            if(VSlave[device_id&0xff].err_counter >= VSlave[device_id&0xff].critical)
              VSlave[device_id&0xff].status_bit=0;

				  } else {
						Client[client_id].stat.errors++;
						func_res_err(req_adu[TCPADU_FUNCTION], &Client[client_id].stat);
						stage_to_stat((MBCOM_REQ<<16) | (MBCOM_TCP_SEND<<8) | status, &Client[client_id].stat);
				    }
				continue;
		  	break;

		  default:;
		  };

//  gettimeofday(&tv1, NULL);
//  printf("recv %lu; ", tv1.tv_sec*1000 + tv1.tv_usec/1000);

    rnum=0; do {


		status = mbcom_tcp_recv(csd,
														rsp_adu,
														&rsp_adu_len);
		
//  gettimeofday(&tv1, NULL);
//  printf("proc %lu; ms\n", tv1.tv_sec*1000 + tv1.tv_usec/1000);

		if(Security.show_data_flow==1)
			show_traffic(TRAFFIC_TCP_RECV, port_id, client_id, rsp_adu, rsp_adu_len);
//	gettimeofday(&tv2, &tz);
//	tcp_master->clients[client_id].stat.request_time_average=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
//	printf("tcp received after %d msec\n", tcp_master->clients[client_id].stat.request_time_average);

		switch(status) {
		  case 0:
		  	break;

		  case TCP_COM_ERR_NULL:
		  case TCP_COM_ERR_TIMEOUT:
		  	
		  	timeout_counter++;
		  	if((status==TCP_COM_ERR_NULL) || (timeout_counter>=16)) {
					shutdown(csd, SHUT_RDWR);
					close(csd);
					if(connum==1) tcp_master->ethernet.csd =-1;
					         else tcp_master->ethernet.csd2=-1;
					if(connum==1) tcp_master->ethernet.status =TCPMSTCON_NOTMADE;
					         else tcp_master->ethernet.status2=TCPMSTCON_NOTMADE;

					timeout_counter=0;

				  sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|GATEWAY_LANTCP, (status==TCP_COM_ERR_NULL?TCPCON_CLOSED_REMSD:TCPCON_CLOSED_TMOUT), 
				    (connum==1? tcp_master->ethernet.ip : tcp_master->ethernet.ip2), 0, 0, tcp_master->queue.port_id);

		  	  }
		  	
		  case TCP_ADU_ERR_MIN:
		  case TCP_ADU_ERR_MAX:
		  case TCP_ADU_ERR_PROTOCOL:
		  case TCP_ADU_ERR_LEN:
		  case TCP_ADU_ERR_UID:
		  case TCP_PDU_ERR:

				tcp_master->stat.errors++;
				func_res_err(rsp_adu[TCPADU_FUNCTION], &tcp_master->stat);
				stage_to_stat((MBCOM_RSP<<16) | (MBCOM_TCP_RECV<<8) | status, &tcp_master->stat);

			 	sysmsg_ex(EVENT_CAT_DEBUG|EVENT_TYPE_WRN|port_id, POLL_TCP_RECV, (unsigned) status, client_id, 0, 0);

				if(client_id==GW_CLIENT_MOXAGATE) {
					tcp_master->Security.stat.errors++;
					func_res_err(rsp_adu[TCPADU_FUNCTION], &tcp_master->Security.stat);
					stage_to_stat((MBCOM_RSP<<16) | (MBCOM_TCP_RECV<<8) | status, &tcp_master->Security.stat);

					PQuery[Q].status_bit=0;
          VSlave[device_id&0xff].status_bit=0;
				  } else if(client_id==GW_CLIENT_KM400) {

//            VSlave[device_id&0xff].err_counter++;
//            if(VSlave[device_id&0xff].err_counter >= VSlave[device_id&0xff].critical)
              VSlave[device_id&0xff].status_bit=0;

				  } else {
						Client[client_id].stat.errors++;
						func_res_err(rsp_adu[TCPADU_FUNCTION], &Client[client_id].stat);
						stage_to_stat((MBCOM_RSP<<16) | (MBCOM_TCP_RECV<<8) | status, &Client[client_id].stat);
				    }

				rnum=4; // continue;
		  	break;

		  default:;
		  };
		
		if(!((rsp_adu[TCPADU_TRANS_HI]==req_adu[TCPADU_TRANS_HI]) && // идентификатор транзакции в полученном ответе не соответствует запросу
				 (rsp_adu[TCPADU_TRANS_LO]==req_adu[TCPADU_TRANS_LO]) )) {
			rnum++;
			tv_wait.tv_sec = 2;
			tv_wait.tv_usec= 0;
			select(0, NULL, NULL, NULL, &tv_wait);
			} else rnum=5;

		} while(rnum<4);

	if(rnum==4) { // идентификатор транзакции в полученном ответе не соответствует запросу
		tcp_master->stat.errors++;
		continue;
	  }

  timeout_counter=0;

///###-----------------------------			

	// восстанавливаем идентификатор транзакции
	req_adu[TCPADU_TRANS_HI]=trans_id >> 8;
	req_adu[TCPADU_TRANS_LO]=trans_id & 0xff;

	rsp_adu_len-=TCPADU_ADDRESS; // получаем и сохраняем длину PDU+1

	if(client_id==GW_CLIENT_MOXAGATE) { // сохраняем локально полученные данные

    pthread_mutex_lock  (&tcp_master->serial_mutex);
    process_proxy_response(Q, rsp_adu, rsp_adu_len+MB_TCP_ADU_HEADER_LEN-1);
    pthread_mutex_unlock(&tcp_master->serial_mutex);

		tcp_master->stat.sended++;
		tcp_master->Security.stat.sended++;
		func_res_ok(rsp_adu[TCPADU_FUNCTION], &tcp_master->stat);
		func_res_ok(rsp_adu[TCPADU_FUNCTION], &tcp_master->Security.stat);

		} else if(client_id==GW_CLIENT_KM400) {

      // отдельный бит статуса связи для операции записи в КМ-400
      VSlave[device_id&0xff].status_bit=1;
      VSlave[device_id&0xff].err_counter=0;

		} else { //отправляем результат запроса непосредственно клиенту

			// функция преобразования ответа в соответствии с контекстом
			prepare_response(device_id, rsp_adu, rsp_adu_len);

			forward_response(port_id, client_id, req_adu, req_adu_len, rsp_adu, rsp_adu_len);
			}

	//tcp_master->stat.proc_time=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
		///!!! статистику обновляем централизованно в функции int refresh_shm(void *arg) или подобной
	//gate502.wData4x[gate502.status_info+3*port_id+2]=tcp_master->stat.request_time_average;

	}

	EndRun: ;
//	close(tcsd);
//	tcp_master->current_connections_number--;
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

#ifndef CYGWIN_KERNEL_231		
    /* Set the KEEPALIVE option active */
    optlen = sizeof(optval);
    if(setsockopt(csd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen)!=0) res++;

    optval = 2; // the number of unacknowledged probes to send before considering the connection dead and notifying the application layer
    if(setsockopt(csd, SOL_TCP, TCP_KEEPCNT, &optval, optlen)!=0) res++;

    optval = TCP_RECONN_INTVL/1000000; // the interval between the last data packet sent and the first keepalive probe
    if(setsockopt(csd, SOL_TCP, TCP_KEEPIDLE, &optval, optlen)!=0) res++;

    optval = TCP_RECONN_INTVL/1000000; // the interval between subsequential keepalive probes
    if(setsockopt(csd, SOL_TCP, TCP_KEEPINTVL, &optval, optlen)!=0) res++;
#endif

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
		res = connect(csd, (struct sockaddr *)&server, sizeof(server));
		if(res==-1 && errno!=EISCONN) {
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
