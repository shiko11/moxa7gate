/*
MOXA7GATE MODBUS GATEWAY SOFTWARE
SEM-ENGINEERING
                    BRYANSK 2009
*/

#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

/* Коммуникационные ошибки */
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
#define TO_B115200            4167


/* Коды функций */

#define COIL_STATUS_TABLE				1
#define MIN_STARTING_ADDRESS_1X	0x0000
#define MAX_STARTING_ADDRESS_1X	0xffff

#define INPUT_STATUS_TABLE			2
#define MIN_STARTING_ADDRESS_2X	0x0000
#define MAX_STARTING_ADDRESS_2X	0xffff

#define INPUT_REGISTER_TABLE		3
#define MIN_STARTING_ADDRESS_3X	0x0000
#define MAX_STARTING_ADDRESS_3X	0xffff

#define HOLDING_REGISTER_TABLE	4
#define MIN_STARTING_ADDRESS_4X	0x0000
#define MAX_STARTING_ADDRESS_4X	0xffff

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

#define MODBUS_ADDRESS_MIN				1
#define MODBUS_ADDRESS_MAX				247
#define MODBUS_ADDRESS_BROADCAST	0

#include "global.h"

u16       crc(u8 *buf,u16 start,u16 cnt);
int       open_comm(char *device,char *mode);
int       set_param_comms(int ttyfd,char *baud,char *parity);

//int       mb_serial_read_holding_registers(int ttyfd,u8 port,u8 slave,u16 start,u16 count,u16 dest,u16 *dst,int timeout,int ch_interval_timeout);
//int       mb_serial_read_input_registers(int ttyfd,u8 port,u8 slave,u16 start,u16 count,u16 dest,u16 *dst,int timeout,int ch_interval_timeout);
//int				mb_serial_analis_adu(u8 *adu);

#endif  /* MODBUS_RTU_H */
