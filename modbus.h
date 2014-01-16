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
#define MB_TCP_MAX_ADU_LENGTH 260	// максимальная длина пакета TCP MODBUS ADU
#define MB_TCP_ADU_HEADER_LEN 7
#define MB_SERIAL_MAX_ADU_LEN 256	// максимальная длина пакета-ответа
#define MB_SERIAL_MIN_ADU_LEN 3		// минимальная длина пакета-ответа
#define MB_SERIAL_CRC_LEN			2		// количество байт для хранения и передачи значения контрольной суммы
#define MB_UNIVERSAL_ADU_LEN	262 // байтовый массив такой длины нужен для работы с RTU и TCP одновременно

#define MB_FUNCTIONS_IMPLEMENTED 8

// адреса в пакетах modbus (действительны для запросов функций чтения и ответов функций записи)

#define TCPADU_TRANS_HI		0
#define TCPADU_TRANS_LO		1
#define TCPADU_PROTO_HI		2
#define TCPADU_PROTO_LO		3
#define TCPADU_SIZE_HI		4
#define TCPADU_SIZE_LO		5
#define TCPADU_ADDRESS		6
#define TCPADU_FUNCTION		7
#define TCPADU_START_HI		8
#define TCPADU_START_LO		9

#define TCPADU_LEN_HI			10
#define TCPADU_LEN_LO			11

#define TCPADU_BYTECOUNT	12

#define RTUADU_ADDRESS		0
#define RTUADU_FUNCTION		1
#define RTUADU_START_HI		2
#define RTUADU_START_LO		3

#define RTUADU_LEN_HI			4
#define RTUADU_LEN_LO			5

#define RTUADU_BYTES			2	// количество байт в полученном ответе

#define TCPADU_BYTES			8	// количество байт в полученном ответе


/* Межсимвольные временные интервалы */
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
#define TO_B115200                 4167


/* Коды функций */

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

#define MODBUS_ADDRESS_BROADCAST	0
#define MODBUS_ADDRESS_MIN				1
#define MODBUS_ADDRESS_MAX				247

/* Коммуникационные ошибки */

#define PORT_FAILURE						-11

// modbus RTU
#define MB_SERIAL_WRITE_ERR			0x10
#define MB_SERIAL_READ_FAILURE	0x20
#define MB_SERIAL_COM_TIMEOUT		0x30

#define MB_SERIAL_ADU_ERR_MIN		0x40
#define MB_SERIAL_ADU_ERR_MAX		0x41
#define	MB_SERIAL_ADU_ERR_UID		0x42

#define MB_SERIAL_CRC_ERROR			0x50
#define MB_SERIAL_PDU_ERR				0x60

// modbus TCP
#define	TCP_COM_ERR_SEND				0x11
#define	TCP_COM_ERR_NULL				0x21
#define TCP_COM_ERR_TIMEOUT  		0x31

#define	TCP_ADU_ERR_MIN					0x43
#define	TCP_ADU_ERR_MAX					0x44
#define	TCP_ADU_ERR_UID					0x45
#define	TCP_ADU_ERR_PROTOCOL		0x46
#define	TCP_ADU_ERR_LEN					0x47

#define	TCP_PDU_ERR							0x61

// унифицированные определения резульлтатов работы коммуникационных функций
#define MBCOM_OK								0x00
#define MBCOM_SEND							0x10
#define MBCOM_RECV							0x20
#define MBCOM_TIMEOUT						0x30
#define MBCOM_ADU								0x40
#define MBCOM_CRC								0x50
#define MBCOM_PDU								0x60

#define MBCOM_STAGE_MASK				0xF0
#define MBCOM_TYPE_MASK					0x0F

/* Типы данных */
typedef unsigned char              u8;
typedef unsigned short             u16;

///*** modbus_tcp.c

int mb_check_request_pdu(unsigned char *pdu, unsigned char len);
int mb_check_response_pdu(unsigned char *pdu, unsigned char len, unsigned char *request);

///*** modbus_rtu.c

int open_comm(char *device,char *mode);
int set_param_comms(int ttyfd,char *baud,char *parity, int timeout);

int receive_response(int ttyfd, u8 *received_string,int timeout,int ch_interval_timeout);
u16 crc(u8 *buf,u16 start,u16 cnt);

int modbus_response_lenght(u8 *adu, u16 adu_len);

// функции ввода/вывода modbus
int mbcom_rtu_send    (int ttyfd,       u8 *adu, u16 adu_len);
int mbcom_rtu_recv_rsp(int fd,          u8 *adu, u16 *adu_len, int timeout, int ch_interval_timeout);
int mbcom_rtu_recv_req(int fd,          u8 *adu, u16 *adu_len);

int mbcom_tcp_recv(int sfd, u8 *adu, u16 *adu_len);
int mbcom_tcp_send(int sfd, u8 *adu, u16  adu_len);

#endif  /* MODBUS_H */
