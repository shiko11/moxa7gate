/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///******************* ÌÎÄÓËÜ ÂÂÎÄÀ/ÂÛÂÎÄÀ MODBUS ******************************

///=== MODBUS_H IMPLEMENTATION

#include <errno.h>

#include "modbus.h"

//#define DEBUG_TCP
//#define CRC_TCP

int mbcom_tcp_recv(int sfd, u8 *adu, u16 *adu_len)
	{
	int		mb_received_adu_len;
	
	*adu_len=0;
	
	mb_received_adu_len = recv(sfd, adu, MB_TCP_MAX_ADU_LENGTH, 0);

	if ( mb_received_adu_len== 0)										 	return TCP_COM_ERR_NULL;
	if ((mb_received_adu_len==-1) && (errno==EAGAIN))	return TCP_COM_ERR_TIMEOUT;
	if ( mb_received_adu_len==-1)										 	return TCP_COM_ERR_NULL;

	*adu_len=mb_received_adu_len;
	if (*adu_len <  MB_TCP_ADU_HEADER_LEN)	return TCP_ADU_ERR_MIN;
	if (*adu_len >  MB_TCP_MAX_ADU_LENGTH)	return TCP_ADU_ERR_MAX;
	
	unsigned int	pi=	(adu[TCPADU_PROTO_HI]<<8) | adu[TCPADU_PROTO_LO];
	unsigned int	len=(adu[TCPADU_SIZE_HI]<<8) | adu[TCPADU_SIZE_LO];
	unsigned char	ui=	adu[TCPADU_ADDRESS];
	
	if(pi!=0x0000)														return TCP_ADU_ERR_PROTOCOL;
	if(len!=(*adu_len-MB_TCP_ADU_HEADER_LEN+1))	return TCP_ADU_ERR_LEN;
	if((ui<MODBUS_ADDRESS_MIN)||(ui>MODBUS_ADDRESS_MAX)) return TCP_ADU_ERR_UID;
	
	return MBCOM_OK;
	}

int mbcom_tcp_send(int sfd, u8 *adu, u16 adu_len)
	{
	int	sended_bytes;
	
	sended_bytes=send(sfd, adu, adu_len, 0);

	if(sended_bytes!=adu_len) return MBCOM_SEND;
	
	return MBCOM_OK;
	}
///-------------------------------------------------------------------------------------------------------
int 		mb_check_request_pdu(unsigned char *pdu, unsigned char len)
  {
  int res=0;
  unsigned short i, j;

  switch(pdu[0]) {

  	case 0x01: // (0x01) Read Coils
  	  j=(pdu[3]<<8)|pdu[4];
  	  if((j<1)||(j>2000)||(len!=5)) res=1;
  	  break;

  	case 0x02: // (0x02) Read Discrete Inputs
  	  j=(pdu[3]<<8)|pdu[4];
  	  if((j<1)||(j>2000)||(len!=5)) res=3;
  	  break;

  	case 0x03: // (0x03) Read Holding Registers
  	  j=(pdu[3]<<8)|pdu[4];
  	  if((j<1)||(j>125)||(len!=5)) res=5;
  	  break;

  	case 0x04: // (0x04) Read Input Registers
  	  j=(pdu[3]<<8)|pdu[4];
  	  if((j<1)||(j>125)||(len!=5)) res=7;
  	  break;

  	case 0x05: // (0x05) Write Single Coil
  	  j=(pdu[3]<<8)|pdu[4];
  	  if(((j!=0x0000)&&(j!=0xff00))||(len!=5)) res=9;
  	  break;

  	case 0x06: // (0x06) Write Single Register
  	  if(len!=5) res=11;
  	  break;

  	case 0x0f: // (0x0F) Write Multiple Coils
  	  i=(pdu[3]<<8)|pdu[4];
	  	j=(i%8)==0?i/8:i/8+1;
  	  if((i<1)||(i>0x07b0)||(pdu[5]!=j)||(j!=len-6)) res=13;
  	  break;

  	case 0x10: // (0x10) Write Multiple registers
  	  i=(pdu[3]<<8)|pdu[4];
  	  j=i*2;
  	  if((i<1)||(i>0x007b)||(j!=pdu[5])||(j!=len-6)) res=15;
  	  break;

  	default: res=0xff;
    };
  
  return res;
  }
