/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010                 
***************************************/

#ifndef MODBUS_H
#define MODBUS_H

///******************* МОДУЛЬ ВВОДА/ВЫВОДА MODBUS ******************************
///*** ОПРЕДЕЛЕННЫЕ СТАНДАРТОМ КОНСТАНТЫ ПРОКОЛА MODBUS
///*** ФУНКЦИИ ВВОДА/ВЫВОДА, ПРЕДНАЗНАЧЕННЫЕ ДЛЯ РАБОТЫ С MODBUS
///*** ФУНКЦИИ КОНФИГУРАЦИИ АППАРАТНЫХ ИНТЕРФЕЙСОВ

#include "statistics.h" /* учет статистики лишний здесь */

///=== MODBUS_H constants

// конфигурационные константы времени компиляции

#define DEVICE_NAME_LENGTH 64

/* Predel'nye ТБЪНЕТЩ */
#define		MB_TCP_MAX_ADU_LENGTH 260		/* максимальная длина пакета  TCP MODBUS ADU принимаемого от OPC */
#define 	MB_TCP_ADU_HEADER_LEN 7
#define MB_SERIAL_MAX_ADU_LEN   256       // максимальная длина пакета-ответа
#define MB_SERIAL_MIN_ADU_LEN   3              // минимальная длина пакета-ответа (7

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

/* лПННХОЙЛБГЙПООЩЕ ПЫЙВЛЙ */
#define COMMS_FAILURE                         0
#define ILLEGAL_FUNCTION                     -1
#define ILLEGAL_DATA_ADDRESS                 -2
#define ILLEGAL_DATA_VALUE                   -3
#define SLAVE_DEVICE_FAILURE                 -4
#define ACKNOWLEDGE                          -5
#define SLAVE_DEVICE_BUSY                    -6
#define NEGATIVE_ACKNOWLEDGE                 -7
#define MEMORY_PARITY_ERROR                  -8

#define PORT_FAILURE                        -11

//MB Err
#define MB_SERIAL_WRITE_ERR                 -1

#define MB_SERIAL_READ_FAILURE              -2
#define MB_SERIAL_COM_TIMEOUT   						-3

#define MB_SERIAL_ADU_ERR_MIN               -4
#define MB_SERIAL_ADU_ERR_MAX              	-5
#define MB_SERIAL_CRC_ERROR             		-6
#define MB_SERIAL_PDU_ERR  								  -7

/* нЕЦУЙНЧПМШОЩЕ ЧТЕНЕООЩЕ ЙОФЕТЧБМЩ */
#define TO_B110                    3200000   /* These values are the timeout delays */
#define TO_B300                    1600000   /* at the end of packets of data.      */
#define TO_B600                    800000    /* At this stage a true calculation    */
#define TO_B1200                   400000    /* has not been worked out. So these   */
#define TO_B2400                   200000    /* values are just a guess.            */
#define TO_B4800                   100000    /*                                     */
#define TO_B9600                   50000     /* The spec says that a message frame  */
#define TO_B19200                  25000     /* starts after a silent interval of   */
#define TO_B38400                  12500     /* at least 3.5 character times.       */
#define TO_B57600                  8333      /* These are uS times.                 */
#define TO_B115200            4167


/* лПДЩ ЖХОЛГЙК */

#define MB_TABLE_NONE 0
#define COIL_STATUS_TABLE				1
#define INPUT_STATUS_TABLE			2
#define INPUT_REGISTER_TABLE		3
#define HOLDING_REGISTER_TABLE	4

#define MB_ADDRESS_FIRST 0x0000
#define MB_ADDRESS_LAST  0xffff

#define MB_FUNC_NONE 0x00

#define MBF_READ_COILS                                 0x01
#define MBF_0x01_MIN_QUANTITY							0x0001
#define MBF_0x01_MAX_QUANTITY							0x07d0

#define MBF_READ_DECRETE_INPUTS                        0x02
#define MBF_0x02_MIN_QUANTITY							0x0001
#define MBF_0x02_MAX_QUANTITY							0x07d0

#define MBF_READ_HOLDING_REGISTERS                     0x03
#define MBF_0x03_MIN_QUANTITY							0x0001
#define MBF_0x03_MAX_QUANTITY							0x007d
///!!!#define MBF_0x03_MAX_QUANTITY							70

