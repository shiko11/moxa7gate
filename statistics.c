/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///*************  ÌÎÄÓËÜ ÑÁÎĞÀ ÑÒÀÒÈÑÒÈÊÈ ÎÏĞÎÑÀ *******************************

///=== STATISTICS_H IMPLEMENTATION

#include "modbus.h"
#include "statistics.h"
#include "forwarding.h"

//*************************************************************************************

int init_statistics_h()
  {

  return 0;
  }

void copy_stat(GW_StaticData *dst, GW_StaticData *src)
	{
	dst->tcp_req_recv =	src->tcp_req_recv;
	dst->tcp_req_adu =	src->tcp_req_adu;
	dst->tcp_req_pdu =	src->tcp_req_pdu;
	dst->rtu_req_recv =	src->rtu_req_recv;
	dst->rtu_req_crc =	src->rtu_req_crc;
	dst->rtu_req_adu =	src->rtu_req_adu;
	dst->rtu_req_pdu =	src->rtu_req_pdu;

	dst->frwd_proxy =	src->frwd_proxy;
	dst->frwd_atm =	src->frwd_atm;
	dst->frwd_rtm =	src->frwd_rtm;
	dst->frwd_queue_in =	src->frwd_queue_in;
	dst->frwd_queue_out =	src->frwd_queue_out;

	dst->rtu_req_send =	src->rtu_req_send;
	dst->rtu_rsp_recv =	src->rtu_rsp_recv;
	dst->rtu_rsp_timeout =	src->rtu_rsp_timeout;
	dst->rtu_rsp_crc =	src->rtu_rsp_crc;
	dst->rtu_rsp_adu =	src->rtu_rsp_adu;
	dst->rtu_rsp_pdu =	src->rtu_rsp_pdu;

	dst->tcp_req_send =	src->tcp_req_send;
	dst->tcp_rsp_recv =	src->tcp_rsp_recv;
	dst->tcp_rsp_timeout =	src->tcp_rsp_timeout;
	dst->tcp_rsp_adu =	src->tcp_rsp_adu;
	dst->tcp_rsp_pdu =	src->tcp_rsp_pdu;

	dst->frwd_rsp =	src->frwd_rsp;
	dst->tcp_rsp_send =	src->tcp_rsp_send;
	dst->rtu_rsp_send =	src->rtu_rsp_send;

	int i;
	for(i=0; i<STAT_FUNC_AMOUNT; i++) {
		dst->func[i][STAT_RES_OK]= 	src->func[i][STAT_RES_OK];
		dst->func[i][STAT_RES_ERR]=	src->func[i][STAT_RES_ERR];
		dst->func[i][STAT_RES_EXP]=	src->func[i][STAT_RES_EXP];
		}

	dst->proc_time =	    src->proc_time;
	dst->proc_time_min =	src->proc_time_min;
	dst->proc_time_max =	src->proc_time_max;
	
	dst->poll_time =	    src->poll_time;
	dst->poll_time_min =	src->poll_time_min;
	dst->poll_time_max =	src->poll_time_max;

	return;
	}
