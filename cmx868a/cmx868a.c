/***********   K M - 4 0 0   *************
      КОММУНИКАЦИОННЫЙ МОДУЛЬ                                
                              ВЕРСИЯ 1.0
      ООО "БЗПА"
               БРЯНСК 2014                 
*****************************************/

///**** МОДУЛЬ ДЛЯ РАБОТЫ С ЧИП-МОДЕМОМ CMX868A, УПРАВЛЯЕМЫМ
///**** МИКРОКОНТРОЛЛЕРОМ PIC16F886 С ВЕРСИЕЙ ПРОШИВКИ 1.0

#include <fcntl.h>  /* File control definitions */
#include <termio.h> /* POSIX terminal control definitions */
#include <errno.h>  /* Error definitions */
#include <unistd.h> /* POSIX Symbolic Constants */

#include <moxadevice.h>

#include "cmx868a.h"

// последовательный порт для связи с модемом
#define CMX868A_TTY "/dev/ttyM3"
// версия программного обеспечения модема
#define CMX868A_FWVER "EF8681 V3.2"

// размеры буферов данных
#define MAX_PACK_LEN  254
#define MAX_ATMSG_LEN 64

#define MAX_ATRES_CNT 4
#define MAX_ATRES_LEN 16

// выдержка времени между символами ESCAPE-последовательности, мксек
#define ESC_GUARD_TIME 60000
// выдержка времени на обработку AT-команды, мксек
#define ATCMD_PROC_TIME 27000
// таймаут ожидания ответа от микроконтроллера модема, 10E-1 сек
#define PIC_TIMEOUT 1
// таймаут ожидания запроса от мастер-контроллера сети ЛТМ, 10E-1 сек
#define LTM_TIMEOUT 12

// возможные значения параметра "уровень передачи по ТЧ"
#define CMX868A_GAIN00DB  7
#define CMX868A_GAIN15DB  6
#define CMX868A_GAIN30DB  5
#define CMX868A_GAIN45DB  4
#define CMX868A_GAIN60DB  3
#define CMX868A_GAIN75DB  2
#define CMX868A_GAIN90DB  1
#define CMX868A_GAIN105DB 0

#define CMX868A_GAINMASK  7

// передатчик и приемник модема работают в режиме:
// - асинхронная передача,
// - 8 бит данных,
// - контроль четности EVEN,
// - один стоповый бит
#define S25ASYNC8E1 112
#define S26ASYNC8E1 184

// коды ответов модема
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

// коды для работы с GPIO
#define EM1240_GPIO_MODDO 0
#define EM1240_GPIO_MODDI 1
#define EM1240_GPIO_VALLO 0
#define EM1240_GPIO_VALHI 1

#define CMX868A_GPIO_GAIN12DB 0

///=== CMX868A_H private variables

unsigned char cmx868a_state; // байт состояния модема

int	ttyfd; // файловый дескриптор порта модема

unsigned char s25, s26; // S-регистры, отвечающие за режим работы модема

struct termios saved_attributes; // Use this variable to remember original terminal attributes
struct termios settings;         // переменная с параметрами настройки последовательного порта модема

char modem_msg[MAX_ATMSG_LEN]; // строка с AT-командой для передачи модему
char modem_rsp[MAX_ATMSG_LEN]; // строка с ответом на AT-команду от модема

char atres[MAX_ATRES_CNT][MAX_ATRES_LEN]; // результат разбора строки с ответом модема

struct timeval tv;

///=== CMX868A_H private functions

// int atcmd(unsigned char *cmd);       // отправка AT-команды микроконтроллеру модема
// int atrsp(unsigned char *rsp);       // чтение результата выполнения AT-команды

// определения для отладки

#ifdef CMX868A_DEBUG

#define PRINT_LINE_SIZE 16

char bufmark; // метка буфера при выводе на экран функцией print_buffer

void print_buffer(unsigned char *buf,
                  unsigned char len); // вывод на экран содержимого буфера данных
void show_modem_lines();              // вывод на экран RS-232 сигналов модема

