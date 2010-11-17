/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///*************  ÌÎÄÓËÜ ÑÁÎÐÀ ÑÒÀÒÈÑÒÈÊÈ ÎÏÐÎÑÀ *******************************

///=== STATISTICS_H IMPLEMENTATION

#include "modbus.h"
#include "statistics.h"

//*************************************************************************************

void copy_stat(GW_StaticData *dst, GW_StaticData *src)
	{
	dst->accepted=src->accepted;
	dst->errors_input_communication=src->errors_input_communication;
	dst->errors_tcp_adu=src->errors_tcp_adu;
	dst->errors_tcp_pdu=src->errors_tcp_pdu;
	dst->errors_serial_sending=src->errors_serial_sending;
	dst->errors_serial_accepting=src->errors_serial_accepting;
	dst->timeouts=src->timeouts;
	dst->crc_errors=src->crc_errors;
	dst->errors_serial_adu=src->errors_serial_adu;
	dst->errors_serial_pdu=src->errors_serial_pdu;
	dst->errors_tcp_sending=src->errors_tcp_sending;
	dst->errors=src->errors;
	dst->sended=src->sended;

	int i;
	for(i=0; i<STAT_FUNC_AMOUNT; i++) {
		dst->func[i][STAT_RES_OK]= 	src->func[i][STAT_RES_OK];
		dst->func[i][STAT_RES_ERR]=	src->func[i][STAT_RES_ERR];
		dst->func[i][STAT_RES_EXP]=	src->func[i][STAT_RES_EXP];
		}

	return;
	}
	
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

	int i;
	for(i=0; i<STAT_FUNC_AMOUNT; i++) {
		dst->func[i][STAT_RES_OK]+= 	src->func[i][STAT_RES_OK];
		dst->func[i][STAT_RES_ERR]+=	src->func[i][STAT_RES_ERR];
		dst->func[i][STAT_RES_EXP]+=	src->func[i][STAT_RES_EXP];
		}

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

	memset(dst->func, 0, sizeof(dst->func));

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
