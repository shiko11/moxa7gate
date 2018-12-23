/***********   K M - 4 0 0   *************
      ���������������� ������                                
                              ������ 1.0
      ��� "����"
               ������ 2014                 
*****************************************/

///**** ������ ��� ������ � ���-������� CMX868A, �����������
///**** ����������������� PIC16F886 � ������� �������� 1.0

#include <fcntl.h>  /* File control definitions */
#include <termio.h> /* POSIX terminal control definitions */
#include <errno.h>  /* Error definitions */
#include <unistd.h> /* POSIX Symbolic Constants */

#include <moxadevice.h>

#include "cmx868a.h"

// ���������������� ���� ��� ����� � �������
#define CMX868A_TTY "/dev/ttyM3"
// ������ ������������ ����������� ������
#define CMX868A_FWVER "EF8681 V3.2"

// ������� ������� ������
#define MAX_PACK_LEN  254
#define MAX_ATMSG_LEN 64

#define MAX_ATRES_CNT 4
#define MAX_ATRES_LEN 16

// �������� ������� ����� ��������� ESCAPE-������������������, �����
#define ESC_GUARD_TIME 60000
// �������� ������� �� ��������� AT-�������, �����
#define ATCMD_PROC_TIME 27000
// ������� �������� ������ �� ���������������� ������, 10E-1 ���
#define PIC_TIMEOUT 1
// ������� �������� ������� �� ������-����������� ���� ���, 10E-1 ���
#define LTM_TIMEOUT 12

// ��������� �������� ��������� "������� �������� �� ��"
#define CMX868A_GAIN00DB  7
#define CMX868A_GAIN15DB  6
#define CMX868A_GAIN30DB  5
#define CMX868A_GAIN45DB  4
#define CMX868A_GAIN60DB  3
#define CMX868A_GAIN75DB  2
#define CMX868A_GAIN90DB  1
#define CMX868A_GAIN105DB 0

#define CMX868A_GAINMASK  7

// ���������� � �������� ������ �������� � ������:
// - ����������� ��������,
// - 8 ��� ������,
// - �������� �������� EVEN,
// - ���� �������� ���
#define S25ASYNC8E1 112
#define S26ASYNC8E1 184

// ���� ������� ������
#define	ATRES_OK         0
#define	ATRES_CONNECT    1
#define	ATRES_RING       2
#define	ATRES_NOCARRIER  3
#define	ATRES_ERROR      4
#define	ATRES_NODIALTONE 5
#define	ATRES_BUSY       6
#define	ATRES_CON2400    7
#define	ATRES_CON1200    8
#define	ATRES_CON600     9
#define	ATRES_CON300     10
#define	ATRES_CON120075  11
#define	ATRES_CON751200  12
#define ATRES_CON1200150 13
#define ATRES_CON1501200 14
#define ATRES_NYI        15
#define ATRES_NORESULT   16

// ���� ��� ������ � GPIO
#define EM1240_GPIO_MODDO 0
#define EM1240_GPIO_MODDI 1
#define EM1240_GPIO_VALLO 0
#define EM1240_GPIO_VALHI 1

#define CMX868A_GPIO_GAIN12DB 0

///=== CMX868A_H private variables

unsigned char cmx868a_state; // ���� ��������� ������

int	ttyfd; // �������� ���������� ����� ������

unsigned char s25, s26; // S-��������, ���������� �� ����� ������ ������

struct termios saved_attributes; // Use this variable to remember original terminal attributes
struct termios settings;         // ���������� � ����������� ��������� ����������������� ����� ������

char modem_msg[MAX_ATMSG_LEN]; // ������ � AT-�������� ��� �������� ������
char modem_rsp[MAX_ATMSG_LEN]; // ������ � ������� �� AT-������� �� ������

char atres[MAX_ATRES_CNT][MAX_ATRES_LEN]; // ��������� ������� ������ � ������� ������

