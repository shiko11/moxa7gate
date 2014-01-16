/************************************************************
ОАО НПО Спецэлектромеханика Брянск 2014г.
Библиотека для работы с протоколом Элеси-МЭК 3


********************************************************** */
#include <stdio.h>  /* Standard input/output */
#include <stdlib.h> /* Standard library definitions */
#include <errno.h>  /* Error definitions */
#include <string.h>


#ifndef _IEC870_3_H_
#define _IEC870_3_H_

//максимальный размер входного буфера от мастера ЛТМ
#define BUF_IN_LEN 255 

//константы для кодов ошибок
#define OK 0
enum ERROR_CODES{ERROR = 1,
		 ERROR_BAD_SERVICE_CODE,       //неправильный код сервесной функции
		 ERROR_CRC_CFL,                //неправильная контрольная сумма у пакета с фиксированной длинны
		 ERROR_CFL_END,                //ошибка - отсутствует завершающий пакет символ
		 ERROR_UNDEFINED_FRAME_TYPE,   //ошибка неизвестный тип пакета
		 ERROR_BAD_SECOND_START_FEILD, //ошибк неверное значение второго стартового поля в пакете переменной длины
		 ERROR_VAR_END,                //ошибка конца пакета переменной длины
		 ERROR_CRC_VAR,                //ошибка контрольной суммы пакета переменной длины
		 ERROR_LEN_VAR,                //ошибка не совпадают поля длина пакета у пакета переменной днлины
		 ERROR_UNKNOWN_COMMAND         //неизвестная команда прешедшая в пакете переменной длины
};

/*Константы для обработки сигналов*/
#define VAR_LEN_FRAME_TYPE      0x68 //кадр переменной длины
#define CONST_LEN_FRAME_TYPE    0x10 //кадр фиксированной длинны
#define ACK_POSITIVE_FRAME_TYPE 0xE5 //кадр положительного подтверждения
#define ACK_NEGATIVE_FRAME_TYPE 0xA2 //кадр отрицательного подтверждения
#define END_OF_FRAME            0x16 //символ конца пакета

#define PRM_MASK 0x40 //маска бита первичного сообщения
#define FCB_MASK 0x20 //маска бита-счетчика кадров
#define FCV_MASK 0x10 //маска бита-действия счетчика кадров
#define DFC_MASK 0x20 //маска для бита потока контроля данных 1- след сообщение может вызвать переполнение буфера 0 - все ОК
#define ADC_MASK 0x40 /*маска бита требование доступа.
Предусмотрено 2 класса сообщений передающих данные называемые класс 1 и 2;
0 - запрос на передачу данных 2-го класса;
1 - запрос на передачу данных 1-го класса.*/

//сервисные функкции первичной станции
#define SERVICE_CODE_MASK             0xF //маска для поля сервисной процедуры
#define SERVICE_CODE_RESET_CONECTION    0 //сброс удаленной связи
#define SERVICE_CODE_RESER_PROCES       1 //сброс пользовательского процесса
#define SERVICE_CODE_TS                 3 //передача с подтверждением
#define SERVICE_CODE_TS_NO_ACK          4 //передача без подтверждения
#define SERVICE_CODE_ACD_NEED           8 //ожидается ответ с интересующим битом ACD
#define SERVICE_CODE_STATE_CONECTION    9 //запроса статуса связи
#define SERVICE_CODE_DATA1_NEED        10 //запрос данных первой категории
#define SERVICE_CODE_DATA2_NEED        11 //запрос данных второй категории
//сервисные функции вторичной станции
#define SERVICE_CODE_ACK             0 //положительное подтверждение
#define SERVICE_CODE_NACK            1 //отрицательное подтверждение
#define SERVICE_CODE_USER_DATA       8 //данные пользоватаеля
#define SERVICE_CODE_NO_DATA         9 //ответ: нет запрашиваемых данных
#define SERVICE_CODE_CON_STAT       11 //ответ статус связи или ACD

//константы смещения для пакета с постоянной длинно CONSTANT FRAME LEN -> CFL
#define START_BYTE_CFL     0 //смещение старт байта от начала пакета
#define SERVICE_CODE_CFL   1 //смещение поля с кодом сервисной функции
#define KP_ADDR_CFL        2 //смещение относительно начала пакета 
#define CHECKSUM_CLF       3 //смещение контрольной суммы
#define END_OF_CFL         4 /*смещение байта конца пакета
 лежит адрес КП для пакета с постоянной длинной*/ 
#define CONSTANT_FRAME_LEN 5 //Длина пакета постоянной длинны  

#define SHORT_ACK_LEN      1 //Длина пакета котроткого потверждения

