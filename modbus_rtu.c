/*
MOXA7GATE MODBUS GATEWAY SOFTWARE
SEM-ENGINEERING
                    BRYANSK 2009
*/

/*        modbus_rtu.c
МОЯ РЕДАКЦИЯ ?1
*/

#include <fcntl.h>  /* File control definitions */
#include <stdio.h>  /* Standard input/output */
#include <string.h>
#include <stdlib.h>
#include <termio.h> /* POSIX terminal control definitions */
#include <sys/time.h>    /* Time structures for select() */
#include <unistd.h> /* POSIX Symbolic Constants */
#include <errno.h>  /* Error definitions */

#include "global.h"

#ifndef ARCHITECTURE_I386
#include <moxadevice.h>
#endif

#include "modbus_rtu.h"

#define DEBUG_MB_RTU          /* uncomment to see the data sent and received */
#define CRC_RTU


/****************************************************************************
Рассчитывает контрольную сумму CRC 16
****************************************************************************/
u16 crc(u8 *buf,u16 start,u16 cnt)
{
     int       i,j;
     u16  temp,temp2,flag;

     temp = 0xFFFF;

     for (i=start; i<cnt; i++) {
          temp=temp ^ buf[i];
          for (j=1;j<=8;j++) {
               flag = temp & 0x0001;
               temp = temp >> 1;
               if (flag) temp = temp ^ 0xA001;
          }
     }
     /* Reverse byte order. */
     temp2 = temp >> 8;
     temp = (temp << 8) | temp2;
     temp &= 0xFFFF;

     return(temp);
}

/***********************************************************************
Сборка пакета-запроса ADU для отправки его slave устройству
возвращаем размер ADU в байтах
***********************************************************************/
/***********************************************************************
  Посылаем готовый пакет-запрос в порт (slave устройству)
  Возвращает кол-во реально посланных байт при удачном
  или -1?? если ошибка
************************************************************************/
int mb_serial_send_adu(int ttyfd, GW_StaticData *stat, u8 *pdu, u16 pdu_len, u8 *adu, u16 *adu_len)
	{
     int       write_stat;
     u16       cur_pos = 0;
     u16       temp_crc;

    int i; // copy PDU
    for(i=0; i<pdu_len; i++)
      adu[cur_pos++]=pdu[i];
    *adu_len=pdu_len+2;

//     adu[adu_len++]       = slave;                     //**************
//     adu[adu_len++]       = function;                  //
//     start -= 1;                                  //
//     adu[adu_len++]       = start >> 8;           //   создаем PDU в ADU
//     adu[adu_len++]       = start & 0x00ff;       //
//     adu[adu_len++]       = count >> 8;                //
//     adu[adu_len++]       = count &0x00ff;             //**************

// ДКЪ ОПНБЕПЙХ ОНБЕДЕМХЪ ОНФЮПМШУ ДЮРВХЙНБ БШДЮЕЛ ЬХПНЙНБЕЫЮРЕКЭМШИ ГЮОПНЯ:
// adu[RTUADU_ADDRESS]=MODBUS_ADDRESS_BROADCAST;

//     if (!(_rtu_crc_off)) {
          temp_crc = crc(adu, 0, cur_pos);
          // ДКЪ ОПНБЕПЙХ ОНБЕДЕМХЪ ОНФЮПМШУ ДЮРВХЙНБ НРЙКЧВЮЕЛ БПЕЛЕММН ТНПЛХПНБЮМХЕ ЙНМРПНКЭМНИ ЯСЛЛШ:
          adu[cur_pos++] = temp_crc >> 8;                   //********
          adu[cur_pos++] = temp_crc & 0x00FF;               //   добавляем CRC в конец
//     }

//	if(gate502.show_data_flow==1) {
//    for (i=0;i<cur_pos;i++) printf("[%0.2X]",adu[i]);
//    printf("\n");
//		}

	tcflush(ttyfd,TCIOFLUSH);     /* flush the input & output streams */
	write_stat = write(ttyfd,adu,*adu_len);
	tcflush(ttyfd,TCIFLUSH); /* maybe not neccesary */
	
	if(write_stat!=*adu_len) {
		stat->errors_serial_sending++;
		return MB_SERIAL_WRITE_ERR;
		}
	
  return 0;
	}