struct timeval tv;

///=== CMX868A_H private functions

// int atcmd(unsigned char *cmd);       // �������� AT-������� ���������������� ������
// int atrsp(unsigned char *rsp);       // ������ ���������� ���������� AT-�������

// ����������� ��� �������

#ifdef CMX868A_DEBUG

#define PRINT_LINE_SIZE 16

char bufmark; // ����� ������ ��� ������ �� ����� �������� print_buffer

void print_buffer(unsigned char *buf,
                  unsigned char len); // ����� �� ����� ����������� ������ ������
void show_modem_lines();              // ����� �� ����� RS-232 �������� ������

#endif /* CMX868A_DEBUG */

///----------------------------------------------------------------------------
int cmx868a_init()                        // ������������� ���������� ������, ����������� � ������
  {
  int	itmp;

  ttyfd = open(CMX868A_TTY, O_RDWR | O_NOCTTY); // O_NDELAY: Use non-blocking I/O. On some systems this also means the RS232 DCD signal line is ignored.
  if(ttyfd < 0 ) {
#ifdef CMX868A_DEBUG
    printf("unable to open modem@%s, error code %d\n", CMX868A_TTY, errno);
#endif
    return CMX868A_ERR_NOTTY;
    }

  itmp=RS232_MODE;
  if(ioctl(ttyfd, MOXA_SET_OP_MODE, &itmp ) < 0 ) {
#ifdef CMX868A_DEBUG
    printf("MOXA_SET_OP_MODE failed, error code %d\n", errno);
#endif
    return CMX868A_ERR_NOTTY;
    }

  tcgetattr(ttyfd, &saved_attributes); // Save the terminal attributes so we can restore them later
  tcgetattr(ttyfd, &settings);

  //# A bit mask specifying flags for input modes
  settings.c_iflag &=~INPCK;  // If this bit is set, input parity checking is enabled
  settings.c_iflag &=~IGNPAR; // If this bit is set, any byte with a framing or parity error is ignored
  settings.c_iflag &=~ISTRIP; // If this bit is set, valid input bytes are stripped to seven bits
  settings.c_iflag |= IGNBRK; // If this bit is set, break conditions are ignored
  settings.c_iflag &=~IXOFF;  // If this bit is set, start/stop control on input is enabled
  settings.c_iflag &=~IXON;   // If this bit is set, start/stop control on output is enabled
  settings.c_iflag &=~IXANY;  // Allow any character to start flow again
  //# A bit mask specifying flags for output modes
  settings.c_oflag &=~OPOST;  // If this bit is set, output data is processed in some unspecified way
  //# A bit mask specifying flags for control modes
  settings.c_cflag |= CLOCAL; // If this bit is set, it indicates that the modem status lines should be ignored
  settings.c_cflag |= CREAD;  // If this bit is set, input can be read from the terminal. Otherwise, input is discarded when it arrives.
  settings.c_cflag &=~CSTOPB; // If this bit is set, two stop bits are used. Otherwise, only one stop bit is used.
  settings.c_cflag &=~PARENB; // If this bit is set, generation and detection of a parity bit are enabled
  settings.c_cflag &=~PARODD; // This bit is only useful if PARENB is set. If PARODD is set, odd parity is used, otherwise even parity is used
  settings.c_cflag &=~CSIZE;  // This is a mask for the number of bits per character
  settings.c_cflag |= CS8;    // This specifies eight bits per byte
  settings.c_cflag |= CRTSCTS;// Enable RTS/CTS (hardware) flow control
  //# A bit mask specifying flags for local modes
  settings.c_lflag &=~ICANON; // This bit, if set, enables canonical input processing mode
  settings.c_lflag &=~ECHO;   // If this bit is set, echoing of input characters back to the terminal is enabled
  settings.c_lflag &=~ECHOE;
  settings.c_lflag &=~ISIG;   // This bit controls whether the INTR, QUIT, and SUSP characters are recognized
  settings.c_lflag &=~IEXTEN;  // Enable implementation-defined input processing
  //# An array specifying which characters are associated with various control functions
  settings.c_cc[VMIN]  = 0;
  settings.c_cc[VTIME] = LTM_TIMEOUT;

  cfsetispeed(&settings, B19200); // This function stores speed in *termios-p as the input speed
  cfsetospeed(&settings, B19200); // This function stores speed in *termios-p as the output speed

  tcflush(ttyfd, TCIOFLUSH);
  if(tcsetattr(ttyfd, TCSANOW, &settings) < 0 ) {
#ifdef CMX868A_DEBUG
    printf("tcsetattr failed, error code %d\n", errno);
#endif
    return CMX868A_ERR_NOTTY;
    }
  		
#ifdef CMX868A_DEBUG
  printf("succesfully opened modem@%s\n", CMX868A_TTY);
#endif

  // --to configure GPIO ports to be DI ports or DO ports
  set_gpio_mode(CMX868A_GPIO_GAIN12DB, EM1240_GPIO_MODDO);

  // --to set the current status of the GPIO, either high or low voltage,
  // used when the GPIO points are configured as DO
  set_gpio_data(CMX868A_GPIO_GAIN12DB, EM1240_GPIO_VALLO);

  cmx868a_state = CMX868A_STATE_INIT;

  return CMX868A_OK;
  }