//константы смещения для пакета с переменной  длинной
#define START_BYTE1_VAR      0 //смещение первого старт байта от начала пакета
#define START_BYTE2_VAR      3 //смещение второго старт байта от начала пакета
#define SIZE_LOW_VAR         1 //смещение младшего поля длины пакета от начала пакета
#define SIZE_HIGH_VAR        2 //смещение старшего поля длины пакета от начала пакета
#define SERVICE_CODE_VAR     4 //смещение поля с кодом сервисной функции
#define KP_ADDR_VAR          5 //смещение поля адреса от начала пакета 
#define COMMAND_FELD         6 //смещение поля с кодом функции
#define ADDR_LOW_BYTE        7 //смещение поля c младшим адресом опрашиваемого сигнала
#define ADDR_HIGT_BYTE       8 //смещение поля со старшим адресом опрашиваемого сигнала
#define ADD_SIZE_VAR         6 //добавка к размеру пакета хранимого в поле SIZE для получения полной длины пакета
#define MAX_VAR_FRAME_LEN   41 // максимальная длина пакета переменной длины
#define SUB_CRC_VAR          2 // Отрицательное смещение от конца пакета перменной длины для нахождения контрольной суммы
#define ADD_CRC_VAR          0 // Относительное смещение от коца блока данных для  контрольной суммы
#define ADD_END_VAR          1 // Относительное смещение от коца блока данных для  признака конца пакета
#define DATA_VAR             6 // смещение начала блока данных
#define CRC_END_LEN          2 /* Совместная длина полей с контрольной суммой и завершающим символом,
				  требуется для вычиления полной расчетной длины пакета перменной длины*/

//Константы для смещения внутри пакета переменной длины для информационных объектов
#define SIGNAL_TYPE_FELD     0 // Смещение поля с типом сигнала относительно другого поля
#define UNDEF_TYPE_FELD      1 //Семещение неопознанного пока поля 
#define ADDRSIG_LOW_BYTE     2 //смещение поля c младшим адресом передоваемого сигнала
#define ADDRSIG_HIGT_BYTE    3 //смещение поля со старшим адресом передоваемого сигнала
#define SIG_TZ               4 //смещение для значения сигнала
#define SIG_TZ_TI4_1         1  //мещение полей значяения сигнала относительно старшего байта адреса!!!
#define SIG_TZ_TI4_2         2
#define SIG_TZ_TI4_3         3
#define SIG_TZ_TI4_4         4
#define PR_FELD              1 //смещение признака для всех сигналов  относительно последнего байта значения!!!


//размеры полей с данными
#define ADD_PACK_SIZE        6 /*Размер на которой надо увеличить сумму  полей данных чтобы получить  
его полную (истиную) длину без поля контрольной суммы и завершающего символа!!!!*/
#define SUB_PACK_SIZE        4 /*Насолько надо уменьшить размер сумму всех полей пакета
без признака коца и котрольной суммы, чтобы получить значение поля размер пакета*/
#define ADD_END_OF_PACK      2 /*Размер группы конец пакета - поле с контрольной суммой и признак крнца пакета*/
#define MAX_SIGN_SIZE       13 //максимальный размер поля с сигналом в пакете
#define SIZE_TI4_WITH_PR     9 //размер поля ТИ4 с признаком 
#define TYPE_FELD_SIZE       1 //размер поля тип передоваемых данных
#define UNDEF_FELD_SIZE      1 //размер не идентифицированного пока поля
#define ADR_FELD_SIZE        2 //размер поля с адресом сигнала
//Размер того что следует перед данными: тип пиредаваемых данных, адрес объекта, неидентифицированное пока поле
#define BEGIN_DATA_ADD       TYPE_FELD_SIZE + UNDEF_FELD_SIZE + ADR_FELD_SIZE 

//константы для сигналов от мастера ЛТМ
#define GENERAL_POLL  0x49 //общий опрос
#define SIGNAL_POLL   0x43  //опрос сигнала
#define TELECONTROL   0x55  //опрос сигнала
#define TR4_TYPE      0x65  //опрос сигнала

#define kp_ALL 255 //адрес КП для общего опроса

/*Константы для различных типов передоваемых данных*/
#define TYPE_TI4                         0x07
#define TYPE_TI4_WITH_TIME               0x0B
#define TYPE_TI4_WITH_STAT               0x08
#define TYPE_TI4_WITH_STAT_AND_TIME      0x0C
#define TYPE_TI2                         0x07
#define TYPE_TI2_WITH_TIME               0x0B
#define TYPE_TI2_WITH_STAT               0x08
#define TYPE_TI2_WITH_STAT_AND_TIME      0x0C
#define TYPE_TI1                         0x07
#define TYPE_TI1_WITH_TIME               0x0B
#define TYPE_TI1_WITH_STAT               0x08
#define TYPE_TI1_WITH_STAT_AND_TIME      0x0C
#define TYPE_TS                          0x07
#define TYPE_TS_WITH_TIME                0x0B
#define TYPE_TS_WITH_STAT                0x08
#define TYPE_TS_WITH_STAT_AND_TIME       0x0C

