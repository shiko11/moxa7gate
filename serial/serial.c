#include "serial.h"

/*Открытие порта в качестве параметра 
номер порта - 1, 2, 3... и до MAX_COM_PORTS*/
int open_port(int port){
  char const *COM_PORTS[] = {COMP1, COMP2, COMP3, COMP4,COMP5, COMP6, COMP7, COMP8};
  if(port > MAX_COM_PORTS)
    return -1;
  else{
    #ifdef DEBUG
        printf("Открываю порт: %s\n", COM_PORTS[port]);
    #endif
    return open(COM_PORTS[port], O_RDWR | O_NOCTTY);
  }
};
/*Установка параметров обмена по данному порту*/
int set_port_options(int port, port_options_t *options){
  struct termios settings;
  //читаем текущие настройки порта
  read_port_options(port, &settings);
  /*
    c_iflag - флаги констант: 
    IGNBRK - игнорировать режим BREAK при вводе. 
    BRKINT - если включен IGNBRK то BREAK игнорируется. Если он не включен, а BRKINT включен, то BREAK вызывает сброс очередей, и, если терминал является управляющим для группы процессов переднего плана, то группе будет отослан сигнал SIGINT. Если ни IGNBRK ни BRKINT не включены, то BREAK считывается как нулевой символ, кроме случая когда установлено PARMRK, тогда он будет считан, как последовательность \377 \0 \0. 
    IGNPAR - игнорировать ошибки четности и позиционирования. 
    PARMRK - если не включен режим IGNPAR, то сопровождать символ с ошибкой четности или позиционирования префиксом \377 \0. Если не включен ни IGNPAR, ни PARMRK, то считывать символ с ошибкой четности или позиционирования как \0. 
    INPCK - запускать проверку четности при вводе. 
    ISTRIP - удалять восьмой бит. 
    INLCR - преобразовывать NL в CR при вводе. 
    IGNCR - игнорировать перевод каретки при вводе. 
    ICRNL -преобразовывать перевод каретки в конец строки при вводе (пока не будет запущен IGNCR). 
    IUCLC - (не включено в POSIX) отображать символы верхнего регистра в нижнем регистре при вводе. 
    IXON - запустить управление потоком данных XON/XOFF при выводе. 
    IXANY - (не включено в POSIX.1; XSI) позволить любому символу заново запускать вывод. 
    IXOFF - запустить управление потоком данных XON/XOFF при вводе. 
    IMAXBEL - (не включено в POSIX) выдавать звуковой сигнал, когда очередь ввода переполнится. Linux не использует этот бит и действует так. будто он всегда включен. 

Константы для флага c_oflag, включенные в POSIX.1: 
    OPOST - включить режим вывода, определяемый реализацией по умолчанию. 
Остальные константы для флага c_oflag определены в POSIX 1003.1-2001 (если явно не указано иначе). 
    OLCUC - (не включено в POSIX) отображать символы нижнего регистра в верхнем регистре при выводе. 
    ONLCR - (XSI) преобразовывать NL в CR-NL при выводе. 
    OCRNL - преобразовывать CR в NL при выводе. 
    ONOCR - не выводить CR в столбце 0. 
    ONLRET - не выводить CR. 
    OFILL - посылать символы заполнения для задержки, а не использовать синхронизированную задержку. 
    OFDEL - (не включено в POSIX) установить символ заполнения ASCII DEL (0177). Иначе установить символ заполнения ASCII NUL. 
    NLDLY - маска задержки конца строки. Значениями будут NL0 и NL1. 
    CRDLY - маска задержки перевода каретки. Значениями будут: CR0, CR1, CR2 или CR3. 
    TABDLY - маска задержки горизонтальной табуляции. Значениями будут: TAB0, TAB1, TAB2, TAB3 (или XTABS). Значение TAB3 или, иначе, XTABS "расширяет" табуляцию в пробелы (с символами табуляции в каждой восьмой позиции). 
    BSDLY - маска задержки символа забоя. Значением будет BS0 или BS1. (Никогда не была реализована.) 
    VTDLY - маска задержки вертикальной табуляции. Значением будет VT0 или VT1. 
    FFDLY - маска задержки прокрутки страницы. Значением будет FF0 или FF1. 

Константы для флага c_cflag: 
    CBAUD - (не включено в POSIX) маска скорости в бодах (4+1 бита). 
    CBAUDEX - (не включено в POSIX) дополнительная маска скорости в бодах (1 бит), включена в CBAUD. 
    (В стандарте POSIX указано, что скорость в бодах хранится в структуре termios, без указания ее точного местонахождения, и предоставляет cfgetispeed() и cgfsetispeed() для получения доступа к ней. Некоторые системы используют биты, выбираемые CBAUD в c_cflag, другие используют отдельные поля, например sg_ispeed и sg_ospeed.) 
    CSIZE - маска размера символов. Значениями будут: CS5, CS6, CS7 или CS8. 
    CSTOPB - устанавливать два стоповых бита вместо одного. 
    CREAD - включить прием. 
    PARENB - запустить генерацию четности при выводе и проверку четности на вводе. 
    PARODD - установить нечетность на вводе и выводе. 
    HUPCL - выключить управление модемом линиями после того, как последний процесс прекратил использование устройства (повесить трубку). 
    CLOCAL - игнорировать управление линиями с помощью модема. 
    LOBLK - (не включено в POSIX) блокировать вывод с нетекущего уровня оболочки. (Для использования с shl.) 
    CIBAUD - (не включено в POSIX) маска для скоростей ввода. Для битов CIBAUD используются те же значения, что и для битов CBAUD, но они смещены влево на ISHIFT битов. 
    CRTSCTS - (не включено в POSIX) разрешить управление потоком данных RTS/CTS (аппаратное). 

Константы для флага c_lflag: 
    ISIG - когда принимаются любые символы из INTR, QUIT, SUSP или DSUSP, то генерировать соответствующий сигнал. 
    ICANON - запустить канонический режим. Это означает, что линии используют специальные символы: EOF, EOL, EOL2, ERASE, KILL, LNEXT, REPRINT, STATUS и WERASE, а также строчную буферизацию. 
    XCASE - (не включено в POSIX; не поддерживается в Linux) если при этом запущен ICANON, то терминал работает только с символами в верхнем регистре. Все вводимое конвертируется в символы нижнего регистра, кроме символов, предваряемых \. При выводе предваряемые \ символы в верхнем регистре и символы в нижнем регистре преобразуются в символы вернего регистра. 
    ECHO - отображать вводимые символы. 
    ECHOE - если запущен ICANON, то символ ERASE удалает предыдущий символ, а WERASE удаляет предыдущее слово. 
    ECHOK - если запущен ICANON, то символ KILL удаляет всю текущую строку. 
    ECHONL - если запущен ICANON, то символ NL отображается, даже если режим ECHO не включен. 
    ECHOCTL - (не включено в POSIX) если запущен ECHO, то управляющие сигналы ASCII, отличающиеся от TAB, NL, START и STOP, отображаются как ^X, где X есть символ из таблицы ASCII с кодом на 0x40 больше, чем у управляющего сигнала. Например, символ 0x08 (BS) отобразится как ^H. 
    ECHOPRT - (не включено в POSIX) если включены ICANON и IECHO, то символы печатаются по мере того, как они уничтожаются. 
    ECHOKE - (не включено в POSIX) если ICANON также включен, то KILL обозначается как уничтожение каждого символа в строке (по спецификации ECHOE и ECHOPRT). 
    DEFECHO  - (не включено в POSIX) выводить только когда процесс считывает. 
    FLUSHO - (не включено в POSIX; не поддерживается в Linux) выводимые данные ускоренно записываются. Этот флаг переключается с помощью набираемого символа DISCARD. 
    NOFLSH - отключить ускоренную запись вводимых и выводимых очередей во время генерации сигналов SIGINT и SIGQUIT и записывать вводимую очередь при генерации сигнала SIGSUSP. 
    TOSTOP - послать сигнал SIGTTOU процессной группе фонового процесса, которая делает попытку записать его на контролирующий терминал. 
    PENDIN - (не включено в POSIX; не поддерживается в Linux) все символы в очереди ввода повторно печатаются, когда считан последующий символ (таким образом bash "перехватывает" предваряющую печать). 
    IEXTEN - включить режим ввода, определяемый реализацией по умолчанию. Этот флаг, как и ICANON должен быть включен для обработки специальных символов EOL2, LNEXT, REPRINT, WERASE, а также для того, чтобы работал флаг IUCLC.
   */
  
  //Параметры приема
  //settings.c_iflag |= INPCK;  // Запускать проверку чености при входа
  //settings.c_iflag |= IGNPAR; // Игнорировать ошибки четности
  settings.c_iflag &=~ISTRIP; // Неотрезаем восьмой бит
  settings.c_iflag |= IGNBRK; // Игнорируем режим BREAK при вводе  
  //settings.c_iflag &=~IXOFF;  // Отключаем аппаратное управление потоком при вводе 
  //settings.c_iflag &=~IXON;   // Отключаем аппаратное управление потоком при выводе
  settings.c_iflag &=~IXANY;  // Запрещаем заново запускать ввод по любому символу
 
  //#Параметры передачи
  settings.c_oflag &=~OPOST;  // Отключаем режим сна
  settings.c_cflag |= CLOCAL; // Игнорируе управление линией с помощью модема
  settings.c_cflag |= CREAD;  // Читаем с терминала
  settings.c_cflag &=~CSTOPB; // Один стоп-бит использовать здесь будем
  //settings.c_cflag |= PARENB; // Будем передовать с битом контроля четности 
  //settings.c_cflag &=~PARODD; // Снимаем бит проверки на нечетность, при этом будет осуществляться проверка на четность при включенном PARENB
  settings.c_cflag &=~CSIZE;  // Очищаем маску размера передоваемого символа
  settings.c_cflag |= CS8;    // Устанавливаем маску передоваемого символа в 8бит
  //settings.c_cflag &=~CRTSCTS;// Отключаем аппаратное управление линиями CRT и CTS*/
  
  //# Общие флаги управления передачей
  settings.c_lflag &=~ ICANON; // Ненадо нам канонического режима
  settings.c_lflag &=~ ECHO;   // И Эхо нам ненадо
  settings.c_lflag &=~ ECHOE;  // удаляем там всякие символы в каноническом режиме :)
  settings.c_lflag &=~ ISIG;   // Негенерировать сигнал при принятии символов  INTR, QUIT, and SUSP 
  settings.c_lflag &=~IEXTEN;  // Отключаем режим ввода определяемый реализацией по умолчанию
  
  //Блокировки на чтение не происходит, таймаут не ставим, минимальное количество принятых симовлов - 0
  settings.c_cc[VMIN]  = 0;
  settings.c_cc[VTIME] = 0;

  tcflush(port, TCIOFLUSH);

  //Настраиваем четность
  set_party(&settings, options->party);
  //Настраиваем контроль управления перечей
  set_flow_control(&settings, options->flow_control);
  //Устанавливаем скорость передачи
  set_baudrate(&settings, options->baudrate);
  //Устанавливаем режим работы порта
  set_port_mode(port, RS232_MODE);

  //Установка скорости передачи данных 
  //cfsetispeed(&settings, B115200); // This function stores speed in *termios-p as the input speed
  //cfsetospeed(&settings, B115200); // This function stores speed in *termios-p as the output speed  
  return tcsetattr(port, TCSANOW, &settings);
};