/***********************************************************************

***********************************************************************/
int receive_response(int ttyfd, u8 *received_string,int timeout,int ch_interval_timeout)
{
     u8        rxchar = PORT_FAILURE;//+
     int       data_avail = 0;
     int       bytes_received = 0;
     int       read_stat;
     fd_set    rfds;
     struct timeval tv;
	  
     
     tv.tv_sec = 0;
	  tv.tv_usec = timeout;

     FD_ZERO( &rfds );
     FD_SET( ttyfd, &rfds );

#ifdef DEBUG_MB_RTU
//     if (_mb_rtu) printf("Waiting for response from slave.\n");
#endif
     /* wait for a response */
     data_avail = select(FD_SETSIZE,&rfds,NULL,NULL,&tv );

//     if( !data_avail ) { /// 07.10.2010: Б ОПНЦПЮЛЛЕ СФЕ ЕЯРЭ ЙНД ДКЪ БШДЮВХ ЯННАЫЕМХЪ ОН РЮИЛЮСРС
//          printf("Comms time out\n");
//     }

     tv.tv_sec = 0;
	  tv.tv_usec = 8*ch_interval_timeout;

     FD_ZERO(&rfds );
     FD_SET(ttyfd, &rfds );
#ifdef DEBUG_MB_RTU
//     if (_mb_rtu) printf("MB Serial Received ADU from Slave: \n");
#endif
     while (data_avail) {
     if (select(FD_SETSIZE,&rfds,NULL,NULL,&tv)) {
          read_stat = read( ttyfd, &rxchar, 1 );
          if (read_stat < 0) {
               bytes_received = read_stat;
               data_avail = 0;
          } else {
               rxchar = rxchar & 0xFF;
               received_string[bytes_received++] = rxchar;
          }
#ifdef DEBUG_MB_RTU
//          if (_mb_rtu) printf("{%0.2X}",rxchar);
#endif
     } else {
     data_avail = 0;
     }
     }
#ifdef DEBUG_MB_RTU
//     int  i;
//     if (_mb_rtu) {
//          printf("\n");
//          printf("Received %d bytes \n",bytes_received);
//     }
#endif

     return (bytes_received);
}

/*********************************************************************
 Прием пакета-ответа данных из порта от slave устройства
 Проверка CRC и кода функции
 при удачной проверке - возвращается длина всех принятых байт (ADU) иначе 0

**********************************************************************/
int mb_serial_receive_adu(int fd, GW_StaticData *stat, u8 *adu, u16 *adu_len, u8 *request, int timeout,int ch_interval_timeout)
	{
	int mb_received_adu_len;
	
	u16 crc_calc = 0;
	u16 crc_received = 0;
	u8 recv_crc_hi;
	u8 recv_crc_lo;

	mb_received_adu_len = receive_response(fd, adu, timeout, ch_interval_timeout);
	*adu_len=mb_received_adu_len;
	
	if (mb_received_adu_len<0) {
		stat->errors_serial_accepting++;
		return MB_SERIAL_READ_FAILURE;
		}
	if (mb_received_adu_len==0) {
		stat->timeouts++;
		return MB_SERIAL_COM_TIMEOUT;
		}
	
	if (mb_received_adu_len < MB_SERIAL_MIN_ADU_LEN) {
		stat->errors_serial_adu++;
		return MB_SERIAL_ADU_ERR_MIN;
		}
	if (mb_received_adu_len > MB_SERIAL_MAX_ADU_LEN) {
		stat->errors_serial_adu++;
		return MB_SERIAL_ADU_ERR_MAX;
		}
	
//	if(gate502.show_data_flow==1) {
//    int i;
//    for (i=0;i<mb_received_adu_len;i++) printf("[%0.2X]",adu[i]);
//		printf("\n");
//	  }

    crc_calc = crc(adu, 0, mb_received_adu_len - 2);
    recv_crc_hi = (u16) adu[mb_received_adu_len - 1];
    recv_crc_lo = (u16) adu[mb_received_adu_len - 2];

    crc_received = recv_crc_lo << 8;
    crc_received = crc_received | recv_crc_hi;
    
    /*********** check CRC of response ************/
    if (crc_calc != crc_received) {
    	stat->crc_errors++;
      return MB_SERIAL_CRC_ERROR;
    	}

		/********** check for exception response *****/
//     printf("TEST1    IN(%0.2d): ", context->tcp_adu_len);
//     for (i=0;i<context->tcp_adu_len;i++) printf("[%0.2X]",context->tcp_adu[i]);
//		 printf("!%d\n", context->tcp_adu[MB_TCP_ADU_HEADER_LEN]);

#ifndef ARCHITECTURE_I386
	if(mb_check_response_pdu(&adu[1], *adu_len-3, request)) {
		stat->errors_serial_pdu++;
		return MB_SERIAL_PDU_ERR;
		}
#endif
     
     return 0;
}

/************************************************************************
Чтение Holding Registers
Формируем массив(u16) и возвращаем ЧИСЛО ПРОЧИТАННЫХ РЕГИСТРОВ!!!
*************************************************************************/
/************************************************************************
*************************************************************************/