//Множество настроек для типов подтверждения
#define KP_CFG_ACK_MASK  0x0001
#define KP_CFG_LONG_ACK  0x0001
#define KP_CFG_SHORT_ACK 0x0000
#define KP_CFG_INIT      0x0008
//Состояние бита FCB 
#define KP_CFG_FCB_MASK 0x0002
//битт статуса сигнала общий опрос
#define KP_CFG_GENERAL_POLL  0x0004

/*Тип данных для хранения параметров КП*/
typedef struct{
  unsigned int addr;           //тут все понятно это адрес
  unsigned int state;          //слово состояние данного КП
  unsigned char addr_sig_low;  //младший адрес сигнала
  unsigned char addr_sig_higt; //старгий адрес сигнала
  unsigned char tz[4];         //текущее значение сигнала
    }kp_cfg_t;

#define MAX_IEC_OUT_BUF_LEN       44 //максимальный возможный размер выходного буфера
#define MAX_IEC_OUT_ALL_BUF_LEN 1024 //Максимальный размер общего буфера для передоваемых данны
#define FIST_PACK                  0//номер первого пакета в буфере

/*Для работы требуется два буфера в первом хрянятся все посылки, во втором, храниться, толлько отс
лаемая информация*/
/*Тип данных для выходного буфера обощего, куда складываются все сигналы*/
typedef struct{
  unsigned char data[MAX_IEC_OUT_ALL_BUF_LEN];//собственно буфер и зачем нужны такие коментарии?
  unsigned int len[MAX_IEC_OUT_ALL_BUF_LEN]; //размер iго элемента буфера 
  unsigned int num; //количество элементов в буфера
  unsigned int begin; //текущая позиция в буфере
  unsigned int end;   //конец буфера
  unsigned int len_begin; //Да я извращенец, в структуре два кольцевых буфера
  unsigned int len_end;   //один хранит данные, другой их длину
}iec3_buf_data_send_t;
iec3_buf_data_send_t iec3_buf; //здесь скалдируются данные на передачу

typedef struct{
  unsigned char data[MAX_IEC_OUT_BUF_LEN];//собственно буфер и зачем нужны такие коментарии?
  int len; //вершина этого буфера
}iec3_buf_out_t;
iec3_buf_out_t iec3_out_buf;

/*Функция дешифровки пакетов по МЭК3*/
int read_iec_data(char *in_buf, int in_len, kp_cfg_t *kp_cfg);

//функция разбора пакета постоянной длинны
int  analize_const_frame(char *in_buf, int in_len, kp_cfg_t *kp_cfg);

/*функция разбора пакета переменной длинны
in_buf - ссылка на буфер с входными данными
in_len - размер данных в буфере
out_buf - ссылка на структуру масива выходных данных
kp_cfg - ссылка на структуру с конфигурацией КП
pack_len - ссылка на переменную в которую будет записана длина обработанного пакета из буфера*/
int  analize_var_frame(char *in_buf, int in_len, kp_cfg_t *kp_cfg, int *pack_len);

/*функция разбора поля управления
in_buf - ссылка на контрольное поле в масиве
code - сюда будет записан код сервисной функции*/
int  analize_control_feld(char control_feld, kp_cfg_t *kp_cfg, int *code);

/*Функция заталкивающая в выходной буфер подтверждающий кадр*/
void send_ACK(kp_cfg_t *kp_cfg); 

/*Функция заталкивающая в выходной буфер отрицательного потверждения*/
void send_NACK(kp_cfg_t *kp_cfg);

/*Функция заталкивающая в выходной буфер кадр с информацией об обсутсвии данных*/
void send_no_data(kp_cfg_t *kp_cfg);

/*Функция записи в большой буфер передачи*/
void iec3_buf_wrt(unsigned char *data, unsigned int len);

/*Функция "очишающая" буфер на передачу*/
void clear_send_buf();

/*Функция записывающая в буфер ответ со статусом связи*/
void write_state_connection(kp_cfg_t *kp_cfg);

/*Функция помещает в буфер данные подлежащие передачи*/
void iec3_send_data(kp_cfg_t *kp_cfg);

/*Функция перекладывающая из большого буфера данных на передачу в малый*/
void wrt_out_buf();

/*Из название понятно что эта хрень делает, для тех кто не догадался - забивает буферы нулями*/
void iec3_init();

/*Функция преобразующая данные из КЛТМ (ПЛК) к виду, требуемому для их передачи с записью в буфер*/
unsigned int iec3_convert_plc2kltm(char *plc_data, unsigned int len, kp_cfg_t *kp_cfg);

#endif