/*Запись данных из буффера в порт*/
int port_write(int port, char *buf, int buf_len){
   return  write(port, buf, buf_len);
};

/*Чтение данных из порта*/
int port_read(int port, char *buf, int buf_len){
  int num_read_bytes = 0;
  num_read_bytes = read(port, buf, buf_len);
  if(num_read_bytes){
    *(buf + num_read_bytes) = 0;
  }
  return num_read_bytes;
};

/*закрытие порта*/
int close_port(int port){
  return close(port);
};

/*Прочитать текущие настройки порта*/
void read_port_options(int port, struct termios *settings){
  tcgetattr(port, settings);
};

/*Установить скорость*/
void set_baudrate(struct termios *settings, int baudrate ){
  speed_t speed;
  switch(baudrate){
  case 300:
      speed = B300;
    break;
  case 600:
      speed = B600;
    break;
  case 1200:
      speed = B1200;
    break;
  case 2400:
      speed = B2400;
    break;
  case 4800:
      speed = B4800;
    break;
  case 9600:
      speed = B9600;
    break;
  case 19200:
      speed = B19200;
    break;
  case 38400:
      speed = B38400;
    break;
  case 57600:
      speed = B57600;
    break;
  case 115200:
      speed = B115200;
    break;
  case 230400:
      speed = B230400;
    break;
  case 460800:
      speed = B460800;
    break;
  case 921600:
      speed = B921600;
    break;
  default:
    speed = B115200;
    break;
  }
  #ifdef DEBUG
      printf("Желаемая скорость обмена: %d\n", baudrate);
      printf("Установлена следубщая скорость обмена: %d\n", speed);
  #endif
  cfsetispeed(settings, speed); 
  cfsetospeed(settings, speed);
};

