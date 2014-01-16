/******************************************************
Библиотека для работы с последовательным портом содержит
в себе функции чтения записи и настройки параметров
обмена по последовательному порту
*******************************************************/
#include <fcntl.h>  /* File control definitions */
#include <stdio.h>  /* Standard input/output */
#include <stdlib.h> /* Standard library definitions */
#include <termio.h> /* POSIX terminal control definitions */
#include <errno.h>  /* Error definitions */
#include <unistd.h> /* POSIX Symbolic Constants */
#include <string.h>
#include <moxadevice.h>

#ifndef _SERIAL_H
#define _SERIAL_H

#define MAX_COM_PORTS 4 //максимальное кол-во портов на плате
#define MAX_COM_RX_BUF_SIZE 1024 //максимальный размер входного буффера

enum PORTS_NUMBERS{ COM1 = 0, COM2, COM3, COM4, COM5, COM6, COM7, COM8};
enum PARTY{NONE = 0, EVEN, ODD};
enum FLOW_CONTROL{CTRCTS = 1, IXONOOFF, BOTH};

            
#define COMP1  "/dev/ttyM0"
#define COMP2  "/dev/ttyM1"
#define COMP3  "/dev/ttyM2"
#define COMP4  "/dev/ttyM3"
#define COMP5  "/dev/ttyM4"
#define COMP6  "/dev/ttyM5"
#define COMP7  "/dev/ttyM6"
#define COMP8  "/dev/ttyM8"

//тип для хранения параметров порта
typedef struct {
  int baudrate;
  int party;
  int mode;
  int flow_control;
} port_options_t;
                        
/*Функции работы с последовательным портом*/
/*Открытие порта в качестве параметра 
номер порта - 1, 2, 3... и до MAX_SERIAL_PORTS 5*/
int open_port(int port);

/*Установка параметров обмена по данному порту*/
int set_port_options(int port, port_options_t *options);

/*Запись данных из буффера в порт*/
int port_write(int port, char *buf, int buf_len);

/*Чтение данных из порта*/
int port_read(int port, char *buf, int buf_len);

/*закрытие порта*/
int  close_port(int port);

/*Прочитать текущие настройки порта*/
void read_port_options(int port, struct termios *settings);

/*Установить скорость*/
void set_baudrate(struct termios *setting, int baudrate);

/*Установить четность*/
void set_party(struct termios *setting, int party);

/*Установить режим управления потоком*/
void set_flow_control(struct termios *setting, int party);

/*Установить режим*/
void set_port_mode(int port, int mode);
#endif

