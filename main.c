/*
MOXA7GATE MODBUS GATEWAY SOFTWARE
SEM-ENGINEERING
                    BRYANSKK 2009
*/

#include <errno.h>
#include <fcntl.h>

#include <netinet/in.h>
#include <signal.h>

#include "modbus_rtu.h"
#include "modbus_tcp.h"
#include "global.h"


#define DEBUG_oDATA
#define DEBUG_iDATA

void *srvr_tcp_child(void *arg) //РТЙЕН - РЕТЕДБЮБ ДБООЩИ РП Modbus TCP
  {
	u8			tcp_adu[MB_TCP_MAX_ADU_LENGTH];// TCP ADU
	u16			tcp_adu_len;
	u8			serial_adu[MB_SERIAL_MAX_ADU_LEN];/// SERIAL ADU
	u16			serial_adu_len;

	int		status;

  int port_id=((unsigned)arg)>>8;
  int client_id=((unsigned)arg)&0xff;

  sysmsg(port_id|(client_id<<8), "GATEWAY mode Connection accepted", 0);

	int		tcsd = iDATA[port_id].clients[client_id].csd;
	GW_StaticData tmpstat;
	
	input_cfg	*inputDATA;
	inputDATA = &iDATA[port_id];
	int fd=inputDATA->serial.fd;
	
	struct timeval tv1, tv2;
	struct timezone tz;

	int i;
	inputDATA->clients[client_id].stat.request_time_min=10000; // 10 seconds, must be "this->serial.timeout"
	inputDATA->clients[client_id].stat.request_time_max=0;
	inputDATA->clients[client_id].stat.request_time_average=0;
	for(i=0; i<MAX_LATENCY_HISTORY_POINTS; i++) inputDATA->clients[client_id].stat.latency_history[i]=1000; //ms
	inputDATA->clients[client_id].stat.clp=0;

	while (1) {
		
		clear_stat(&tmpstat);

		if(_show_data_flow) printf("TCP%4.4d  IN: ", inputDATA->clients[client_id].port);
		status = mb_tcp_receive_adu(tcsd, &tmpstat, tcp_adu, &tcp_adu_len);
		gettimeofday(&tv2, &tz);
		inputDATA->clients[client_id].stat.scan_rate=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
		if(inputDATA->clients[client_id].stat.scan_rate>MB_SCAN_RATE_INFINITE)
		  inputDATA->clients[client_id].stat.scan_rate=MB_SCAN_RATE_INFINITE;
		gettimeofday(&tv1, &tz);
		tmpstat.accepted++;

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
				sprintf(eventmsg, "tcp receive error: %d", status);
  			sysmsg(port_id|(client_id<<8), eventmsg, 0);
				//update_stat(&inputDATA->clients[client_id].stat, &tmpstat);
				//update_stat(&iDATA[port_id].stat, &tmpstat);
				inputDATA->clients[client_id].connection_status=MB_CONNECTION_CLOSED;
				if(status==TCP_COM_ERR_NULL) goto EndRun;
				continue;
		  	break;
		  default:;
		  };
		
	pthread_mutex_lock(&inputDATA->serial_mutex);

		if(_show_data_flow) printf("PORT%d   OUT: ", port_id+1);
		status = mb_serial_send_adu(fd, &tmpstat, &tcp_adu[6], tcp_adu_len-6, serial_adu, &serial_adu_len);

		switch(status) {
		  case 0:
		  	break;
		  case MB_SERIAL_WRITE_ERR:
		  	tmpstat.errors++;
				sprintf(eventmsg, "serial send error: %d", status);
  			sysmsg(port_id|(client_id<<8), eventmsg, 0);
				update_stat(&inputDATA->clients[client_id].stat, &tmpstat);
				update_stat(&iDATA[port_id].stat, &tmpstat);
				continue;
		  	break;
		  default:;
		  };

	if(_show_data_flow) printf("PORT%d    IN: ", port_id+1);
    /// kazhetsya net zaschity ot perepolneniya bufera priema "serial_adu[]"
	  status = mb_serial_receive_adu(fd, &tmpstat, serial_adu, &serial_adu_len, &tcp_adu[MB_TCP_ADU_HEADER_LEN], inputDATA->serial.timeout, inputDATA->serial.ch_interval_timeout);

	  pthread_mutex_unlock(&inputDATA->serial_mutex);
	  
		switch(status) {
		  case 0:
		  	break;
		  case MB_SERIAL_READ_FAILURE:
		  case MB_SERIAL_COM_TIMEOUT:
		  case MB_SERIAL_ADU_ERR_MIN:
		  case MB_SERIAL_ADU_ERR_MAX:
		  case MB_SERIAL_CRC_ERROR:
		  case MB_SERIAL_PDU_ERR:
		  	tmpstat.errors++;
				sprintf(eventmsg, "serial receive error: %d", status);
  			sysmsg(port_id|(client_id<<8), eventmsg, 0);
				update_stat(&inputDATA->clients[client_id].stat, &tmpstat);
				update_stat(&iDATA[port_id].stat, &tmpstat);
				if(status==MB_SERIAL_READ_FAILURE) goto EndRun;
				continue;
		  	break;
		  default:;
		  };

///###-----------------------------			
//			status = mb_tcp_make_adu(&OPC,oDATA,mb_tcp_adu);

			if(_show_data_flow) printf("TCP%4.4d OUT: ", inputDATA->clients[client_id].port);
			status = mb_tcp_send_adu(tcsd, &tmpstat, serial_adu, serial_adu_len-2, tcp_adu, &tcp_adu_len);

		switch(status) {
		  case 0:
		  	tmpstat.sended++;
		  	break;
		  case TCP_COM_ERR_SEND:
		  	tmpstat.errors++;
				sprintf(eventmsg, "tcp send error: %d", status);
  			sysmsg(port_id|(client_id<<8), eventmsg, 0);
		  	break;
		  default:;
		  };

	update_stat(&inputDATA->clients[client_id].stat, &tmpstat);
	update_stat(&iDATA[port_id].stat, &tmpstat);

	gettimeofday(&tv2, &tz);

	inputDATA->clients[client_id].stat.request_time_average=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
	inputDATA->clients[client_id].stat.latency_history[inputDATA->clients[client_id].stat.clp]=inputDATA->clients[client_id].stat.request_time_average;
	inputDATA->clients[client_id].stat.clp=inputDATA->clients[client_id].stat.clp<MAX_LATENCY_HISTORY_POINTS?inputDATA->clients[client_id].stat.clp+1:0;

	if(inputDATA->clients[client_id].stat.request_time_min>inputDATA->clients[client_id].stat.request_time_average)
	  inputDATA->clients[client_id].stat.request_time_min=inputDATA->clients[client_id].stat.request_time_average;
	if(inputDATA->clients[client_id].stat.request_time_max<inputDATA->clients[client_id].stat.request_time_average)
	  inputDATA->clients[client_id].stat.request_time_max=inputDATA->clients[client_id].stat.request_time_average;

	inputDATA->clients[client_id].stat.request_time_average=0;
	for(i=0; i<MAX_LATENCY_HISTORY_POINTS; i++)
	  inputDATA->clients[client_id].stat.request_time_average+=inputDATA->clients[client_id].stat.latency_history[i];
	inputDATA->clients[client_id].stat.request_time_average/=MAX_LATENCY_HISTORY_POINTS;
	
//	printf("%d\n", inputDATA->stat.request_time_average);
//	printf("%d:%d\n", tv1.tv_sec, tv1.tv_usec);
//	printf("%d:%d\n", tv2.tv_sec, tv2.tv_usec);
	}
	EndRun: ;
	close(tcsd);
	inputDATA->current_connections_number--;
	inputDATA->clients[client_id].rc=1;
	pthread_exit (0);	
}

