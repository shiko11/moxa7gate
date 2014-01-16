/***********   K M - 4 0 0   *************
      КОММУНИКАЦИОННЫЙ МОДУЛЬ                                
                              ВЕРСИЯ 1.0
      ООО "БЗПА"
               БРЯНСК 2013                 
*****************************************/

///**** МОДУЛЬ ДЛЯ РАБОТЫ С КОНТРОЛЛЕРОМ ТЕЛЕМЕХАНИКИ ПО ЧАСТНОМУ ПРОТОКОЛУ
///**** ("ПРОТОКОЛ ПЕРЕДАЧИ ПАКЕТОВ", РАБОТАЮЩИЙ ПОВЕРХ MODBUS RTU/MODBUS TCP)

/* Предполагается использование этого модуля в параллельном потоке программы,
   требуется настроенный интерфейс типа IFACE_RTUMASTER или IFACE_TCPMASTER
   для циклического опроса и записи значений переменных PLC и CM в КЛТМ */

#include <fcntl.h>  /* File control definitions */
//#include <stdio.h>  /* Standard input/output */
//#include <stdlib.h> /* Standard library definitions */
#include <termio.h> /* POSIX terminal control definitions */
#include <errno.h>  /* Error definitions */
#include <unistd.h> /* POSIX Symbolic Constants */
//#include <string.h>

#include <pthread.h>

#include "kltm.h"
#include "../interfaces.h"
#include "../moxagate.h"

///=== KLTM_H private variables

#define DEVICE_NAME "/dev/ttyM3"

//#define MODEM_COMMAND_LEN 80
//typedef struct { // структура параметров состояния и настройки модема
//  char at[MODEM_COMMAND_LEN]; // символьная строка с командой
//  char rsp[];
// firmware revision number (ATI3)
// copyright notice (ATI4)
//  } kltm_sCom;

#define PRINT_LINE_SIZE 16

int	ttyfd; // порт модема

struct termios saved_attributes; // Use this variable to remember original terminal attributes
struct termios settings;

char modem_msg[80];
char modem_rsp[80];

GW_Iface *kltm_iface; // указатель на структуру параметров интерфейса

///=== KLTM_H private functions

///----------------------------------------------------------------------------
// условно конструктор
int  init_kltm_h()
  {
  int i;
  	
  // проверка идентификатора интерфейса
  if( ((kltm_port > GATEWAY_P8 ) && (kltm_port < GATEWAY_T01)) ||
       (kltm_port > GATEWAY_T32)
    ) return 1;

  if(kltm_port<=GATEWAY_P8) {
    i=kltm_port;
    kltm_iface=&IfaceRTU[i];
  } else {
    i=kltm_port - GATEWAY_T01;
    kltm_iface=&IfaceTCP[i];
    }

  // проверка режима работы интерфейса
  if(!(
       kltm_iface->modbus_mode==IFACE_RTUMASTER ||
       kltm_iface->modbus_mode==IFACE_TCPMASTER
    )) return 2;

  kltm_PLC=kltm_iface->PQueryIndex[0];
  // проверка таблицы опроса интерфейса
  if( (kltm_iface->PQueryIndex[MAX_QUERY_ENTRIES] != 1)  ||
      (PQuery[kltm_PLC].mbf!=MBF_READ_HOLDING_REGISTERS) ||
      (PQuery[kltm_PLC].access!=QT_ACCESS_READONLY)      ||
      (PQuery[kltm_PLC].length!=KLTM_SCOM_STRUCT_SIZE)   ) return 3;

  // запись в таблице виртуальных устройств существует
  for(kltm_CM=0; kltm_CM<MAX_VIRTUAL_SLAVES; kltm_CM++)
    if(VSlave[kltm_CM].iface==kltm_port) break;
  if(kltm_CM==MAX_VIRTUAL_SLAVES) return 4;

  // проверка таблицы виртуальных устройств интерфейса
    if( (VSlave[kltm_CM].device != PQuery[kltm_PLC].device) ||
        (VSlave[kltm_CM].length != PQuery[kltm_PLC].length) ||
        (VSlave[kltm_CM].modbus_table!=HOLDING_REGISTER_TABLE) ) return 5;

  return 0;
  }

