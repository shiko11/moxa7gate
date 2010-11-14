#ifndef MODBUS_H
#define MODBUS_H

/*фЙРЩ ДБООЩИ*/
typedef unsigned char              u8;
typedef unsigned short             u16;

/* Predel'nye ТБЪНЕТЩ */
#define		MB_TCP_MAX_ADU_LENGTH		260		/* максимальная длина пакета  TCP MODBUS ADU принимаемого от OPC */
#define 	MB_TCP_ADU_HEADER_LEN		7
#define MB_SERIAL_MAX_ADU_LEN			256       // максимальная длина пакета-ответа
#define MB_SERIAL_MIN_ADU_LEN			3              // минимальная длина пакета-ответа (7

#define MB_FUNCTIONS_IMPLEMENTED 8
#define MB_FUNCTION_0x01	0
#define MB_FUNCTION_0x02	1
#define MB_FUNCTION_0x03	2
#define MB_FUNCTION_0x04	3
#define MB_FUNCTION_0x05	4
#define MB_FUNCTION_0x06	5
#define MB_FUNCTION_0x0f	6
#define MB_FUNCTION_0x10	7

#define TCPADU_SIZE_HI		4
#define TCPADU_SIZE_LO		5
#define TCPADU_ADDRESS		6
#define TCPADU_FUNCTION		7
#define TCPADU_START_HI		8
#define TCPADU_START_LO		9
#define TCPADU_LEN_HI			10
#define TCPADU_LEN_LO			11

#define RTUADU_ADDRESS		0
#define RTUADU_FUNCTION		1
#define RTUADU_START_HI		2
#define RTUADU_START_LO		3
#define RTUADU_LEN_HI			4
#define RTUADU_LEN_LO			5

#define RTUADU_BYTES			2

int 		mb_check_request_pdu(unsigned char *pdu, unsigned char len);
int 		mb_check_response_pdu(unsigned char *pdu, unsigned char len, unsigned char *request);

#endif  /* MODBUS_H */
