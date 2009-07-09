/*


*/


#include "modbus_tcp.h"
#include "global.h"

#define DEBUG_TCP
#define CRC_TCP

//int mb_tcp_receive_adu(int sfd,u8 *mb_received_adu) //прием ADU от подключившихся клиентов
int		mb_tcp_receive_adu(int sfd, GW_StaticData *stat, u8 *mb_received_adu, u16 *adu_len)
{	
	int		mb_received_adu_len;

	mb_received_adu_len = recv(sfd,mb_received_adu,MB_TCP_MAX_ADU_LENGTH,0);
	*adu_len=mb_received_adu_len;
	
	if (*adu_len<=0) {
		stat->errors_input_communication++;
		return TCP_COM_ERR_NULL;
		}
	if (mb_received_adu_len<MB_TCP_ADU_HEADER_LEN) {
		stat->errors_tcp_adu++;
		return TCP_ADU_ERR_MIN;
		}
	if (mb_received_adu_len>MB_TCP_MAX_ADU_LENGTH) {
		stat->errors_tcp_adu++;
		return TCP_ADU_ERR_MAX;
		}

	if(_show_data_flow) {
		int	i;
		for (i=0;i<mb_received_adu_len;i++) printf("{%0.2X}",mb_received_adu[i]);
		printf("\n");
	  }

	//unsigned int	ti=	(mb_received_adu[0]<<8)|mb_received_adu[1]; //any value
	unsigned int	pi=	(mb_received_adu[2]<<8)|mb_received_adu[3];
	unsigned int	len=(mb_received_adu[4]<<8)|mb_received_adu[5];
	unsigned char	ui=	mb_received_adu[6];
	
	if(pi!=0x0000) {
		stat->errors_tcp_adu++;
		return TCP_ADU_ERR_PROTOCOL;
		}
	if(len!=mb_received_adu_len-MB_TCP_ADU_HEADER_LEN+1) {
		stat->errors_tcp_adu++;
		return TCP_ADU_ERR_LEN;
		}
	if((ui<1)||(ui>247)) {
		stat->errors_tcp_adu++;
		return TCP_ADU_ERR_UID;
		}
	
//	if(mb_check_request_pdu(&mb_received_adu[7], mb_received_adu_len-MB_TCP_ADU_HEADER_LEN)) {
//		stat->errors_tcp_pdu++;
//	  return TCP_PDU_ERR;
//		}

	return 0;
}