///----------------------------------------------------------------------------
// получить от КЛТМ запрос (актуальное значение переменной PLC)
int kltm_getPLC(kltm_sCom *plc)
  {
  static int i;
  
  // проверка бита наличия связи с КЛТМ
  if(PQuery[kltm_PLC].status_bit == 0) return 1;
  
  pthread_mutex_lock(&kltm_iface->serial_mutex);
  
  plc->R_PAC_NUM  = MoxaDevice.wData4x[PQuery[kltm_PLC].offset + 0] & 0x00ff; // номер пакета принятого
  plc->T_PAC_NUM  = MoxaDevice.wData4x[PQuery[kltm_PLC].offset + 0] >>     8; // номер пакета отправленного
  plc->T_PAC_LEN  = MoxaDevice.wData4x[PQuery[kltm_PLC].offset + 1] & 0x00ff; // актуальная длина блока данных в поле T_DATA
  plc->T_PAC_CODE = MoxaDevice.wData4x[PQuery[kltm_PLC].offset + 1] >>     8; // код пакета (операции)

  //!!! здесь реализовывать дополнительные проверки на корректность данных в принятой переменной PLC

  if(plc->T_PAC_LEN > KLTM_DATA_FIELD_SIZE) {
    pthread_mutex_unlock(&kltm_iface->serial_mutex);
  	return 2;
    }

  // чтение массива с данными
  for(i=0; i<plc->T_PAC_LEN; i++)
    plc->T_DATA[i] = (i%2)==0 ?
      MoxaDevice.wData4x[PQuery[kltm_PLC].offset + 2 + i/2] & 0x00ff :
      MoxaDevice.wData4x[PQuery[kltm_PLC].offset + 2 + i/2] >>     8 ;
  
  pthread_mutex_unlock(&kltm_iface->serial_mutex);
  
  return 0;
  }
  	
///----------------------------------------------------------------------------
// записать в  КЛТМ ответ  (новое значение переменной CM)
int kltm_setCM (kltm_sCom *cm)
  {
  static int i;

  if(cm->T_PAC_LEN > KLTM_DATA_FIELD_SIZE) return 1;

  Client[kltm_client].out_len = cm->T_PAC_LEN+4 + TCPADU_FUNCTION;
  make_tcp_adu(Client[kltm_client].out_adu,
	             Client[kltm_client].out_len);
  Client[kltm_client].out_len += TCPADU_ADDRESS;

//Client[kltm_client].out_adu[TCPADU_ADDRESS ] = VSlave[kltm_CM].device      ;   // 6
  Client[kltm_client].out_adu[TCPADU_FUNCTION] = MBF_WRITE_MULTIPLE_REGISTERS;   // 7

  Client[kltm_client].out_adu[TCPADU_START_HI] = VSlave[kltm_CM].start >>   8;   // 8
  Client[kltm_client].out_adu[TCPADU_START_LO] = VSlave[kltm_CM].start & 0xff;   // 9
//  printf("kltm_setCM: %d:[%X][%X]\n", VSlave[kltm_CM].start,
//                                      Client[kltm_client].out_adu[TCPADU_START_HI],
//  	                                  Client[kltm_client].out_adu[TCPADU_START_LO]);

  Client[kltm_client].out_adu[TCPADU_LEN_HI  ] = (cm->T_PAC_LEN+4 + 1)/2 >>   8; // 10
  Client[kltm_client].out_adu[TCPADU_LEN_LO  ] = (cm->T_PAC_LEN+4 + 1)/2 & 0xff; // 11
  Client[kltm_client].out_adu[TCPADU_BYTECOUNT] = cm->T_PAC_LEN+4;               // 12

  // запись массива с данными
  Client[kltm_client].out_adu[TCPADU_BYTECOUNT+2]= cm->R_PAC_NUM;
  Client[kltm_client].out_adu[TCPADU_BYTECOUNT+1]= cm->T_PAC_NUM;
  Client[kltm_client].out_adu[TCPADU_BYTECOUNT+4]= cm->T_PAC_LEN;
  Client[kltm_client].out_adu[TCPADU_BYTECOUNT+3]= cm->T_PAC_CODE;

  for(i=0; i<cm->T_PAC_LEN; i++)
    Client[kltm_client].out_adu[TCPADU_BYTECOUNT+5+i] = i%2==0 ? cm->T_DATA[i+1] : cm->T_DATA[i-1];

  if(i%2!=0) {
  	Client[kltm_client].out_adu[TCPADU_BYTECOUNT+5+i] = cm->T_DATA[i-1];
  	Client[kltm_client].out_adu[TCPADU_BYTECOUNT+5+i-1] = 0;
    Client[kltm_client].out_len++;
    }
  
  /// ставим запрос в очередь MASTER-интерфейса
  if(enqueue_query_ex(&kltm_iface->queue,
	                    kltm_client,
	                    (FRWD_TYPE_REGISTER<<8)|(kltm_CM&0xff),
	                    Client[kltm_client].out_adu,
	                    Client[kltm_client].out_len)!=0) return 2;

  return 0;
  }

