/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///******************* МОДУЛЬ ВВОДА/ВЫВОДА MODBUS ******************************

///=== MODBUS_H IMPLEMENTATION

#include <fcntl.h>  /* File control definitions */
#include <stdio.h>  /* Standard input/output */
#include <string.h>
#include <stdlib.h>
#include <termio.h> /* POSIX terminal control definitions */
#include <sys/time.h>    /* Time structures for select() */
#include <unistd.h> /* POSIX Symbolic Constants */
#include <errno.h>  /* Error definitions */

#ifndef ARCHITECTURE_I386
#include <moxadevice.h>
#endif

#include "modbus.h"

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

///-------------------------------------------------------------------------------

int mbcom_rtu_send(int ttyfd, u8 *adu, u16 adu_len)
	{
	int write_stat;

	tcflush(ttyfd, TCIOFLUSH);	/* flush the input & output streams */
	write_stat = write(ttyfd, adu, adu_len);
	tcflush(ttyfd, TCIFLUSH);		/* maybe not neccesary */
	
	if(write_stat!=adu_len) return MBCOM_SEND;
	
	return MBCOM_OK;
	}

///-------------------------------------------------------------------------------

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

//     if( !data_avail ) { /// 07.10.2010: в программе уже есть код для выдачи сообщения по таймауту
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

///-------------------------------------------------------------------------------

int mbcom_rtu_recv_rsp(int fd, u8 *adu, u16 *adu_len, int timeout, int ch_interval_timeout)
	{
	int mb_received_adu_len;
	
	u16 crc_calc;
	u16 crc_received;
	u8 recv_crc_hi;
	u8 recv_crc_lo;

  fd_set rfds;
  struct timeval tv;
  int data_avail = 0;
  int nbytes;
  struct termios settings;
  
  nbytes = *adu_len;
	
//-------------------------------------------------------------
  if(nbytes == 0) { // старая версия
	mb_received_adu_len = receive_response(	fd,
																					adu,
																					timeout,
																					ch_interval_timeout);
  } else { //оптимизированная версия --------------------------

	tv.tv_sec = 0;
	tv.tv_usec = timeout;

	FD_ZERO( &rfds );
	FD_SET ( fd, &rfds );

	/* wait for a response */
	data_avail = select(FD_SETSIZE, &rfds, NULL, NULL, &tv);

  tcgetattr(fd, &settings);
  settings.c_cc[VMIN]  = nbytes;
  tcsetattr(fd, TCSANOW, &settings);

//  tv.tv_sec=0;
//  tv.tv_usec=94000;
//  select(0, NULL, NULL, NULL, &tv);

  mb_received_adu_len = data_avail==0 ? 0 : read (fd, adu, nbytes);

  settings.c_cc[VMIN]  = 0;
  tcsetattr(fd, TCSANOW, &settings);

  //printf("read: %d/%d\n", mb_received_adu_len, nbytes);
  }
//-------------------------------------------------------------
	*adu_len=mb_received_adu_len;
	
	if (mb_received_adu_len<0) return MB_SERIAL_READ_FAILURE;
	if (mb_received_adu_len==0) return MB_SERIAL_COM_TIMEOUT;
	
	if (mb_received_adu_len < MB_SERIAL_MIN_ADU_LEN) return MB_SERIAL_ADU_ERR_MIN;
	if (mb_received_adu_len > MB_SERIAL_MAX_ADU_LEN) return MB_SERIAL_ADU_ERR_MAX;
	
	/*********** check CRC of response ************/
	crc_calc = crc(adu, 0, mb_received_adu_len-2);
	recv_crc_lo = (u16) adu[mb_received_adu_len-1];
	recv_crc_hi = (u16) adu[mb_received_adu_len-2];
	
	crc_received = (recv_crc_hi << 8) | recv_crc_lo;
	
	if (crc_calc != crc_received) return MB_SERIAL_CRC_ERROR;
	
//	if(mb_check_response_pdu(&adu[1], *adu_len-3, request)) {
//		return MB_SERIAL_PDU_ERR;
//		}
	
	return MBCOM_OK;
	}

