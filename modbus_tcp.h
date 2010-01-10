/*
MOXA7GATE MODBUS GATEWAY SOFTWARE
SEM-ENGINEERING
                    BRYANSK 2009
*/

#include "modbus_rtu.h"
#include "global.h"

#ifndef MODBUS_TCP_H
#define MODBUS_TCP_H



//*** TCP Recv ERR
#define	TCP_COM_ERR_NULL					-1
#define	TCP_ADU_ERR_MIN						-2
#define	TCP_ADU_ERR_MAX						-3
#define	TCP_ADU_ERR_PROTOCOL			-4
#define	TCP_ADU_ERR_LEN						-5
#define	TCP_ADU_ERR_UID						-6
#define	TCP_PDU_ERR								-7

#define	TCP_COM_ERR_SEND					-8

//UID - ы статусных устройств
#define	DATA_DEV_UID						1
#define	OFFSET_STATUS_DEV					10
#define	UID_STATUS_DEV_0					0 
#define	UID_STATUS_DEV_1					1 
#define	UID_STATUS_DEV_2					2 
#define	UID_STATUS_DEV_3					3 
#define	UID_STATUS_DEV_4					4 
#define	UID_STATUS_DEV_5					5 
#define	UID_STATUS_DEV_6					6 
#define	UID_STATUS_DEV_7					7



/*###typedef struct {
	struct {
		u16		Tid;
		u16		Pid;
		u16		Length;
		u8			Uid;
	} mbhap;
	struct {
		u8		slave;
		u8		function;
		u16		start;
		u16		count;
	} pdu;
} opc;*/

int		mb_tcp_receive_adu(int sfd, GW_StaticData *stat, u8 *mb_received_adu, u16 *adu_len);
//int mb_tcp_send_adu(int sfd, input_cfg *context, u8 *pdu, u16 pdu_len, u8 *rq_adu);
//int mb_tcp_send_adu(int sfd, input_cfg *context, u8 *pdu, u16 pdu_len);
int mb_tcp_send_adu(int sfd, GW_StaticData *stat, u8 *pdu, u16 pdu_len, u8 *request, u16 *tcp_len);

//int		mb_tcp_analis_adu(opc *OPC,u8 *adu);
//int		mb_tcp_make_adu(opc *OPC,u16 *oDATA,u8 *mb_received_adu);
//int		mb_tcp_make_status_adu(opc *OPC,u8 *mb_received_adu);




#endif  /* MODBUS_TCP_H */