/*	
void update_stat(GW_StaticData *dst, GW_StaticData *src)
	{
	dst->tcp_req_recv +=	src->tcp_req_recv;
	dst->tcp_req_adu +=	src->tcp_req_adu;
	dst->tcp_req_pdu +=	src->tcp_req_pdu;
	dst->rtu_req_recv +=	src->rtu_req_recv;
	dst->rtu_req_crc +=	src->rtu_req_crc;
	dst->rtu_req_adu +=	src->rtu_req_adu;
	dst->rtu_req_pdu +=	src->rtu_req_pdu;

	dst->frwd_proxy +=	src->frwd_proxy;
	dst->frwd_atm +=	src->frwd_atm;
	dst->frwd_rtm +=	src->frwd_rtm;
	dst->frwd_queue_in +=	src->frwd_queue_in;
	dst->frwd_queue_out +=	src->frwd_queue_out;

	dst->rtu_req_send +=	src->rtu_req_send;
	dst->rtu_rsp_recv +=	src->rtu_rsp_recv;
	dst->rtu_rsp_timeout +=	src->rtu_rsp_timeout;
	dst->rtu_rsp_crc +=	src->rtu_rsp_crc;
	dst->rtu_rsp_adu +=	src->rtu_rsp_adu;
	dst->rtu_rsp_pdu +=	src->rtu_rsp_pdu;

	dst->tcp_req_send +=	src->tcp_req_send;
	dst->tcp_rsp_recv +=	src->tcp_rsp_recv;
	dst->tcp_rsp_timeout +=	src->tcp_rsp_timeout;
	dst->tcp_rsp_adu +=	src->tcp_rsp_adu;
	dst->tcp_rsp_pdu +=	src->tcp_rsp_pdu;

	dst->frwd_rsp +=	src->frwd_rsp;
	dst->tcp_rsp_send +=	src->tcp_rsp_send;
	dst->rtu_rsp_send +=	src->rtu_rsp_send;

	int i;
	for(i=0; i<STAT_FUNC_AMOUNT; i++) {
		dst->func[i][STAT_RES_OK]+= 	src->func[i][STAT_RES_OK];
		dst->func[i][STAT_RES_ERR]+=	src->func[i][STAT_RES_ERR];
		dst->func[i][STAT_RES_EXP]+=	src->func[i][STAT_RES_EXP];
		}

	return;
	}
*/
void clear_stat(GW_StaticData *dst)
	{
	dst->tcp_req_recv = \
	dst->tcp_req_adu = \
	dst->tcp_req_pdu = \
	dst->rtu_req_recv = \
	dst->rtu_req_crc = \
	dst->rtu_req_adu = \
	dst->rtu_req_pdu = \

	dst->frwd_proxy = \
	dst->frwd_atm = \
	dst->frwd_rtm = \
	dst->frwd_queue_in = \
	dst->frwd_queue_out = \

	dst->rtu_req_send = \
	dst->rtu_rsp_recv = \
	dst->rtu_rsp_timeout = \
	dst->rtu_rsp_crc = \
	dst->rtu_rsp_adu = \
	dst->rtu_rsp_pdu = \

	dst->tcp_req_send = \
	dst->tcp_rsp_recv = \
	dst->tcp_rsp_timeout = \
	dst->tcp_rsp_adu = \
	dst->tcp_rsp_pdu = \

	dst->frwd_rsp = \
	dst->tcp_rsp_send = \
	dst->rtu_rsp_send = 0;

	memset(dst->func, 0, sizeof(dst->func));

	dst->proc_time =	\
	dst->proc_time_min =	\
	dst->proc_time_max =	0;
	
	dst->poll_time =	\
	dst->poll_time_min =	\
	dst->poll_time_max =	0;

  return;
	}

void func_res_ok(int mbf, GW_StaticData *dst)
	{

	static int res;
	res=(mbf&0x80)==0?STAT_RES_OK:STAT_RES_EXP;

	switch(mbf & 0x7f) {
		case MBF_READ_COILS:							dst->func[STAT_FUNC_0x01][res]++; break;
		case MBF_READ_DECRETE_INPUTS:			dst->func[STAT_FUNC_0x02][res]++; break;
		case MBF_READ_HOLDING_REGISTERS:	dst->func[STAT_FUNC_0x03][res]++; break;
		case MBF_READ_INPUT_REGISTERS:		dst->func[STAT_FUNC_0x04][res]++; break;

		case MBF_WRITE_SINGLE_COIL:					dst->func[STAT_FUNC_0x05][res]++; break;
		case MBF_WRITE_SINGLE_REGISTER:			dst->func[STAT_FUNC_0x06][res]++; break;
		case MBF_WRITE_MULTIPLE_COILS:			dst->func[STAT_FUNC_0x0f][res]++; break;
		case MBF_WRITE_MULTIPLE_REGISTERS:	dst->func[STAT_FUNC_0x10][res]++; break;

		default: dst->func[STAT_FUNC_OTHER][res]++;
		}

	return;
	}

void func_res_err(int mbf, GW_StaticData *dst)
	{

	static int res;
	res=(mbf&0x80)==0?STAT_RES_ERR:STAT_RES_EXP;

	switch(mbf & 0x7f) {
		case MBF_READ_COILS:							dst->func[STAT_FUNC_0x01][res]++; break;
		case MBF_READ_DECRETE_INPUTS:			dst->func[STAT_FUNC_0x02][res]++; break;
		case MBF_READ_HOLDING_REGISTERS:	dst->func[STAT_FUNC_0x03][res]++; break;
		case MBF_READ_INPUT_REGISTERS:		dst->func[STAT_FUNC_0x04][res]++; break;

		case MBF_WRITE_SINGLE_COIL:					dst->func[STAT_FUNC_0x05][res]++; break;
		case MBF_WRITE_SINGLE_REGISTER:			dst->func[STAT_FUNC_0x06][res]++; break;
		case MBF_WRITE_MULTIPLE_COILS:			dst->func[STAT_FUNC_0x0f][res]++; break;
		case MBF_WRITE_MULTIPLE_REGISTERS:	dst->func[STAT_FUNC_0x10][res]++; break;

		default: dst->func[STAT_FUNC_OTHER][res]++;
		}

	return;
	}