///-----------------------------------------------------------------------------------------------------------------
void *srvr_tcp_bridge(void *arg) //РТЙЕН - РЕТЕДБЮБ ДБООЩИ РП Modbus TCP
  {

	u8			tcp_adu[MB_TCP_MAX_ADU_LENGTH];// TCP ADU
	u16			tcp_adu_len;
	u8			serial_adu[MB_SERIAL_MAX_ADU_LEN];/// SERIAL ADU
	u16			serial_adu_len;
	u8			exception_adu[MB_SERIAL_MAX_ADU_LEN];/// SERIAL ADU
	u16			exception_adu_len;

	int		status;

  int port_id=((unsigned)arg)>>8;
  int client_id=((unsigned)arg)&0xff;

//  printf("port %d cliet %d\n", port_id, client_id);
	GW_StaticData tmpstat;
	
	input_cfg	*inputDATA;
	inputDATA = &iDATA[port_id];
	int fd=inputDATA->serial.fd;
	
	struct timeval tv1, tv2;
	struct timezone tz;

	int i;
	inputDATA->clients[client_id].stat.request_time_min=10000; // 10 seconds, must be "this->serial.timeout"
	inputDATA->clients[client_id].stat.request_time_max=0;
	inputDATA->clients[client_id].stat.request_time_average=0;
	for(i=0; i<MAX_LATENCY_HISTORY_POINTS; i++) inputDATA->clients[client_id].stat.latency_history[i]=1000; //ms
	inputDATA->clients[client_id].stat.clp=0;

	for(i=0; i<iDATA[port_id].accepted_connections_number; i++)
		bridge_reset_tcp(&iDATA[port_id], i);

///-----------------------------------------

	while (1) {
		
		//if(inputDATA->clients[0].connection_status!=MB_CONNECTION_ESTABLISHED) pthread_exit (0);
		
		clear_stat(&tmpstat);
		
		exception_adu_len=0;

///-----------------------------------
	  if(_show_data_flow) printf("PORT%d    IN: ", port_id+1);
    /// kazhetsya net zaschity ot perepolneniya bufera priema "serial_adu[]"
	  status = serial_receive_adu(fd, &tmpstat, serial_adu, &serial_adu_len, &tcp_adu[MB_TCP_ADU_HEADER_LEN], inputDATA->serial.timeout, inputDATA->serial.ch_interval_timeout);

		gettimeofday(&tv2, &tz);
		inputDATA->clients[client_id].stat.scan_rate=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
		if(inputDATA->clients[client_id].stat.scan_rate>MB_SCAN_RATE_INFINITE)
		  inputDATA->clients[client_id].stat.scan_rate=MB_SCAN_RATE_INFINITE;
//	inputDATA->clients[client_id].stat.request_time_average=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
//	printf("cycle begins after %d msec\n", inputDATA->clients[client_id].stat.request_time_average);
		gettimeofday(&tv1, &tz);

//	  pthread_mutex_unlock(&inputDATA->serial_mutex);
	  
		tmpstat.accepted++;

		switch(status) {
		  case 0:
		  	break;
		  case MB_SERIAL_READ_FAILURE:
		  case MB_SERIAL_COM_TIMEOUT:
		  case MB_SERIAL_ADU_ERR_MIN:
		  case MB_SERIAL_ADU_ERR_MAX:
		  case MB_SERIAL_CRC_ERROR:
		  case MB_SERIAL_PDU_ERR:
		  	tmpstat.errors++;
				sprintf(eventmsg, "serial receive error: %d", status);
  			sysmsg(port_id|(client_id<<8), eventmsg, 0);
				update_stat(&inputDATA->clients[client_id].stat, &tmpstat);
				update_stat(&iDATA[port_id].stat, &tmpstat);
				iDATA[port_id].modbus_mode=MODBUS_PORT_ERROR;
				if(status==MB_SERIAL_READ_FAILURE) goto EndRun;
		  	break;
		  default:;
		  };

///###-----------------------------			
//			status = mb_tcp_make_adu(&OPC,oDATA,mb_tcp_adu);

    tcp_adu[0]=0x03; ///!!!
    tcp_adu[1]=0x03; 
    tcp_adu[2]=0x00;
    tcp_adu[3]=0x00; 
    tcp_adu[4]=0x00;

    for(i=0; i<inputDATA->accepted_connections_number; i++)
      if(inputDATA->clients[i].mb_slave==serial_adu[0]) {
      	client_id=i;
      	break;
        }
//    printf("i=%d, total=%d\n", i, inputDATA->accepted_connections_number);

    if(i==inputDATA->accepted_connections_number) {
  		tmpstat.errors_serial_adu++;
	  	tmpstat.errors++;
			update_stat(&inputDATA->clients[client_id].stat, &tmpstat);
			update_stat(&iDATA[port_id].stat, &tmpstat);
			exception_adu_len=9;
			tcp_adu[7]=serial_adu[1]|0x80;
			tcp_adu[8]=0x0a;
      } else if(iDATA[port_id].clients[client_id].connection_status!=MB_CONNECTION_ESTABLISHED) {
				exception_adu_len=9;
				tcp_adu[7]=serial_adu[1]|0x80;
				tcp_adu[8]=0x0b;
				bridge_reset_tcp(&iDATA[port_id], client_id);
			  }

    tcp_adu[6]=serial_adu[0]; /// MODBUS DEVICE UID
    
		if(exception_adu_len==0) {
			
	  if(inputDATA->clients[client_id].address_shift!=MB_ADDRESS_NO_SHIFT) {
	  	status=(serial_adu[2]<<8)|serial_adu[3];
	  	status+=inputDATA->clients[client_id].address_shift;
	  	serial_adu[2]=(status>>8)&0xff;
	  	serial_adu[3]=status&0xff;
	  	}

		if(_show_data_flow) printf("TCP%4.4d OUT: ", inputDATA->clients[client_id].port);
		status = mb_tcp_send_adu(inputDATA->clients[client_id].csd, &tmpstat, serial_adu, serial_adu_len-2, tcp_adu, &tcp_adu_len);

//	gettimeofday(&tv2, &tz);
//	inputDATA->clients[client_id].stat.request_time_average=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
//	printf("tcp sended after %d msec\n", inputDATA->clients[client_id].stat.request_time_average);

		switch(status) {
		  case 0:
		  	break;
		  case TCP_COM_ERR_SEND:
		  	tmpstat.errors++;
				sprintf(eventmsg, "tcp send error: %d", status);
  			sysmsg(port_id|(client_id<<8), eventmsg, 0);
				//iDATA[port_id].modbus_mode=MODBUS_PORT_ERROR;
				shutdown(inputDATA->clients[client_id].csd, SHUT_RDWR);
				close(inputDATA->clients[client_id].csd);
				inputDATA->clients[client_id].csd=-1;
				iDATA[port_id].clients[client_id].connection_status=MB_CONNECTION_ERROR;
				iDATA[port_id].current_connections_number--;
				continue;
		  	break;
		  default:;
		  };

		if(_show_data_flow) printf("TCP%4.4d  IN: ", inputDATA->clients[client_id].port);
		status = mb_tcp_receive_adu(inputDATA->clients[client_id].csd, &tmpstat, tcp_adu, &tcp_adu_len);

//	gettimeofday(&tv2, &tz);
//	inputDATA->clients[client_id].stat.request_time_average=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
//	printf("tcp received after %d msec\n", inputDATA->clients[client_id].stat.request_time_average);

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
		  	tmpstat.errors++;
				sprintf(eventmsg, "tcp receive error: %d", status);
  			sysmsg(port_id|(client_id<<8), eventmsg, 0);
				update_stat(&inputDATA->clients[client_id].stat, &tmpstat);
				update_stat(&iDATA[port_id].stat, &tmpstat);
				//iDATA[port_id].modbus_mode=MODBUS_PORT_ERROR;
				//if(status==TCP_COM_ERR_NULL) goto EndRun;
				//bridge_reset_tcp(&iDATA[port_id]);
				continue;
		  	break;
		  default:;
		  };
		
		}
//	pthread_mutex_lock(&inputDATA->serial_mutex);

/*  пример того, как делать не надо
	  if(	(inputDATA->clients[client_id].address_shift!=MB_ADDRESS_NO_SHIFT)&&
	  		((tcp_adu[6+1]&0x80)==0)	) {
	  	status=(tcp_adu[6+2]<<8)|tcp_adu[6+3];
	  	status-=inputDATA->clients[client_id].address_shift;
	  	tcp_adu[6+2]=(status>>8)&0xff;
	  	tcp_adu[6+3]=status&0xff;
	  	}*/

		if(_show_data_flow) printf("PORT%d   OUT: ", port_id+1);
		status = mb_serial_send_adu(fd, &tmpstat, &tcp_adu[6], tcp_adu_len-6, serial_adu, &serial_adu_len);

		switch(status) {
		  case 0:
		  	if(exception_adu_len==0) tmpstat.sended++;
		  	  else {
			  		tmpstat.errors_serial_adu++;
				  	tmpstat.errors++;
		  	    }
		  	break;
		  case MB_SERIAL_WRITE_ERR:
		  	tmpstat.errors++;
				sprintf(eventmsg, "serial send error: %d", status);
  			sysmsg(port_id|(client_id<<8), eventmsg, 0);
				update_stat(&inputDATA->clients[client_id].stat, &tmpstat);
				update_stat(&iDATA[port_id].stat, &tmpstat);
				continue;
		  	break;
		  default:;
		  };

	update_stat(&inputDATA->clients[client_id].stat, &tmpstat);
	update_stat(&iDATA[port_id].stat, &tmpstat);

	gettimeofday(&tv2, &tz);

	inputDATA->clients[client_id].stat.request_time_average=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
	inputDATA->clients[client_id].stat.latency_history[inputDATA->clients[client_id].stat.clp]=inputDATA->clients[client_id].stat.request_time_average;
	inputDATA->clients[client_id].stat.clp=inputDATA->clients[client_id].stat.clp<MAX_LATENCY_HISTORY_POINTS?inputDATA->clients[client_id].stat.clp+1:0;

	if(inputDATA->clients[client_id].stat.request_time_min>inputDATA->clients[client_id].stat.request_time_average)
	  inputDATA->clients[client_id].stat.request_time_min=inputDATA->clients[client_id].stat.request_time_average;
	if(inputDATA->clients[client_id].stat.request_time_max<inputDATA->clients[client_id].stat.request_time_average)
	  inputDATA->clients[client_id].stat.request_time_max=inputDATA->clients[client_id].stat.request_time_average;

	inputDATA->clients[client_id].stat.request_time_average=0;
	for(i=0; i<MAX_LATENCY_HISTORY_POINTS; i++)
	  inputDATA->clients[client_id].stat.request_time_average+=inputDATA->clients[client_id].stat.latency_history[i];
	inputDATA->clients[client_id].stat.request_time_average/=MAX_LATENCY_HISTORY_POINTS;
	
//	printf("%d\n", inputDATA->stat.request_time_average);
//	printf("%d:%d\n", tv1.tv_sec, tv1.tv_usec);
//	printf("%d:%d\n", tv2.tv_sec, tv2.tv_usec);
//	printf("noop\n");
//  usleep(100000);

//	gettimeofday(&tv2, &tz);
//	inputDATA->clients[client_id].stat.request_time_average=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;
//	printf("cycle ended after %d msec\n", inputDATA->clients[client_id].stat.request_time_average);
	}

	EndRun: ;