///-------------------------------------------------------------------------------------------------------
int 		mb_check_response_pdu(unsigned char *pdu, unsigned char len, unsigned char *request)
  {
  if((pdu[0]&0x7f)!=(request[0]&0x7f)) return 0xff; // function mismath

  int res=0;
  unsigned short i, j;


  switch(pdu[0]&0x7f) {

  	case 0x01: // (0x01) Read Coils
  	  if((pdu[0]&0x80)==0) {
	  	  i=(request[3]<<8)|request[4];
	  	  j=(i%8)==0?i/8:i/8+1;
	  	  if((j!=pdu[1])||(j!=len-2)) res=1;
  	    } else if((pdu[1]<1)||(pdu[1]>4)) res=2;
  	  break;

  	case 0x02: // (0x02) Read Discrete Inputs
  	  if((pdu[0]&0x80)==0) {
	  	  i=(request[3]<<8)|request[4];
	  	  j=(i%8)==0?i/8:i/8+1;
	  	  if((j!=pdu[1])||(j!=len-2)) res=3;
  	    } else if((pdu[1]<1)||(pdu[1]>4)) res=4;
  	  break;

  	case 0x03: // (0x03) Read Holding Registers
  	  if((pdu[0]&0x80)==0) {
	  	  i=(request[3]<<8)|request[4];
	  	  j=i*2;
	  	  if((j!=pdu[1])||(j!=len-2)) res=5;
  	    } else if((pdu[1]<1)||(pdu[1]>4)) res=6;
  	  break;

  	case 0x04: // (0x04) Read Input Registers
  	  if((pdu[0]&0x80)==0) {
	  	  i=(request[3]<<8)|request[4];
	  	  j=i*2;
	  	  if((j!=pdu[1])||(j!=len-2)) res=7;
  	    } else if((pdu[1]<1)||(pdu[1]>4)) res=8;
  	  break;

  	case 0x05: // (0x05) Write Single Coil
  	  if((pdu[0]&0x80)==0) {
	  	  i=(request[1]<<8)|request[2];
  	  	j=(pdu[1]<<8)|pdu[2];
	  	  if(i!=j) res=9;
	  	  i=(request[3]<<8)|request[4];
  	  	j=(pdu[3]<<8)|pdu[4];
	  	  if((i!=j)||(len!=5)) res=9;
  	    } else if((pdu[1]<1)||(pdu[1]>4)) res=10;
  	  break;

  	case 0x06: // (0x06) Write Single Register
  	  if((pdu[0]&0x80)==0) {
	  	  i=(request[1]<<8)|request[2];
  	  	j=(pdu[1]<<8)|pdu[2];
	  	  if(i!=j) res=11;
	  	  i=(request[3]<<8)|request[4];
  	  	j=(pdu[3]<<8)|pdu[4];
	  	  if((i!=j)||(len!=5)) res=11;
  	    } else if((pdu[1]<1)||(pdu[1]>4)) res=12;
  	  break;

  	case 0x0f: // (0x0F) Write Multiple Coils
  	  if((pdu[0]&0x80)==0) {
	  	  i=(request[3]<<8)|request[4];
	  	  j=(pdu[3]<<8)|pdu[4];
	  	  if((i!=j)||(len!=5)) res=13;
  	  	} else if((pdu[1]<1)||(pdu[1]>4)) res=14;
  	  break;

  	case 0x10: // (0x10) Write Multiple registers
  	  if((pdu[0]&0x80)==0) {
	  	  i=(request[1]<<8)|request[2];
  	  	j=(pdu[1]<<8)|pdu[2];
	  	  if(i!=j) res=15;
	  	  i=(request[3]<<8)|request[4];
  	  	j=(pdu[3]<<8)|pdu[4];
	  	  if((i!=j)||(len!=5)) res=15;
  	    } else if((pdu[1]<1)||(pdu[1]>4)) res=16;
  	  break;

  	default: res=0xff; // unsupported function
    };
  
  return res;
  }
///-------------------------------------------------------------------------------------------------------
