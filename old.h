#define	 NOP_CNT							50		//число пропущеных циклов чтения у-ва при ошибки связи с ним


#define   MAX_MOXA_REG             10000 //максимальное число виртуальных 4х регистров МОХА (пока так, а вообще по усмотрению)
#define   MAX_MAP_POINT            1024 //максимальное число читаемых точек (в принципе их должно быть меньше чем регистров, по усмотрению)

//типы регистров
#define   INPUT_REGISTERS          "3x"
#define   HOLDING_REGISTERS        "4x"

#define	MB_TCP_MAX_SEND_ADU_LEN			1024		/* максимальная длина пакета-запроса посылаемого slave устройству в байтах  */

//параметры статуса для параметра --debug-idata
u16                 _moxa_status_port;
u16                 _moxa_status_start;
u16                 _moxa_status_count;
//параметры статуса для параметра --debug-odata
u16                 _moxa_4x_start;
u16                 _moxa_4x_count;