///-------------------------------------------------------------------------------

int open_comm(char *device,char *mode)
{
	int	ttyfd;
	int	itmp;
	//ПФЛТЩФЙЕ РПТФБ
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
	//БОБМЙЪ РБТБНЕФТБ "ЙОФЕТЖЕКУ РПТФБ (ТЕЦЙН)"

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

///-------------------------------------------------------------------------------

int set_param_comms(int ttyfd,char *baud,char *parity, int timeout)
{
     struct termios      settings;
     speed_t             baud_rate;
	  int						char_interval_timeout;

	//БОБМЙЪ РБТБНЕФТБ "УЛПТПУФШ РПТФБ "
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

     settings.c_oflag &=~ OPOST;
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
	//БОБМЙЪ РБТБНЕФТБ "ЛПОФТПМШ ЮЕФОПУФЙ РПТФБ "
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

          settings.c_cc[VMIN] = 0; // этот параметр проверяется при каждом вызове функции чтения ответа

          if(timeout<100) {
            settings.c_cc[VTIME] = 1;
            } else if(timeout>=25500) {
            settings.c_cc[VTIME] = 255;
            } else {
            settings.c_cc[VTIME] = (timeout+50)/100;
            }

          if( tcsetattr( ttyfd, TCSANOW, &settings ) < 0 ) {
               fprintf( stderr, "tcsetattr failed\n");
               exit(1);
          }
			 return (char_interval_timeout);
}
/************************************************************************/

int mbcom_rtu_recv_req(int fd, u8 *adu, u16 *adu_len)
	{
	int mb_received_adu_len;
	int pdu_check;
	
	u16 crc_calc;
	u16 crc_received;
	u8 recv_crc_hi;
	u8 recv_crc_lo;
	
	///-------------------------------------------------
	
	struct termios newtio;
	tcgetattr(fd, &newtio);
	
	newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
	newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */
	
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd,TCSANOW,&newtio);
	
	mb_received_adu_len = read(fd, adu, MB_SERIAL_MAX_ADU_LEN);   /* returns after 5 chars have been input */
	
	///-------------------------------------------------

	*adu_len=mb_received_adu_len;
	
	if(mb_received_adu_len<0)  return MB_SERIAL_READ_FAILURE;
	if(mb_received_adu_len==0) return MB_SERIAL_COM_TIMEOUT; // !!! не действительно здесь
	
	if(mb_received_adu_len < MB_SERIAL_MIN_ADU_LEN) return MB_SERIAL_ADU_ERR_MIN;
	if(mb_received_adu_len > MB_SERIAL_MAX_ADU_LEN) return MB_SERIAL_ADU_ERR_MAX;
	
	/*********** check CRC of response ************/
	crc_calc = crc(adu, 0, mb_received_adu_len-2);
	recv_crc_lo = (u16) adu[mb_received_adu_len-1];
	recv_crc_hi = (u16) adu[mb_received_adu_len-2];
	
	crc_received = (recv_crc_hi << 8) | recv_crc_lo;
	
	if (crc_calc != crc_received) return MB_SERIAL_CRC_ERROR;
	
	/********** check for exception response *****/
//	if((pdu_check=mb_check_response_pdu(&adu[1], *adu_len-3, request))!=0) {
//		return MB_SERIAL_PDU_ERR;
//		}
	
	return MBCOM_OK;
	}
	
/************************************************************************/
int modbus_response_lenght(u8 *adu, u16 adu_len)
  {
  static int len;
  	
	switch(adu[TCPADU_FUNCTION]) {

		case MBF_READ_HOLDING_REGISTERS:
      len = 2*((adu[TCPADU_LEN_HI]<<8)+adu[TCPADU_LEN_LO]) + 5;
			break;
			
		case MBF_WRITE_MULTIPLE_REGISTERS:
      len = 8;
			break;
			
  	default: len=0;
    }
  	
  return len;
  }
/************************************************************************/