///----------------------------------------------------------------------------
int cmx868a_reset(unsigned char leader,   // �����/������������� ������, ����� ������ ������ �� ��
	                unsigned char baudrate, // �����/������������� ������, �������� ������ �� ��
	                unsigned char gain)     // �����/������������� ������, ������� �������� �� ��
  {
  int i, j, g;

  if(cmx868a_state != CMX868A_STATE_INIT     &&
     cmx868a_state != CMX868A_STATE_ATMODE   &&
     cmx868a_state != CMX868A_STATE_DATAXFER &&
     cmx868a_state != CMX868A_STATE_POWERSAVE) return CMX868A_ERR_NOINIT;

  settings.c_cc[VMIN]  = 0;
  settings.c_cc[VTIME] = PIC_TIMEOUT;

  if(tcsetattr(ttyfd, TCSANOW, &settings) < 0 ) {
#ifdef CMX868A_DEBUG
    printf("tcsetattr failed, error code %d\n", errno);
#endif
    return CMX868A_ERR_NOTTY;
    }

// �������� ������� ����� � ����������������� PIC16F886, ����� ��� ��������

// ������� ������ � ����� AT-������ ����������� �������� DTR

  strcpy(modem_msg, "+++");
  atcmd(modem_msg);
  tv.tv_sec=0; tv.tv_usec=ATCMD_PROC_TIME;
  select(0, NULL, NULL, NULL, &tv);

  j=atrsp(modem_rsp); // �������, ���� ����� ���������������, �� �� ��������� � ������ ��
#ifdef CMX868A_DEBUG
  for(i=0; i<j; i++)
    if(strlen(atres[i])<MAX_ATRES_LEN)
      printf("\nprm %d: %s", i, atres[i]);
#endif
  if(j>0) { // ������� �����
  	i=strlen(atres[j-1]);
    // ���� ��� ATRES_OK, �� ����� ��� ��������� � ������ ��
  	g = (atres[j-1][i-1]==0x30 &&
  	     atres[j-1][i-2]==0x30) ? 1 : 0;
    }

  strcpy(modem_msg, "ATZ0E0V0I0\r");
  atcmd(modem_msg);
  tv.tv_sec=0; tv.tv_usec=ATCMD_PROC_TIME;
  select(0, NULL, NULL, NULL, &tv);

  j=atrsp(modem_rsp);
#ifdef CMX868A_DEBUG
  for(i=0; i<j; i++)
    if(strlen(atres[i])<MAX_ATRES_LEN)
      printf("\nprm %d: %s", i, atres[i]);
#endif

  if(j==2) { // ����� ���������������
  	i=0;
    } else if(j==3) { // ����� �� ���������������
      i=1;
      }else { // ������� ��� ������ ������
#ifdef CMX868A_DEBUG
        printf("\nno pic");
#endif
        return CMX868A_ERR_NOPIC;
        };
        
  if(strcmp(CMX868A_FWVER, atres[i])==0 && atoi(atres[i+1])==ATRES_OK) {
#ifdef CMX868A_DEBUG
  	printf("\nfwver: OK, res %d", atoi(atres[i+1]));
#endif
    ;
    } else {
#ifdef CMX868A_DEBUG
      printf("\nfwver: FAIL, res %d", atoi(atres[i+1]));
#endif
      return CMX868A_ERR_NOPIC;
      }

  // �������� � ���� ����� ������������ �������� ����������,
  // ���� ����� ������������ � ��������� �������� ������
  if(g==1) {
#ifdef CMX868A_DEBUG
  	printf("\nsleep");
#endif
  	sleep(1);
    }

  cmx868a_state = CMX868A_STATE_ATMODE;

  // �������� ������ � ������-������������ ����� ���� �����:
  //   0 - ������ ����� �� ������������ ��� ����� � �������� ���
  //   1 - ������
  //   2 - 1200 ���/� �� V.23 ��� ����
  //   3 - 2400 ���/� �� V.22bis ��� ����
  
  if(baudrate==0) {
  	cmx868a_sleep(); // ������� ������ � �������� �����
  	return CMX868A_OK;
    }

  if(baudrate!=2) return CMX868A_ERR_SETT;

  // ���� "����� ������ ������ �� ��" ... ��� �������� ����� 50 (����� � ��������� 10..127)
  if(leader<10 || leader>127) return CMX868A_ERR_SETT;

  // ���� "������� �������� �� ��" �������� �������� ������ �������� �� ������ ��.
  // ��������� �������� ����� ����� ����� � ��������� 0..31, ��� ������������� ������ 0..-31 dB.
  // ������������� �������� 10, �.�. -10 dB.
  // �������������� ����������� ������ �������� � ��������� 0..-22.5 dB � ����� 1.5  dB.
								 
  s25 = S25ASYNC8E1 | CMX868A_GAIN105DB;
  s26 = S26ASYNC8E1 | CMX868A_GAIN105DB;

  if(gain<=31) {
    g=(10*gain+5)/15;
    if(g>15) g=15;
    // S-registers values
    s25 = (s25 & ~CMX868A_GAINMASK) | (7-(g%8));
    s26 = (s26 & ~CMX868A_GAINMASK) | (7-(g%8));
    // gpio 0 value
    set_gpio_data(CMX868A_GPIO_GAIN12DB, g/8);
    } else return CMX868A_ERR_SETT;

// ����������� � ���� ��������� �������

  // ������ ������������� ������
  sprintf(modem_msg, "ATS6=0S7=2B2S25=%dS26=%dD0\r", s25, s26);
  atcmd(modem_msg);
  tv.tv_sec=3; tv.tv_usec=ATCMD_PROC_TIME;
  select(0, NULL, NULL, NULL, &tv);

  j=atrsp(modem_rsp);
#ifdef CMX868A_DEBUG
  for(i=0; i<j; i++)
    if(strlen(atres[i])<MAX_ATRES_LEN)
      printf("\nprm %d: %s", i, atres[i]);
#endif

  if(j==2 &&
     atoi(atres[0])==ATRES_NORESULT &&
     atoi(atres[1])==ATRES_NOCARRIER) { // ��� �������
  	printf("\nNO CARRIER");
  	return CMX868A_NOANSW;
    } else if(j==2 &&
              atoi(atres[0])==ATRES_NORESULT &&
              atoi(atres[1])==ATRES_CON1200) { // ���������� �����������
  	  printf("\nCONNECT 1200");
      }else { // ������� ��� ������ ������
#ifdef CMX868A_DEBUG
        printf("\nno pic");
#endif
        return CMX868A_ERR_NOPIC;
        };
        
  settings.c_cc[VMIN ] = 0;
  settings.c_cc[VTIME] = LTM_TIMEOUT;

  tcflush(ttyfd, TCIOFLUSH);
  if(tcsetattr(ttyfd, TCSANOW, &settings) < 0 ) {
#ifdef CMX868A_DEBUG
    printf("tcsetattr failed, error code %d\n", errno);
#endif
    return CMX868A_ERR_NOTTY;
    }

  cmx868a_state = CMX868A_STATE_DATAXFER;

  return CMX868A_OK;
  }