/*Установить четность*/
void set_party(struct termios *settings, int  party){
  
  switch(party){
  case NONE:
    #ifdef DEBUG
        printf("Контроль четности: НЕТ\n");
    #endif
    settings->c_iflag &= ~INPCK;
    settings->c_cflag &= ~PARENB;
    break;
  case EVEN:
    #ifdef DEBUG
        printf("Контроль четности: ЧЕТНОСТЬ\n");
    #endif
    settings->c_iflag |= INPCK;
    settings->c_cflag |= PARENB;
    settings->c_cflag &=~PARODD;
    break;
  case ODD:
    #ifdef DEBUG
        printf("Контроль четности: НЕЧЕТНОСТЬ\n");
    #endif
    settings->c_iflag |= INPCK;
    settings->c_cflag |= PARENB;
    settings->c_cflag |= PARODD;
    break;
  default:
    #ifdef DEBUG
        printf("Контроль четности: НЕТ по умолчанию\n");
    #endif
    settings->c_iflag &= ~INPCK;
    settings->c_cflag |= PARENB;
    break;
  }
};

//Умтановка параметров управления потоком
void set_flow_control(struct termios *settings, int  flow){
  switch(flow){
  case NONE:
    #ifdef DEBUG
    printf("Управление потоком: ОТКЛЮЧЕНО\n");
    #endif
    settings->c_iflag &=~IXOFF;
    settings->c_iflag &=~IXON;
    settings->c_cflag &=~CRTSCTS;
    break;
  case CTRCTS:
    #ifdef DEBUG
    printf("Управление потоком: ВКЛЮЧЕНО АППАРАТНОЕ\n");
    #endif
    settings->c_cflag |=CRTSCTS;
    break;
  case IXONOOFF:
      #ifdef DEBUG
           printf("Управление потоком: ВКЛЮЧЕНО ПРОГРАММНОЕ\n");
      #endif
      settings->c_iflag |=IXOFF;
      settings->c_iflag |=IXON;
      break;
  case BOTH:
      #ifdef DEBUG
          printf("Управление потоком: ВКЛЮЧЕНО ПРОГРАММНОЕ и АППАРАТНОЕ\n");
      #endif
      settings->c_iflag |=IXOFF;
      settings->c_iflag |=IXON;
      settings->c_cflag |=CRTSCTS;
      break;
  default:
    settings->c_iflag &=~IXOFF;
    settings->c_iflag &=~IXON;
    settings->c_cflag &=~CRTSCTS;
    #ifdef DEBUG
        printf("Управление потоком: ОТКЛЮЧЕНО ПРОГРАММНОЕ и АППАРАТНОЕ по умолчанию\n");
    #endif
    break;
  }
}
/*Установить режим*/
void set_port_mode(int port, int mode){
  mode=RS232_MODE;
  ioctl(port, MOXA_SET_OP_MODE, &mode);
};