//	close(tcsd);
//	inputDATA->current_connections_number--;
	inputDATA->clients[client_id].rc=1;
	
	pthread_exit (0);	
}
///----------------------------------------------------------------------------------------------------------------
void update_stat(GW_StaticData *dst, GW_StaticData *src)
	{
	dst->accepted+=src->accepted;
	dst->errors_input_communication+=src->errors_input_communication;
	dst->errors_tcp_adu+=src->errors_tcp_adu;
	dst->errors_tcp_pdu+=src->errors_tcp_pdu;
	dst->errors_serial_sending+=src->errors_serial_sending;
	dst->errors_serial_accepting+=src->errors_serial_accepting;
	dst->timeouts+=src->timeouts;
	dst->crc_errors+=src->crc_errors;
	dst->errors_serial_adu+=src->errors_serial_adu;
	dst->errors_serial_pdu+=src->errors_serial_pdu;
	dst->errors_tcp_sending+=src->errors_tcp_sending;
	dst->errors+=src->errors;
	dst->sended+=src->sended;

	return;
	}
	
void clear_stat(GW_StaticData *dst)
	{
	dst->accepted=\
	dst->errors_input_communication=\
	dst->errors_tcp_adu=\
	dst->errors_tcp_pdu=\
	dst->errors_serial_sending=\
	dst->errors_serial_accepting=\
	dst->timeouts=\
	dst->crc_errors=\
	dst->errors_serial_adu=\
	dst->errors_serial_pdu=\
	dst->errors_tcp_sending=\
	dst->errors=\
	dst->sended=0;

	return;
	}
	