///----------------------------------------------------------------------------
int atcmd(char *cmd)  // �������� AT-������� ���������������� ������
  {
  int len, i;
  
  len = strlen(cmd);
  if(len<3 || (
     (cmd[0]!='A' && cmd[1]!='T') &&
     (cmd[0]!='+' && cmd[1]!='+' && cmd[2]!='+')
     )) return 1;
     
  tcflush(ttyfd, TCIOFLUSH);

  if(cmd[0]=='+' && cmd[1]=='+' && cmd[2]=='+') {
  	write(ttyfd, &cmd[0], 1);
    tv.tv_sec=0; tv.tv_usec=ESC_GUARD_TIME;
    select(0, NULL, NULL, NULL, &tv);
  	write(ttyfd, &cmd[1], 1);
    tv.tv_sec=0; tv.tv_usec=ESC_GUARD_TIME;
    select(0, NULL, NULL, NULL, &tv);
  	write(ttyfd, &cmd[2], 1);
    } else write(ttyfd, cmd, len);

  return 0;
  }
///----------------------------------------------------------------------------
int atrsp(char *rsp)  // ������ ���������� ���������� AT-�������
  {
  int i, j, k, len;
  
  len = read(ttyfd, rsp, MAX_ATMSG_LEN);

  if(len>0) {

#ifdef CMX868A_DEBUG
    bufmark='<';
    print_buffer(rsp, len);
#endif

    // ������ ������ � �������
    j=k=0;
    for(i=0; i<len; i++) {

      if(rsp[i]==0x0a) {
        if(k==0) continue;
        if(k<MAX_ATRES_LEN) atres[j][k  ]=0;
          else              atres[j][k-1]=0;
        k=0;
        j++;
        if(j>=MAX_ATRES_CNT) break;
        continue;
        }

      if(k<MAX_ATRES_LEN) atres[j][k++]=rsp[i];
      }
    
    if(len==3 && rsp[0]=='+' && rsp[1]=='+' && rsp[2]=='+') {atres[j][k  ]=0; j++;}

    } else if(len==0) { // �������
#ifdef CMX868A_DEBUG
    	printf("\nget_modem_data: timeout");
#endif
      j=0;
      } else if(len<0) { // ������ ������
#ifdef CMX868A_DEBUG
  	printf("\nget_modem_data: read error");
#endif
        j=len;
        }

#ifdef CMX868A_DEBUG
  show_modem_lines();
#endif

  return j;
  }