///----------------------------------------------------------------------------
int init_modem()
  {

  ttyfd = open(DEVICE_NAME, O_RDWR | O_NOCTTY); // O_NDELAY: The O_NDELAY flag tells UNIX that this program doesn't care what state the DCD signal line is in
  if(ttyfd < 0 ) {
    printf("unable to open modem@%s, error code %d\n", DEVICE_NAME, errno);
    return 1;
    }

//  int	itmp=RS232_MODE;
//  if(ioctl(ttyfd, MOXA_SET_OP_MODE, &itmp ) < 0 ) {
//    printf("MOXA_SET_OP_MODE failed, error code %d\n", errno);
//    return EXIT_FAILURE;
//    }

  tcgetattr(ttyfd, &saved_attributes); // Save the terminal attributes so we can restore them later
  tcgetattr(ttyfd, &settings);

  //# A bit mask specifying flags for input modes
  settings.c_iflag |= INPCK;  // If this bit is set, input parity checking is enabled
  settings.c_iflag |= IGNPAR; // If this bit is set, any byte with a framing or parity error is ignored
  settings.c_iflag &=~ISTRIP; // If this bit is set, valid input bytes are stripped to seven bits
  settings.c_iflag |= IGNBRK; // If this bit is set, break conditions are ignored
  settings.c_iflag &=~IXOFF;  // If this bit is set, start/stop control on input is enabled
  settings.c_iflag &=~IXON;   // If this bit is set, start/stop control on output is enabled
  settings.c_iflag &=~IXANY;
  //# A bit mask specifying flags for output modes
  settings.c_oflag &=~OPOST;  // If this bit is set, output data is processed in some unspecified way
  //# A bit mask specifying flags for control modes
  settings.c_cflag |= CLOCAL; // If this bit is set, it indicates that the modem status lines should be ignored
  settings.c_cflag |= CREAD;  // If this bit is set, input can be read from the terminal. Otherwise, input is discarded when it arrives.
  settings.c_cflag &=~CSTOPB; // If this bit is set, two stop bits are used. Otherwise, only one stop bit is used.
  settings.c_cflag |= PARENB; // If this bit is set, generation and detection of a parity bit are enabled
  settings.c_cflag &=~PARODD; // This bit is only useful if PARENB is set. If PARODD is set, odd parity is used, otherwise even parity is used
  settings.c_cflag &=~CSIZE;  // This is a mask for the number of bits per character
  settings.c_cflag |= CS8;    // This specifies eight bits per byte
  settings.c_cflag &=~CRTSCTS;// Enable RTS/CTS (hardware) flow control
  //# A bit mask specifying flags for local modes
  settings.c_lflag &=~ ICANON; // This bit, if set, enables canonical input processing mode
  settings.c_lflag &=~ ECHO;   // If this bit is set, echoing of input characters back to the terminal is enabled
  settings.c_lflag &=~ ECHOE;
  settings.c_lflag &=~ ISIG;   // This bit controls whether the INTR, QUIT, and SUSP characters are recognized
  settings.c_lflag &=~IEXTEN;  // Enable implementation-defined input processing
  //# An array specifying which characters are associated with various control functions
  settings.c_cc[VMIN]  = 0; // AT\n\n\nOK\n\n
  settings.c_cc[VTIME] = 40;

  cfsetispeed(&settings, B1200); // This function stores speed in *termios-p as the input speed
  cfsetospeed(&settings, B1200); // This function stores speed in *termios-p as the output speed

  tcflush(ttyfd, TCIOFLUSH);
  if(tcsetattr(ttyfd, TCSANOW, &settings) < 0 ) {
    printf("tcsetattr failed, error code %d\n", errno);
    return 2;
    }
  		
  printf("succesfully opened modem@%s\n", DEVICE_NAME);

  return 0;
  }

///----------------------------------------------------------------------------
void show_modem_lines()
  {
  int status, i, m;

  ioctl(ttyfd, TIOCMGET, &status);

  /* modem lines */
  for(i=0; i<16; i++) {

    m=1<<i;
    switch(m) {
      case TIOCM_LE:  // 0x001
        if((status&m)==0) printf("  0 "); else printf("DSR ");
        break;
      case TIOCM_DTR: // 0x002
        if((status&m)==0) printf("  0 "); else printf("DTR ");
        break;
      case TIOCM_RTS: // 0x004
        if((status&m)==0) printf("  0 "); else printf("RTS ");
        break;
      case TIOCM_ST:  // 0x008
        if((status&m)==0) printf("  0 "); else printf("STX ");
        break;
      case TIOCM_SR:  // 0x010
        if((status&m)==0) printf("  0 "); else printf("SRX ");
        break;
      case TIOCM_CTS: // 0x020
        if((status&m)==0) printf("  0 "); else printf("CTS ");
        break;
      case TIOCM_CAR: // 0x040
        if((status&m)==0) printf("  0 "); else printf("DCD ");
        break;
      case TIOCM_RNG: // 0x080
        if((status&m)==0) printf("  0 "); else printf("RNG ");
        break;
      case TIOCM_DSR: // 0x100
        if((status&m)==0) printf("  0 "); else printf("DSR ");
        break;
//      case TIOCM_CD:  // TIOCM_CAR
//      case TIOCM_RI:  // TIOCM_RNG
//      case TIOCM_OUT1: // 0x2000
//      case TIOCM_OUT2: // 0x4000
//      case TIOCM_LOOP: // 0x8000
      default:
        if((status&m)==0) printf("  0 "); else printf("-1- ");
      }

    if((i+1)%4==0) printf("\n");
    }
  
  return;
  }