int bridge_reset_tcp(input_cfg *bridge, int client)
	{
	if (bridge->clients[client].csd < 0)
	  bridge->clients[client].csd = socket(AF_INET, SOCK_STREAM, 0);
	if (bridge->clients[client].csd < 0) {
		perror("bridge socket");
		//bridge->modbus_mode=MODBUS_PORT_ERROR;
    //strcpy(bridge->bridge_status, "ERR");
		return 1;
		} // else printf("\nPORT2 BRIDGE #%d socket OK...\n", client);

	struct sockaddr_in server;
	//struct hostent *h;
	//h = gethostbyname ("server.domain.ru");
	//memcpy ((char *)kserver.sin_addr,h->h_addr,h->h_length);
	// Определяем семейство протоколов
	server.sin_family = AF_INET;
	// определяем IP-адрес сервера
	//server.sin_addr.s_addr = 0x0a0006f0; // 10.0.0.240
	server.sin_addr.s_addr = bridge->clients[client].ip;
  //	server.sin_addr.s_addr = 0xc00000fc; // 192.0.0.252
	// Определяем порт сервера
	server.sin_port =  htons(bridge->clients[client].port);

	struct timeval tvs, tvr;
	int optlen=sizeof(tvs);
	tvs.tv_sec=0; tvs.tv_usec=500000;
	tvr.tv_sec=0; tvr.tv_usec=500000;
	if(
	(setsockopt(bridge->clients[client].csd, SOL_SOCKET, SO_SNDTIMEO, &tvs, optlen)!=0)||
	(setsockopt(bridge->clients[client].csd, SOL_SOCKET, SO_RCVTIMEO, &tvr, optlen)!=0)) 
	sysmsg((bridge->serial.p_num-1)|(client<<8), "setting socket timeout error", 0);
//	printf("for client%d send_timeout=%dms, receive_timeout=%dms\n", client, tvs.tv_sec*1000+tvs.tv_usec/1000, tvr.tv_sec*1000+tvr.tv_usec/1000);

	// Вызов функции connect()
	if(connect(bridge->clients[client].csd, (struct sockaddr *)&server, sizeof(server))==-1) {
		perror("bridge tcp connection");																			
		sprintf(eventmsg, "connect failed %d.%d.%d.%d", 
													bridge->clients[client].ip>>24,
													(bridge->clients[client].ip>>16)&0xff,
													(bridge->clients[client].ip>>8)&0xff,
													bridge->clients[client].ip&0xff);
		
		sysmsg((bridge->serial.p_num-1)|(client<<8), eventmsg, 0);
		return 2;
		} else sysmsg((bridge->serial.p_num-1)|(client<<8), "connection established", 0);


	bridge->clients[client].connection_status=MB_CONNECTION_ESTABLISHED;
  bridge->current_connections_number++;
	time(&bridge->clients[client].connection_time);
	return 0;
	}

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