int open_comm(char *device,char *mode)
{
	int	ttyfd;
	int	itmp;
	//открытие порта
#ifdef DEBUG_MB_RTU
//     if (_mb_rtu) printf("opening %s\n",device);
#endif
	if((ttyfd = open(device,O_RDWR)) < 0 ) { ///!!! | O_NOCTTY
	fprintf(stderr,"Error opening device %s. ",device);
	fprintf(stderr,"Error no. %d \n",errno);
	exit (1); 
	}
#ifdef DEBUG_MB_RTU
//     if (_mb_rtu) printf( "%s open\n", device );
#endif
	//анализ параметра "интерфейс порта (режим)"

#ifndef ARCHITECTURE_I386

	if ( strcmp( mode, "RS232") == 0 ) {
	itmp = RS232_MODE;
	} else {
		if ( strcmp( mode, "RS485_2w") == 0 ) {
			itmp = RS485_2WIRE_MODE;
		} else {
			if ( strcmp( mode, "RS422") == 0 ) {
				itmp = RS422_MODE;
			} else {
				if ( strcmp( mode, "RS485_4w") == 0 ) {
					itmp = RS485_4WIRE_MODE;
				} else {
					printf("Unknown mode: %s\n ",mode);
					exit (1);
				}
			}
		}
	}
	if( ioctl( ttyfd, MOXA_SET_OP_MODE, &itmp ) < 0 ) {
		fprintf( stderr, "set mode failed\n");
		exit(1);
	}
#endif

	return (ttyfd);	
}
/************************************************************************

**************************************************************************/
int set_param_comms(int ttyfd,char *baud,char *parity)
{
     struct termios      settings;
     speed_t             baud_rate;
	  int						char_interval_timeout;

	//анализ параметра "скорость порта "
     if (strcmp(baud,"110" ) == 0) {
     baud_rate = B110;
     char_interval_timeout = TO_B110;
     } else
          if (strcmp(baud,"300" ) == 0) {
               baud_rate = B300;
               char_interval_timeout = TO_B300;
          } else
               if (strcmp(baud,"600" ) == 0) {
                    baud_rate = B600;
                    char_interval_timeout = TO_B600;
               } else
                    if (strcmp(baud,"1200" ) == 0) {
                         baud_rate = B1200;
                         char_interval_timeout = TO_B1200;
                    } else
                         if (strcmp(baud,"2400" ) == 0) {
                              baud_rate = B2400;
                              char_interval_timeout = TO_B2400;
                         } else
                              if (strcmp(baud,"4800" ) == 0) {
                                   baud_rate = B4800;
                                   char_interval_timeout = TO_B4800;
                              } else
                                   if (strcmp(baud,"9600" ) == 0) {
                                        baud_rate = B9600;
                                        char_interval_timeout = TO_B9600;
                                   } else
                                        if (strcmp(baud,"19200" ) == 0) {
                                             baud_rate = B19200;
                                             char_interval_timeout = TO_B19200;
                                        } else
                                             if (strcmp(baud,"38400" ) == 0) {
                                                  baud_rate = B38400;
                                                  char_interval_timeout = TO_B38400;
                                             } else
                                                  if (strcmp(baud,"57600" ) == 0) {
                                                       baud_rate = B57600;
                                                       char_interval_timeout = TO_B57600;
                                                  } else
                                                       if (strcmp(baud,"115200" ) == 0) {
                                                            baud_rate = B115200;
                                                            char_interval_timeout = TO_B115200;
                                                       } else
                                                       {
                                                            printf("Unknown baud rate %s\n ",baud);
                                                            exit (1);
                                                       }

     /* read your man page for the meaning of all this. # man termios */
     /* Its a bit to involved to comment here                         */
     cfsetispeed( &settings, baud_rate );/* Set the baud rate */
     cfsetospeed( &settings, baud_rate );

     settings.c_line = 0;

     settings.c_iflag |= IGNBRK;
     settings.c_iflag |= IGNPAR;
     settings.c_iflag &=~ PARMRK;
     settings.c_iflag &=~ INPCK;
     settings.c_iflag &=~ ISTRIP;
     settings.c_iflag &=~ INLCR;
     settings.c_iflag &=~ IGNCR;
     settings.c_iflag &=~ ICRNL;
     settings.c_iflag &=~ IUCLC;
     settings.c_iflag &=~ IXON;
     settings.c_iflag |= IXANY;
     settings.c_iflag &=~ IXOFF;
     settings.c_iflag &=~ IMAXBEL;

     settings.c_oflag |= OPOST;
     settings.c_oflag &=~ OLCUC;
     settings.c_oflag &=~ ONLCR;
     settings.c_oflag &=~ OCRNL;
     settings.c_oflag |= ONOCR;
     settings.c_oflag &=~ ONLRET;
     settings.c_oflag &=~ OFILL;
     settings.c_oflag &=~ OFDEL;

     settings.c_cflag &=~ CSIZE;
     settings.c_cflag |= CS8;
     settings.c_cflag &=~ CSTOPB;
     settings.c_cflag |= CREAD;
	//анализ параметра "контроль четности порта "
     if( strcmp( parity, "none" ) == 0 ) {
          settings.c_cflag &=~ PARENB;
          settings.c_cflag &=~ PARODD;
     }
     else
          if( strcmp( parity, "even" ) == 0 ) {
          settings.c_cflag |= PARENB;
          settings.c_cflag &=~ PARODD;
          }
          else {
				printf("Unknown parity: %s\n ",parity);
				exit (1);
          }
          settings.c_cflag &=~ HUPCL;
          settings.c_cflag |= CLOCAL;
          settings.c_cflag &=~ CRTSCTS;

          settings.c_lflag &=~ ISIG;
          settings.c_lflag &=~ ICANON;
          settings.c_lflag &=~ ECHO;
          settings.c_lflag |= IEXTEN;

          settings.c_cc[VMIN] = 0;
          settings.c_cc[VTIME] = 0;

          if( tcsetattr( ttyfd, TCSANOW, &settings ) < 0 ) {
               fprintf( stderr, "tcsetattr failed\n");
               exit(1);
          }
			 return (char_interval_timeout);
}
/************************************************************************/