/*###int mb_tcp_analis_adu(opc *OPC,u8 *adu) //анализируем принятый ADU
{
	int		i=0;
	
	OPC->mbhap.Tid 	= (adu[i] << 8) | (adu[i+=1] & 0x00FF);
	OPC->mbhap.Pid		= (adu[i+=1] << 8) | (adu[i+=1] & 0x00FF);
	OPC->mbhap.Length 	= (adu[i+=1] << 8) | (adu[i+=1] & 0x00FF);
	OPC->mbhap.Uid 	= adu[i+=1];
	

	OPC->pdu.function 	= adu[i+=1];
	OPC->pdu.start		= (adu[i+=1] << 8) | (adu[i+=1] & 0x00FF);	
	OPC->pdu.count		= (adu[i+=1] << 8) | (adu[i+=1] & 0x00FF);
	
///###	i = OPC->mbhap.Uid ;
	
	return i;
}

int mb_tcp_make_adu(opc *OPC,u16 *oDATA,u8 *mb_received_adu) //создаем ADU с считанными с у-в данными для отправки его клиенту
{
	int		adu_len = 0;
	int		i;
	u16		temp_crc;
#ifdef DEBUG_TCP			
	if (_mb_tcp) printf("MB TCP Maked ADU ... ");
#endif		
	if (OPC->pdu.function == MBF_READ_HOLDING_REGISTERS) {
		mb_received_adu[adu_len++]    = (OPC->mbhap.Tid) >> 8; 
		mb_received_adu[adu_len++] = (OPC->mbhap.Tid) & 0x00FF; 
		mb_received_adu[adu_len++] = (OPC->mbhap.Pid) >> 8; 
		mb_received_adu[adu_len++] = (OPC->mbhap.Pid) & 0x00FF; 
		mb_received_adu[adu_len++] = (OPC->mbhap.Length) >> 8; 
		mb_received_adu[adu_len++] = (OPC->mbhap.Length) & 0x00FF; 
		mb_received_adu[adu_len++] = (OPC->mbhap.Uid); 
		mb_received_adu[adu_len++] = (OPC->pdu.function); 
		mb_received_adu[adu_len++] = (OPC->pdu.count*2) & 0xFF; 		
		
		for (i=0;i<OPC->pdu.count;i++) {
			mb_received_adu[adu_len++] = oDATA[OPC->pdu.start + i ] >> 8; 
			mb_received_adu[adu_len++] = oDATA[OPC->pdu.start + i ] & 0x00FF; 
		}	
#ifdef 	CRC_TCP	
		if (!(_tcp_crc_off)) {
			temp_crc = crc(mb_received_adu,0,adu_len);
			mb_received_adu[adu_len++] = temp_crc >> 8;				//********
			mb_received_adu[adu_len++] = temp_crc & 0x00FF;			//	добавляем CRC в конец
		}
#endif
	}
#ifdef DEBUG_TCP	
	if (_mb_tcp) {		
		printf("OK");
		printf(" | Len = %d bytes\n",adu_len);
	}
#endif	
	return (adu_len);
}

int mb_tcp_make_status_adu(opc *OPC,u8 *mb_received_adu) //создаем ADU с данными о статусе точек для отправки его клиенту
{
	int		adu_len = 0;
	int		i;
	u16		temp_crc;
	
#ifdef DEBUG_TCP			
	if (_mb_tcp) printf("MB TCP Maked status ADU ... ");
#endif		
	if ((OPC->pdu.function == MBF_READ_HOLDING_REGISTERS) &&
		((OPC->mbhap.Uid == (OFFSET_STATUS_DEV + UID_STATUS_DEV_0)) || 
		 (OPC->mbhap.Uid == (OFFSET_STATUS_DEV + UID_STATUS_DEV_1)) ||
		 (OPC->mbhap.Uid == (OFFSET_STATUS_DEV + UID_STATUS_DEV_2)) ||
		 (OPC->mbhap.Uid == (OFFSET_STATUS_DEV + UID_STATUS_DEV_3)) ||
		 (OPC->mbhap.Uid == (OFFSET_STATUS_DEV + UID_STATUS_DEV_4)) ||
		 (OPC->mbhap.Uid == (OFFSET_STATUS_DEV + UID_STATUS_DEV_5)) ||
		 (OPC->mbhap.Uid == (OFFSET_STATUS_DEV + UID_STATUS_DEV_6)) ||
		 (OPC->mbhap.Uid == (OFFSET_STATUS_DEV + UID_STATUS_DEV_7)))) {
	
			mb_received_adu[adu_len++] = (OPC->mbhap.Tid) >> 8; 
			mb_received_adu[adu_len++] = (OPC->mbhap.Tid) & 0x00FF; 
			mb_received_adu[adu_len++] = (OPC->mbhap.Pid) >> 8; 
			mb_received_adu[adu_len++] = (OPC->mbhap.Pid) & 0x00FF; 
			mb_received_adu[adu_len++] = (OPC->mbhap.Length) >> 8; 
			mb_received_adu[adu_len++] = (OPC->mbhap.Length) & 0x00FF; 
			mb_received_adu[adu_len++] = (OPC->mbhap.Uid); 
			mb_received_adu[adu_len++] = (OPC->pdu.function); 
			mb_received_adu[adu_len++] = (OPC->pdu.count*2) & 0xFF; 		
		
			for (i=0;i<OPC->pdu.count;i++) {
				mb_received_adu[adu_len++] = (iDATA[OPC->mbhap.Uid - OFFSET_STATUS_DEV].map[OPC->pdu.start + i ].status) >> 8; 
				mb_received_adu[adu_len++] = (iDATA[OPC->mbhap.Uid - OFFSET_STATUS_DEV].map[OPC->pdu.start + i ].status) & 0x00FF; 
			}	
#ifdef 	CRC_TCP	
			if (!(_tcp_crc_off)) {
				temp_crc = crc(mb_received_adu,0,adu_len);
				mb_received_adu[adu_len++] = temp_crc >> 8;				//********
				mb_received_adu[adu_len++] = temp_crc & 0x00FF;			//	добавляем CRC в конец
			}
#endif
#ifdef DEBUG_TCP	
			if (_mb_tcp) {		
				printf("OK");
				printf(" | Len = %d bytes\n",adu_len);
			}
#endif	
	} else {
#ifdef DEBUG_TCP	
				if (_mb_tcp) {		
					printf("FAIL");
					printf(" | Len = %d bytes\n",adu_len);
				}
#endif		
	}
	return (adu_len);
}*/
	
//int mb_tcp_send_adu(int sfd, input_cfg *context) //собственно сама отправка ADU клиенту
int mb_tcp_send_adu(int sfd, GW_StaticData *stat, u8 *pdu, u16 pdu_len, u8 *request, u16 *tcp_len)
{
	int	sended_bytes;

	int i, j=6;
	for(i=0; i<pdu_len; i++)
	  request[j++]=pdu[i];
	request[5]=pdu_len; // unsigned char
	*tcp_len=pdu_len-1+MB_TCP_ADU_HEADER_LEN;

	if(_show_data_flow) {
		for (i=0;i<*tcp_len;i++) printf("{%0.2X}",request[i]);
		printf("\n");
	  }

	sended_bytes=send(sfd, request, *tcp_len, 0);
	if(sended_bytes!=*tcp_len) {
		stat->errors_tcp_sending++;
		return TCP_COM_ERR_SEND;
		}

//	printf("noop\n");

	return 0;
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