///----------------------------------------------------------------------------
int cmx868a_test(unsigned char code)      // ���������/���������� ��������� (��������) ������� ������
  {
  
  return 0;
  }

///----------------------------------------------------------------------------
int cmx868a_sleep()                       // ������� ������ � ����� ����������������
  {

  return 0;
  }

///----------------------------------------------------------------------------
int cmx868a_close()                      // ���������� ������ � �������, ����� ������, ������������ ��������
  {

  // ���� ����� ��������������� � � ������, ����� ������

//  sleep(1);
//  strcpy(modem_msg, "+++");
//  send_AT(modem_msg);
//  sleep(1);
//  strcpy(modem_msg, "ATZ\r");
//  send_AT(modem_msg);
//  sleep(1);
//  get_modem_data(modem_rsp);  

  tcsetattr(ttyfd, TCSANOW, &saved_attributes);
  close(ttyfd);

  return 0;
  }
///----------------------------------------------------------------------------

int cmx868a_recv_packet(unsigned char *packet,
                        unsigned char *length)  // ������� ���� �� ������ ��
  {
  int len, i, pck, var, rcv, timeout, errcntr, skipped;
  unsigned char byte;
  
  if(cmx868a_state != CMX868A_STATE_DATAXFER) return CMX868A_ERR_NOTRDY;

  *length = i = pck = rcv = timeout = errcntr = skipped = 0;

  while(rcv==0) { // ��������� ����, ���� �� �������� ����� �����

    len = read(ttyfd, &byte, 1); // ������ ��������� ����

//    printf("\nlen=%d, byte=%2.2X", len, byte);

    if(len==0) { // ���� �������, 1200 ����
#ifdef CMX868A_DEBUG
      printf("\ncmx868a_recv_packet: timeout");
#endif
      pck=0;
      timeout++;
      // �������� ������� DCD ��� ������������� ����������� � ��������� �������
      if(timeout>=8) return CMX868A_ERR_TIMEOUT;
      }

    if(pck==0 && i!=0) { // ����� ���������� ����� ���������� �������
#ifdef CMX868A_DEBUG
      printf("\ncmx868a_recv_packet: errcntr");
#endif
      i=0;
      errcntr++;
      if(errcntr>=3) return CMX868A_ERR_COUNTER;
      }
      
    if(skipped>MAX_PACK_LEN) { // ���� � ������������� ����� �� ������
      return CMX868A_ERR_COUNTER;
      }

    if(len<0) { // ���� ������ ������
#ifdef CMX868A_DEBUG
    	printf("\ncmx868a_recv_packet: read error");
#endif
      pck=0;
      return CMX868A_ERR_SYSFAIL;
      }

    if(len>0) { // ������

    	if(pck==0 && (byte==0x10 || byte==0x68)) { // ������ �����
        i=0;
    		pck=1;
    		var = byte==0x68 ? 1 : 0;
    		packet[i++]=byte;
    		continue;
    		}

      if(pck==1 && var==0) { // ����������� ���� ���������� �����

        if(i==4) { // ��������� ���� �����
          if(byte==0x16) rcv=1; // ����� ����� ��������
            else pck=0;         // ������ ������� �����
          }

        packet[i++]=byte;
    		continue;
        }

      if(pck==1 && var==1) { // ����������� ���� ���������� �����

        if(i==3) { // ��������� ���� ��������� �����
          if(byte==0x68 && packet[1]==packet[2]) ; // ��������� ����� ������ ��� ������
            else pck=0;  // ������ ������� �����
          }

        if(i==(5+packet[1])) { // ��������� ���� �����
          if(byte==0x16) rcv=1; // ����� ����� ��������
            else pck=0;       // ������ ������� �����
          }

        packet[i++]=byte;
    		continue;
        }

      // �������� ���� �������������
      skipped++;

      } // ������
    
    } // ��������� ����, ���� �� �������� ����� �����

#ifdef CMX868A_DEBUG
  bufmark='<';
//  print_buffer(packet, i);

//  show_modem_lines();
#endif

  *length=i;

  // �������� �� �������� ������ ����� ��������� �������
  tv.tv_sec=0; tv.tv_usec=12000; // �����������  ��������, ����������� ������� �����
//tv.tv_sec=1; tv.tv_usec=1000; // ������������ ��������, ����������� ������� �����
  select(0, NULL, NULL, NULL, &tv);

  return CMX868A_OK;
  }