#define MBF_READ_INPUT_REGISTERS                       0x04
#define MBF_0x04_MIN_QUANTITY							0x0001
#define MBF_0x04_MAX_QUANTITY							0x007d
///!!!#define MBF_0x04_MAX_QUANTITY							70

#define MBF_WRITE_SINGLE_COIL                          0x05
#define MBF_SINGLE_COIL_OFF	 							0x0000
#define MBF_SINGLE_COIL_ON								0xff00

#define MBF_WRITE_SINGLE_REGISTER                      0x06

#define MBF_WRITE_MULTIPLE_COILS                       0x0F
#define MBF_0x0F_MIN_QUANTITY							0x0001
#define MBF_0x0F_MAX_QUANTITY							0x07b0

#define MBF_WRITE_MULTIPLE_REGISTERS                   0x10
#define MBF_0x10_MIN_QUANTITY							0x0001
#define MBF_0x10_MAX_QUANTITY							0x007b

#define MBF_READ_DEVICE_IDENTIFICATION                 0x2B
#define MBF_READ_WRITE_MULTIPLE_REGISTERS              0x17
#define MBF_MASK_WRITE_REGISTER                        0x16
#define MBF_READ_FIFO_QUEUE                            0x18
#define MBF_READ_FILE_RECORD                           0x14
#define MBF_WRITE_FILE_RECORD                          0x15
#define NBF_FILE_RECORD                                0x06
#define MBF_READ_EXCEPTION_STATUS                      0x07 ///< just for over serial line
#define MBF_DIAGNOSTIC                                 0x08 ///< just for over serial line
#define MBF_GET_COMM_EVENT_COUNTER                     0x0B ///< just for over serial line
#define MBF_GET_COMM_EVENT_LOG                         0x0C ///< just for over serial line
#define MBF_REPORT_SLAVE_ID                            0x11 ///< just for over serial line

#define MBF_HAS_EXCEPTION                              0x80
#define MBF_MASK                                       0x7F ///< mask function code

#define MODBUS_ADDRESS_BROADCAST	0
#define MODBUS_ADDRESS_MIN				1
#define MODBUS_ADDRESS_MAX				247

//*** TCP Recv ERR
#define	TCP_COM_ERR_NULL					-1
#define	TCP_ADU_ERR_MIN						-2
#define	TCP_ADU_ERR_MAX						-3
#define	TCP_ADU_ERR_PROTOCOL			-4
#define	TCP_ADU_ERR_LEN						-5
#define	TCP_ADU_ERR_UID						-6
#define	TCP_PDU_ERR								-7
#define	TCP_COM_ERR_SEND					-8

/*фЙРЩ ДБООЩИ*/
typedef unsigned char              u8;
typedef unsigned short             u16;

///*** modbus_tcp.c

int		mb_tcp_receive_adu(int sfd, GW_StaticData *stat, u8 *mb_received_adu, u16 *adu_len);
//int mb_tcp_send_adu(int sfd, input_cfg *context, u8 *pdu, u16 pdu_len, u8 *rq_adu);
//int mb_tcp_send_adu(int sfd, input_cfg *context, u8 *pdu, u16 pdu_len);
int mb_tcp_send_adu(int sfd, GW_StaticData *stat, u8 *pdu, u16 pdu_len, u8 *request, u16 *tcp_len);
int 		mb_check_request_pdu(unsigned char *pdu, unsigned char len);
int 		mb_check_response_pdu(unsigned char *pdu, unsigned char len, unsigned char *request);

///*** modbus_rtu.c

u16       crc(u8 *buf,u16 start,u16 cnt);
int mb_serial_send_adu(int ttyfd, GW_StaticData *stat, u8 *pdu, u16 pdu_len, u8 *adu, u16 *adu_len);
int receive_response(int ttyfd, u8 *received_string,int timeout,int ch_interval_timeout);
int mb_serial_receive_adu(int fd, GW_StaticData *stat, u8 *adu, u16 *adu_len, u8 *request, int timeout,int ch_interval_timeout);
int       open_comm(char *device,char *mode);
int       set_param_comms(int ttyfd,char *baud,char *parity);
int serial_receive_adu(int fd, GW_StaticData *stat, u8 *adu, u16 *adu_len, u8 *request, int timeout,int ch_interval_timeout); // iface_rtu_slave uses this

#endif  /* MODBUS_H */