int serial_receive_adu(int fd, GW_StaticData *stat, u8 *adu, u16 *adu_len, u8 *request, int timeout,int ch_interval_timeout)
	{
	int mb_received_adu_len;
	
	u16 crc_calc = 0;
	u16 crc_received = 0;
	u8 recv_crc_hi;
	u8 recv_crc_lo;
///-------------------------------------------------
//	mb_received_adu_len = receive_response(fd, adu, timeout, ch_interval_timeout);
	
        struct termios newtio;
				tcgetattr(fd,&newtio);
         
        newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
        newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */
        
        tcflush(fd, TCIFLUSH);
        tcsetattr(fd,TCSANOW,&newtio);
        
//        while (STOP==FALSE) {       /* loop for input */
          mb_received_adu_len = read(fd, adu, MB_SERIAL_MAX_ADU_LEN);   /* returns after 5 chars have been input */
//          buf[res]=0;               /* so we can printf... */
//          printf(":%s:%d\n", buf, res);
//          if (buf[0]=='z') STOP=TRUE;
//        }

///-------------------------------------------------
	*adu_len=mb_received_adu_len;

	if (mb_received_adu_len<0) {
		stat->errors_serial_accepting++;
		return MB_SERIAL_READ_FAILURE;
		}
	if (mb_received_adu_len==0) {
		stat->timeouts++;
		return MB_SERIAL_COM_TIMEOUT;
		}
	
	if (mb_received_adu_len < MB_SERIAL_MIN_ADU_LEN) {
		stat->errors_serial_adu++;
		return MB_SERIAL_ADU_ERR_MIN;
		}
	if (mb_received_adu_len > MB_SERIAL_MAX_ADU_LEN) {
		stat->errors_serial_adu++;
		return MB_SERIAL_ADU_ERR_MAX;
		}
	
//	if(gate502.show_data_flow==1) {
//    int i;
//    for (i=0;i<mb_received_adu_len;i++) printf("[%0.2X]",adu[i]);
//		printf("\n");
//	  }

    crc_calc = crc(adu, 0, mb_received_adu_len - 2);
    recv_crc_hi = (u16) adu[mb_received_adu_len - 1];
    recv_crc_lo = (u16) adu[mb_received_adu_len - 2];

    crc_received = recv_crc_lo << 8;
    crc_received = crc_received | recv_crc_hi;
    
    /*********** check CRC of response ************/
    if (crc_calc != crc_received) {
    	stat->crc_errors++;
      return MB_SERIAL_CRC_ERROR;
    	}

		/********** check for exception response *****/
//     printf("TEST1    IN(%0.2d): ", context->tcp_adu_len);
//     for (i=0;i<context->tcp_adu_len;i++) printf("[%0.2X]",context->tcp_adu[i]);
//		 printf("!%d\n", context->tcp_adu[MB_TCP_ADU_HEADER_LEN]);

//  int test;
//	if((test=mb_check_response_pdu(&adu[1], *adu_len-3, request))!=0) {
//		stat->errors_serial_pdu++;
//		return MB_SERIAL_PDU_ERR;
//		printf("pdu_err:%d\n", test);
//		return test;
//		}
     
     return 0;
}

/************************************************************************/