///----------------------------------------------------------------------------
int cmx868a_send_packet(unsigned char *packet,
                        unsigned char length)   // ��������� ���� � ����� ��
  {

  write(ttyfd, packet, length);

#ifdef CMX868A_DEBUG
  bufmark='>';
//  print_buffer(packet, length);

//  show_modem_lines();
#endif

  return CMX868A_OK;
  }
///----------------------------------------------------------------------------

#ifdef CMX868A_DEBUG

///----------------------------------------------------------------------------
void show_modem_lines()
  {
  int status, i, m;

  ioctl(ttyfd, TIOCMGET, &status);
  printf(" ");

  /* modem lines */
  for(i=0; i<16; i++) {

    m=1<<i;
    switch(m) {
      case TIOCM_LE:  // 0x001
        if((status&m)==0) /*printf("  0 ")*/; else printf("DSR ");
        break;
      case TIOCM_DTR: // 0x002
        if((status&m)==0) /*printf("  0 ")*/; else printf("DTR ");
        break;
      case TIOCM_RTS: // 0x004
        if((status&m)==0) /*printf("  0 ")*/; else printf("RTS ");
        break;
      case TIOCM_ST:  // 0x008
        if((status&m)==0) /*printf("  0 ")*/; else printf("STX ");
        break;
      case TIOCM_SR:  // 0x010
        if((status&m)==0) /*printf("  0 ")*/; else printf("SRX ");
        break;
      case TIOCM_CTS: // 0x020
        if((status&m)==0) /*printf("  0 ")*/; else printf("CTS ");
        break;
      case TIOCM_CAR: // 0x040
        if((status&m)==0) /*printf("  0 ")*/; else printf("DCD ");
        break;
      case TIOCM_RNG: // 0x080
        if((status&m)==0) /*printf("  0 ")*/; else printf("RNG ");
        break;
      case TIOCM_DSR: // 0x100
        if((status&m)==0) /*printf("  0 ")*/; else printf("DSR ");
        break;
//      case TIOCM_CD:  // TIOCM_CAR
//      case TIOCM_RI:  // TIOCM_RNG
//      case TIOCM_OUT1: // 0x2000
//      case TIOCM_OUT2: // 0x4000
//      case TIOCM_LOOP: // 0x8000
      default:
        if((status&m)==0) /*printf("  0 ")*/; else printf("%2.2Xh ", status&m);
      }

//    if((i+1)%4==0) printf("\n");
    }
//  printf("\n");
  
  return;
  }