void sysmsg(int source, char *message, int show_anyway)
{
int i;
char string[EVENT_MESSAGE_LENGTH];
//if(strlen(string)>=EVENT_MESSAGE_LENGTH) string[EVENT_MESSAGE_LENGTH]=0;
for(i=0; i<EVENT_MESSAGE_LENGTH-1; i++)
  string[i]=message[i];
string[EVENT_MESSAGE_LENGTH-1]=0;

if(_show_sys_messages==1 || show_anyway==1)
  if((source&0xff)<8) {
    if(((source>>8)&0xff)<8)
		  printf("PORT%d.CLIENT%d: %s\n", (source&0xff)+1, ((source>>8)&0xff)+1, string);
		  else printf("PORT%d         : %s\n", (source&0xff)+1, string);
	  } else printf(" SYSTEM #%4.4X: %s\n", ((source>>8)&0xff)+1, string);

///!!! добавление записи в журнал сообщений
//if(strlen(string)>=EVENT_MESSAGE_LENGTH) return;
	
return;
}

int main(int argc, char *argv[])
{
	int				res_cl;
//	u16				tcp_port;
	memset(iDATA,0,sizeof(iDATA)); // must be standart constructor for each data structure
//	memset(oDATA,0,sizeof(oDATA));	// zdes' ispol'zuem dinamicheskoe vydelenit pamyati

	int			i, j;
	for(i=0; i<MAX_MOXA_PORTS; i++) {

		for(j=0; j<MAX_TCP_CLIENTS_PER_PORT; j++) {
			iDATA[i].clients[j].rc=1;
			iDATA[i].clients[j].csd=-1;
			iDATA[i].clients[j].connection_status=MB_CONNECTION_CLOSED;
			iDATA[i].clients[j].mb_slave=MB_SLAVE_NOT_DEFINED;
			iDATA[i].clients[j].address_shift=MB_ADDRESS_NO_SHIFT;
		  }

	  iDATA[i].accepted_connections_number=0;
	  iDATA[i].current_connections_number=0;
	  iDATA[i].rejected_connections_number=0;

		iDATA[i].ssd=-1;
		iDATA[i].modbus_mode=MODBUS_PORT_OFF;
		strcpy(iDATA[i].bridge_status, "OFF");
		sprintf(iDATA[i].serial.p_name, "/dev/ttyM%d", i);

		iDATA[i].serial.p_num=i+1;
		strcpy(iDATA[i].serial.p_mode, "RS485_2w");
		strcpy(iDATA[i].serial.speed, "9600");
		strcpy(iDATA[i].serial.parity, "none");
		iDATA[i].serial.timeout=2000000;
		iDATA[i].tcp_port=1000*i+502;
		
    iDATA[i].current_client=0;

		p_errors[i]=0; // this value for buzzer function
		iDATA[i].start_time=0;
		}

	res_cl = get_command_line (argc, argv,
								 						 iDATA,
														 &_show_data_flow,
														 &_show_sys_messages);
	switch(res_cl) {
		case CL_ERR_NONE_PARAM:
			printf("Command line parsing error: No Configuration Parameters\n");
			exit(1);
		case CL_INFO:
			exit(1);
		case CL_ERR_PORT_WORD:
			printf("Command line parsing error: Keyword PORT not found\n");
			exit(1);
		case CL_ERR_IN_STRUCT:
			printf("Command line parsing error: Invalid parameters amount for some PORTs\n");
			exit(1);
		case CL_ERR_IN_PORT_SETT:
			printf("Command line parsing error: Wrong Serial Port Configuration Parameter(s)\n");
			exit(1);
		case CL_ERR_GATEWAY_MODE:
			printf("Command line parsing error: Wrong GATEWAY mode keyword\n");
			exit(1);
		case CL_ERR_IN_MAP:
			printf("Command line parsing error: Wrong MAP data\n");
			exit(1);
		case CL_ERR_MIN_PARAM:
			printf("Command line parsing error: Too low Serial Port Configuration Parameters\n");
			exit(1);
		case CL_OK:
			printf("Command line parsed successfully\n");
		default:;
		}

//printf("\n\nCOMMAND LINE TESTING\n");
//exit (1);
//-------------------------------------------------------	
	mxkpd_handle=keypad_open();
	if (mxkpd_handle < 0 ) sysmsg(EVENT_SOURCE_SYSTEM, "Keypad init ERROR", 1);
	  else sysmsg(EVENT_SOURCE_SYSTEM, "Keypad init OK", 1);

	mxlcm_handle = mxlcm_open();
	if (mxlcm_handle < 0 ) sysmsg(EVENT_SOURCE_SYSTEM, "LCM init ERROR", 1);
	  else sysmsg(EVENT_SOURCE_SYSTEM, "LCM init OK", 1);
  mxlcm_control(mxlcm_handle, IOCTL_LCM_AUTO_SCROLL_OFF);

	mxbzr_handle = mxbuzzer_open();
	if (mxbzr_handle < 0 ) sysmsg(EVENT_SOURCE_SYSTEM, "Buzzer init ERROR", 1);
	  else sysmsg(EVENT_SOURCE_SYSTEM, "Buzzer init OK", 1);

	screen.current_screen=LCM_SCREEN_MAIN;

  screen.main_scr_mode=1;
  screen.menu_scr_mode=1;
  screen.secr_scr_mode=1;
  screen.back_light=1;
  screen.max_tcp_clients_per_com=8;
  screen.watch_dog_control=0;
  screen.buzzer_control=1;
  screen.secr_scr_changes_was_made=0;

	gettimeofday(&tv_mem, &tz);
//	printf("tv_mem %d\n", tv_mem.tv_sec);

	init_shm();
	signal(SIGPIPE, sigpipe_handler);
	signal(SIGIO, sigio_handler);

	int			rc;
	pthread_t		tstTH;
	rc = pthread_create(
		&tstTH,
		NULL,
		mx_keypad_lcm,
		NULL);
	//-------------------------------------------------------
	struct sockaddr_in	addr;
	int csd, arg, P;
	int ports[MAX_MOXA_PORTS];
	for(j=0; j<MAX_MOXA_PORTS; j++) ports[j]=0;

	for(i=0; i<MAX_MOXA_PORTS; i++) {
		
		P=0xff;
		for(j=0; j<MAX_MOXA_PORTS; j++)
		  if((iDATA[j].modbus_mode==MODBUS_GATEWAY_MODE)&&(ports[j]==0)) {
		  	ports[j]=1; P=j; break;
		    }
		if(P==0xff) for(j=0; j<MAX_MOXA_PORTS; j++)
		  if((iDATA[j].modbus_mode==MODBUS_BRIDGE_MODE)&&(ports[j]==0)) {
		  	ports[j]=1; P=j; break;
		    }
		if(P==0xff) continue;
		
		// opening serial ports
		if(iDATA[P].modbus_mode!=MODBUS_PORT_OFF) {
	    iDATA[P].serial.fd = open_comm(iDATA[P].serial.p_name, iDATA[P].serial.p_mode);
	    
			iDATA[P].serial.ch_interval_timeout = set_param_comms(iDATA[P].serial.fd, iDATA[P].serial.speed, iDATA[P].serial.parity);
		//	printf("speed %s\n", inputDATA->serial.speed);
		//	printf("parity %s\n", inputDATA->serial.parity);
	    
	    if(0) {iDATA[P].modbus_mode=MODBUS_PORT_OFF; strcpy(iDATA[P].bridge_status, "ERR"); continue;} ///!!!
	    sprintf(eventmsg, "Serial port P%d init OK; mode %d", P+1, iDATA[P].modbus_mode);
			sysmsg(P, eventmsg, 1);
			} else continue;

		switch(iDATA[P].modbus_mode) {
			case MODBUS_GATEWAY_MODE:
				
	    	sprintf(eventmsg, "Server socket init for P%d ", P+1);
				iDATA[P].ssd = socket(AF_INET, SOCK_STREAM, 0);
				if (iDATA[P].ssd < 0) {
					perror("csdet");
					iDATA[P].modbus_mode=MODBUS_PORT_ERROR;
			    strcpy(iDATA[P].bridge_status, "ERR");
			    strcat(eventmsg, "ERROR");
			    sysmsg(P, eventmsg, 1);
			    break;
					}
		    strcat(eventmsg, "OK");
		    sysmsg(P, eventmsg, 1);
				
				addr.sin_family = AF_INET;
				addr.sin_port = htons(iDATA[P].tcp_port);
				addr.sin_addr.s_addr = htonl(INADDR_ANY);
				//РТЙЧСЪЩЧБЕН УПЛЕФ 
				sprintf(eventmsg, "Bind Socket for P%d ", P+1);
				if (bind(iDATA[P].ssd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
					perror("bind");
					//status
					//exit(1);
					close(iDATA[P].ssd);
					iDATA[P].ssd=-1;
					iDATA[P].modbus_mode=MODBUS_PORT_ERROR;
			    strcpy(iDATA[P].bridge_status, "ERR");
			    strcat(eventmsg, "ERROR");
			    sysmsg(P, eventmsg, 1);
			    break;
					}
		    strcat(eventmsg, "OK");
		    sysmsg(P, eventmsg, 1);

				//УФБЧЙН Ч ПЮЕТЕДШ
				listen(iDATA[P].ssd, MAX_TCP_CLIENTS_PER_PORT);
				
				fcntl(iDATA[P].ssd, F_SETFL, fcntl(iDATA[P].ssd, F_GETFL, 0) | O_NONBLOCK);
				pthread_mutex_init(&iDATA[P].serial_mutex, NULL);
			  strcpy(iDATA[P].bridge_status, "00G");
		
				break;

			/// инициализация BRIDGE-соединений по ЛВС выполняется ПОСЛЕ инициализации
			/// прослушивающих сокетов GATEWAY-портов
			case MODBUS_BRIDGE_MODE:

				//инициализацию сетевых соединений порта в режиме BRIDGE проиводим в потоке порта

				arg=(P<<8)|(iDATA[P].current_client&0xff);
				//printf("arg:%d\n", arg);
				iDATA[P].clients[0].rc = pthread_create(
					&iDATA[P].clients[0].tid_srvr,
					NULL,
					srvr_tcp_bridge,
					(void *) arg);
				
				if (iDATA[P].clients[0].rc){
					sprintf(eventmsg, "pthread_create() ERROR %d", iDATA[P].clients[0].rc);
			    sysmsg(P, eventmsg, 1);
					iDATA[P].modbus_mode=MODBUS_PORT_ERROR;
		      strcpy(iDATA[P].bridge_status, "ERR");
				  } else strcpy(iDATA[P].bridge_status, "00B");
			  break;

			case MODBUS_MASTER_MODE: ///!!!
			default: iDATA[P].modbus_mode=MODBUS_PORT_OFF;
			}
		
		time(&iDATA[P].start_time);
		}
	
	while (1) {
		//
	  for(i=0; i<MAX_MOXA_PORTS; i++) {
	  	
		  if(iDATA[i].modbus_mode!=MODBUS_GATEWAY_MODE) continue;
	  	
		  for(j=0; j<MAX_TCP_CLIENTS_PER_PORT; j++)
		    if(iDATA[i].clients[j].rc) break;
		  iDATA[i].current_client=j;

		 rc=sizeof(addr);
		 if(iDATA[i].ssd>=0) csd = accept(iDATA[i].ssd, (struct sockaddr *)&addr, &rc);
		   else continue;
			
			if((csd<0)&&(errno==EAGAIN)) {
				//printf("noop\n");
				continue;
			  }
			  
			if (csd < 0) {
				perror("accept");
				close(iDATA[i].ssd);
				iDATA[i].ssd=-1;
				iDATA[i].modbus_mode=MODBUS_PORT_ERROR;
	      strcpy(iDATA[i].bridge_status, "ERR");
			}

	   if(iDATA[i].current_client==MAX_TCP_CLIENTS_PER_PORT) {
			 iDATA[i].rejected_connections_number++;
			 
			 sprintf(eventmsg, "Conn rejected %d.%d.%d.%d",
			 															 addr.sin_addr.s_addr >> 24,
																		 (addr.sin_addr.s_addr >> 16) & 0xff,
																		 (addr.sin_addr.s_addr >> 8) & 0xff,
																		 addr.sin_addr.s_addr & 0xff);

	     sysmsg(i, eventmsg, 0);
			 
			 close(csd);
	   	 continue;
	     }
	   iDATA[i].clients[iDATA[i].current_client].csd=csd;
	  	
			//printf("accepted %d connection for port %d\n", iDATA[i].current_client+1, i+1);
			
			int arg=(i<<8)|(iDATA[i].current_client&0xff);
			iDATA[i].clients[iDATA[i].current_client].rc = pthread_create(
				&iDATA[i].clients[iDATA[i].current_client].tid_srvr,
				NULL,
				srvr_tcp_child,
				(void *) arg);
			
			time(&iDATA[i].clients[iDATA[i].current_client].connection_time);
			iDATA[i].clients[iDATA[i].current_client].ip=addr.sin_addr.s_addr;
			iDATA[i].clients[iDATA[i].current_client].port=addr.sin_port;
			iDATA[i].clients[iDATA[i].current_client].connection_status=MB_CONNECTION_ESTABLISHED;
			clear_stat(&iDATA[i].clients[iDATA[i].current_client].stat);
			//printf("ip%X\n", addr.sin_addr.s_addr);
			
			if (iDATA[i].clients[iDATA[i].current_client].rc){
				close(iDATA[i].ssd);
				iDATA[i].ssd=-1;
				iDATA[i].modbus_mode=MODBUS_PORT_ERROR;
	      strcpy(iDATA[i].bridge_status, "ERR");
				sprintf(eventmsg, "pthread_create() ERROR %d", iDATA[i].clients[iDATA[i].current_client].rc);
		    sysmsg(i, eventmsg, 0);
			  }
		  iDATA[i].accepted_connections_number++;
		  iDATA[i].current_connections_number++;
	    }
	  usleep(750000); // proverit' zaderzhku na obrabotku vseh klientov v rezhime MASTER
	  }
	//pthread_join(csd, NULL);
	//------------------------------------------------------
	for(i=0; i<MAX_MOXA_PORTS; i++)
	  if(iDATA[i].ssd>=0) {
			shutdown(iDATA[i].ssd, SHUT_RDWR);
	  	close(iDATA[i].ssd);
	  	}

  mxlcm_close(mxlcm_handle);
  keypad_close(mxkpd_handle);
  mxbuzzer_close(mxbzr_handle);

	close_shm();

	sysmsg(EVENT_SOURCE_SYSTEM, "Program stopped", 1);

	return (0);
}
///-----------------------------------------------------------------------------------------------------------------