///-----------------------------------------------------------------------------------------------------------------
void stage_to_stat(int mbcom, GW_StaticData *dst)
	{

	if((mbcom >> 16)==MBCOM_REQ) {

		switch((mbcom >> 8)&0xff) {
			case MBCOM_RTU_RECV:
				if((mbcom&MBCOM_STAGE_MASK)==MBCOM_RECV) dst->rtu_req_recv++;
				if((mbcom&MBCOM_STAGE_MASK)==MBCOM_CRC ) dst->rtu_req_crc++;
				if((mbcom&MBCOM_STAGE_MASK)==MBCOM_ADU ) dst->rtu_req_adu++;
				if((mbcom&MBCOM_STAGE_MASK)==MBCOM_PDU ) dst->rtu_req_pdu++;
				break;

			case MBCOM_RTU_SEND: dst->rtu_req_send++; break;

			case MBCOM_TCP_RECV:
				if((mbcom&MBCOM_STAGE_MASK)==MBCOM_RECV) dst->tcp_req_recv++;
				if((mbcom&MBCOM_STAGE_MASK)==MBCOM_ADU ) dst->tcp_req_adu++;
				if((mbcom&MBCOM_STAGE_MASK)==MBCOM_PDU ) dst->tcp_req_pdu++;
				break;

			case MBCOM_TCP_SEND: dst->tcp_req_send++; break;

			case MBCOM_FRWD_REQ:
				if((mbcom&MBCOM_STAGE_MASK)==FRWD_TYPE_PROXY   ) dst->frwd_proxy++;
				if((mbcom&MBCOM_STAGE_MASK)==FRWD_TYPE_ADDRESS ) dst->frwd_atm++;
				if((mbcom&MBCOM_STAGE_MASK)==FRWD_TYPE_REGISTER) dst->frwd_rtm++;
				if((mbcom&MBCOM_STAGE_MASK)==FRWD_RESULT_QUEUE_FAIL) dst->frwd_queue_in++;
				break;

			case MBCOM_QUEUE_OUT: dst->frwd_queue_out++; break;

//			case MBCOM_FRWD_RSP:

			default:;
		  }

	} else if((mbcom >> 16)==MBCOM_RSP) {

		switch((mbcom >> 8)&0xff) {
			case MBCOM_RTU_RECV:
				if((mbcom&MBCOM_STAGE_MASK)==MBCOM_RECV) dst->rtu_rsp_recv++;
				if((mbcom&MBCOM_STAGE_MASK)==MBCOM_TIMEOUT) dst->rtu_rsp_timeout++;
				if((mbcom&MBCOM_STAGE_MASK)==MBCOM_CRC ) dst->rtu_rsp_crc++;
				if((mbcom&MBCOM_STAGE_MASK)==MBCOM_ADU ) dst->rtu_rsp_adu++;
				if((mbcom&MBCOM_STAGE_MASK)==MBCOM_PDU ) dst->rtu_rsp_pdu++;
				break;

			case MBCOM_RTU_SEND: dst->rtu_rsp_send++; break;

			case MBCOM_TCP_RECV:
				if((mbcom&MBCOM_STAGE_MASK)==MBCOM_RECV) dst->tcp_rsp_recv++;
				if((mbcom&MBCOM_STAGE_MASK)==MBCOM_TIMEOUT) dst->tcp_rsp_timeout++;
				if((mbcom&MBCOM_STAGE_MASK)==MBCOM_ADU ) dst->tcp_rsp_adu++;
				if((mbcom&MBCOM_STAGE_MASK)==MBCOM_PDU ) dst->tcp_rsp_pdu++;
				break;

			case MBCOM_TCP_SEND: dst->tcp_rsp_send++; break;

//			case MBCOM_FRWD_REQ:
//			case MBCOM_QUEUE_OUT:

			case MBCOM_FRWD_RSP: dst->frwd_rsp++; break;

			default:;
		  }

	}

	return;
	}

///-----------------------------------------------------------------------------------------------------------------