///----------------------------------------------------------------------------
void print_buffer(unsigned char *buf, unsigned char len)
  {
  unsigned char i, j=0;
  
  while(j*PRINT_LINE_SIZE < len) {

    printf("\n%c%3.3d: ", bufmark, j*PRINT_LINE_SIZE);

    for(i=0; i<PRINT_LINE_SIZE; i++) {
    	if(j*PRINT_LINE_SIZE+i <  len) printf("%2.2X ", buf[j*PRINT_LINE_SIZE+i]); else printf("   ");
    	if((i+1)%8 == 0) printf("|");
    	}

    for(i=0; i<PRINT_LINE_SIZE; i++)
    	if(j*PRINT_LINE_SIZE+i<len) {
    	  if((buf[j*PRINT_LINE_SIZE+i]>=0x30 && buf[j*PRINT_LINE_SIZE+i]<=0x39) ||
           (buf[j*PRINT_LINE_SIZE+i]>=0x41 && buf[j*PRINT_LINE_SIZE+i]<=0x5a) ||
           (buf[j*PRINT_LINE_SIZE+i]>=0x61 && buf[j*PRINT_LINE_SIZE+i]<=0x7a)
           ) printf("%c", buf[j*PRINT_LINE_SIZE+i]);
    	    else printf(".");
    	  } else printf(" ");

//    printf("\n");

    j++;
    }
  
  return;
  }
///----------------------------------------------------------------------------

#endif /* CMX868A_DEBUG */