#endif /* CMX868A_DEBUG */

///----------------------------------------------------------------------------
int cmx868a_init()                        // инициализация переменных модуля, подключение к модему
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
int cmx868a_reset(unsigned char leader,   // сброс/инициализация модема, длина лидера ответа по ТЧ
	                unsigned char baudrate, // сброс/инициализация модема, скорость обмена по ТЧ
	                unsigned char gain)     // сброс/инициализация модема, уровень передачи по ТЧ
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

// проверка наличия связи с микроконтроллером PIC16F886, сброс его настроек

// перевод модема в режим AT-команд производить сигналом DTR

  strcpy(modem_msg, "+++");
  atcmd(modem_msg);
  tv.tv_sec=0; tv.tv_usec=ATCMD_PROC_TIME;
  select(0, NULL, NULL, NULL, &tv);

  j=atrsp(modem_rsp); // таймаут, если модем инициализирован, но не подключен к каналу ТЧ
#ifdef CMX868A_DEBUG
  for(i=0; i<j; i++)
    if(strlen(atres[i])<MAX_ATRES_LEN)
      printf("\nprm %d: %s", i, atres[i]);
#endif
  if(j>0) { // получен ответ
  	i=strlen(atres[j-1]);
    // если код ATRES_OK, то модем был подключен к каналу ТЧ
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

  if(j==2) { // модем инициализирован
  	i=0;
    } else if(j==3) { // модем не инициализирован
      i=1;
      }else { // таймаут или ошибка чтения
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

  // задержка в этом месте обеспечивает успешное соединение,
  // если модем сбрасывается в состоянии передачи данных
  if(g==1) {
#ifdef CMX868A_DEBUG
  	printf("\nsleep");
#endif
  	sleep(1);
    }

  cmx868a_state = CMX868A_STATE_ATMODE;

  // Скорость обмена с мастер-контроллером может быть равна:
  //   0 - данный канал не используется для связи с мастером ЛТМ
  //   1 - резерв
  //   2 - 1200 бит/с по V.23 или ВОЛС
  //   3 - 2400 бит/с по V.22bis или ВОЛС
  
  if(baudrate==0) {
  	cmx868a_sleep(); // перевод модема в дежурный режим
  	return CMX868A_OK;
    }

  if(baudrate!=2) return CMX868A_ERR_SETT;

  // Поле "длина лидера ответа по ТЧ" ... это значение равно 50 (лежит в диапазоне 10..127)
  if(leader<10 || leader>127) return CMX868A_ERR_SETT;

  // Поле "Уровень передачи по ТЧ" содержит значение уровня передачи по каналу ТЧ.
  // Возможное значение этого числа лежит в диапазоне 0..31, что соответствует уровню 0..-31 dB.
  // Рекомендуемое значение 10, т.е. -10 dB.
  // Обеспечивается регулировка уровня передачи в диапазоне 0..-22.5 dB с шагом 1.5  dB.
								 
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

// подключение к сети тональной частоты

  // строка инициализации модема
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
     atoi(atres[1])==ATRES_NOCARRIER) { // нет несущей
  	printf("\nNO CARRIER");
  	return CMX868A_NOANSW;
    } else if(j==2 &&
              atoi(atres[0])==ATRES_NORESULT &&
              atoi(atres[1])==ATRES_CON1200) { // соединение установлено
  	  printf("\nCONNECT 1200");
      }else { // таймаут или ошибка чтения
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
int atcmd(char *cmd)  // отправка AT-команды микроконтроллеру модема
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
int atrsp(char *rsp)  // чтение результата выполнения AT-команды
  {
  int i, j, k, len;
  
  len = read(ttyfd, rsp, MAX_ATMSG_LEN);

  if(len>0) {

#ifdef CMX868A_DEBUG
    bufmark='<';
    print_buffer(rsp, len);
#endif

    // разбор строки с ответом
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

    } else if(len==0) { // таймаут
#ifdef CMX868A_DEBUG
    	printf("\nget_modem_data: timeout");
#endif
      j=0;
      } else if(len<0) { // ошибка чтения
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
int cmx868a_test(unsigned char code)      // включение/отключение сервисной (тестовой) функции модема
  {
  
  return 0;
  }

///----------------------------------------------------------------------------
int cmx868a_sleep()                       // перевод модема в режим энергосбережения
  {

  return 0;
  }

///----------------------------------------------------------------------------
int cmx868a_close()                      // завершение работы с модулем, сброс модема, освобождение ресурсов
  {

  // если модем инициализирован и в работе, сброс модема

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
                        unsigned char *length)  // принять кадр из канала ТЧ
  {
  int len, i, pck, var, rcv, timeout, errcntr, skipped;
  unsigned char byte;
  
  if(cmx868a_state != CMX868A_STATE_DATAXFER) return CMX868A_ERR_NOTRDY;

  *length = i = pck = rcv = timeout = errcntr = skipped = 0;

  while(rcv==0) { // выполнять цикл, пока не завершен прием кадра

    len = read(ttyfd, &byte, 1); // читаем очередной байт

//    printf("\nlen=%d, byte=%2.2X", len, byte);

    if(len==0) { // если таймаут, 1200 мсек
#ifdef CMX868A_DEBUG
      printf("\ncmx868a_recv_packet: timeout");
#endif
      pck=0;
      timeout++;
      // контроль сигнала DCD при необходимости производить в отдельной функции
      if(timeout>=8) return CMX868A_ERR_TIMEOUT;
      }

    if(pck==0 && i!=0) { // прием очередного кадра завершился ошибкой
#ifdef CMX868A_DEBUG
      printf("\ncmx868a_recv_packet: errcntr");
#endif
      i=0;
      errcntr++;
      if(errcntr>=3) return CMX868A_ERR_COUNTER;
      }
      
    if(skipped>MAX_PACK_LEN) { // кадр в установленное время не принят
      return CMX868A_ERR_COUNTER;
      }

    if(len<0) { // если ошибка чтения
#ifdef CMX868A_DEBUG
    	printf("\ncmx868a_recv_packet: read error");
#endif
      pck=0;
      return CMX868A_ERR_SYSFAIL;
      }

    if(len>0) { // разбор

    	if(pck==0 && (byte==0x10 || byte==0x68)) { // начало кадра
        i=0;
    		pck=1;
    		var = byte==0x68 ? 1 : 0;
    		packet[i++]=byte;
    		continue;
    		}

      if(pck==1 && var==0) { // принимается кадр постоянной длины

        if(i==4) { // последний байт кадра
          if(byte==0x16) rcv=1; // прием кадра завершен
            else pck=0;         // ошибка формата кадра
          }

        packet[i++]=byte;
    		continue;
        }

      if(pck==1 && var==1) { // принимается кадр переменной длины

        if(i==3) { // последний байт заголовка кадра
          if(byte==0x68 && packet[1]==packet[2]) ; // заголовок кадра принят без ошибок
            else pck=0;  // ошибка формата кадра
          }

        if(i==(5+packet[1])) { // последний байт кадра
          if(byte==0x16) rcv=1; // прием кадра завершен
            else pck=0;       // ошибка формата кадра
          }

        packet[i++]=byte;
    		continue;
        }

      // принятый байт отбрасывается
      skipped++;

      } // разбор
    
    } // выполнять цикл, пока не завершен прием кадра

#ifdef CMX868A_DEBUG
  bufmark='<';
//  print_buffer(packet, i);

//  show_modem_lines();
#endif

  *length=i;

  // задержка на отправку ответа после получения запроса
  tv.tv_sec=0; tv.tv_usec=12000; // минимальная  задержка, установлено опытным путем
//tv.tv_sec=1; tv.tv_usec=1000; // максимальная задержка, установлено опытным путем
  select(0, NULL, NULL, NULL, &tv);

  return CMX868A_OK;
  }
///----------------------------------------------------------------------------
int cmx868a_send_packet(unsigned char *packet,
                        unsigned char length)   // отправить кадр в канал ТЧ
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