///----------------------------------------------------------------------------
void print_buffer(unsigned char *buf, unsigned char len)
  {
  unsigned char i, j=0;
  
  while(j*PRINT_LINE_SIZE < len) {

    printf("%3.3d: ", j*PRINT_LINE_SIZE);

    for(i=0; i<PRINT_LINE_SIZE; i++) {
    	if(j*PRINT_LINE_SIZE+i <  len) printf("%2.2X ", buf[j*PRINT_LINE_SIZE+i]); else printf("   ");
    	if((i+1)%8 == 0) printf("|");
    	}

    for(i=0; i<PRINT_LINE_SIZE; i++)
    	if(j*PRINT_LINE_SIZE+i<len) {
    	  if(buf[j*PRINT_LINE_SIZE+i]=='\n') printf("."); else printf("%c", buf[j*PRINT_LINE_SIZE+i]);
    	  } else printf(" ");

    printf("\n");

    j++;
    }
  
  return;
  }
///----------------------------------------------------------------------------
int set_rts(int val)
  {
	static int i=0;
  int status;

/*Constant Description 
TIOCM_LE   DSR (data set ready/line enable) 
TIOCM_DTR  DTR (data terminal ready) 
TIOCM_RTS  RTS (request to send) 
TIOCM_ST   Secondary TXD (transmit) 
TIOCM_SR   Secondary RXD (receive) 
TIOCM_CTS  CTS (clear to send) 
TIOCM_CAR  DCD (data carrier detect) 
TIOCM_CD   Synonym for TIOCM_CAR 
TIOCM_RNG  RNG (ring) 
TIOCM_RI   Synonym for TIOCM_RNG 
TIOCM_DSR  DSR (data set ready) */

  ioctl(ttyfd, TIOCMGET, &status);
  status = ++i%2==0 ? status & ~TIOCM_RTS : status | TIOCM_RTS;
  ioctl(ttyfd, TIOCMSET, &status);
  
  printf("RTS: %d\n", status & TIOCM_RTS);
  show_modem_lines();

  return 0;
  }

///----------------------------------------------------------------------------
int close_modem()
  {

  tcsetattr(ttyfd, TCSANOW, &saved_attributes);
  close(ttyfd);

  return 0;
  }
///----------------------------------------------------------------------------
int send_AT(char *at)
  {
  int len, i;
  
  // ATFY0B6C2R2S10=255@L1
  // sprintf(modem_msg, "AT\r");
  len = strlen(at);
  if(len<3 || (
     (at[0]!='A' && at[1]!='T') &&
     (at[0]!='+' && at[1]!='+' && at[2]!='+')
     )) return 1;
     
  tcflush(ttyfd, TCIOFLUSH);

  if(at[0]=='+' && at[1]=='+' && at[2]=='+') {
  	write(ttyfd, &at[0], 1);
  	sleep(1);
  	write(ttyfd, &at[1], 1);
  	sleep(1);
  	write(ttyfd, &at[2], 1);
    } else write(ttyfd, at, len);

/*  len = read(ttyfd, modem_rsp, 79);

  if(len>=2) {
  	modem_rsp[79]=0;
    //for(i=0; i<len; i++) printf("{%c}", modem_rsp[i]);
  	//printf("\n");
    print_buffer(modem_rsp, len);
    } else printf("init_modem: error\n");

  if (len<0) printf("init_modem: read error occurred\n");*/
  
  return 0;
  }
///----------------------------------------------------------------------------
int get_modem_data(char *at)
  {
  int len, i;
  
  // ATFY0B6C2R2S10=255@L1
  // sprintf(modem_msg, "AT\r");
/*  len = strlen(at);
  if(len<3 || (at[0]!='A' && at[1]!='T')) return 1;
  
  write(ttyfd, at, len);*/

  len = read(ttyfd, modem_rsp, 79);

  if(len>0) {
  	modem_rsp[79]=0;
    //for(i=0; i<len; i++) printf("{%c}", modem_rsp[i]);
  	//printf("\n");
    print_buffer(modem_rsp, len);
    } else if(len==0) printf("get_modem_data: timeout\n");

  if (len<0) printf("get_modem_data: read error occurred\n");
  
  return 0;
  }
///----------------------------------------------------------------------------
