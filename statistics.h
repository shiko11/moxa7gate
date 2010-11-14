#ifndef STATISTICS_H
#define STATISTICS_H

///************* М Е Х А Н И З М   С Б О Р А   С Т А Т И С Т И К И ********************

#define MAX_LATENCY_HISTORY_POINTS	8

#define STAT_FUNC_0x01		0
#define STAT_FUNC_0x02		1
#define STAT_FUNC_0x03		2
#define STAT_FUNC_0x04		3
#define STAT_FUNC_0x05		4
#define STAT_FUNC_0x06		5
#define STAT_FUNC_0x0f		6
#define STAT_FUNC_0x10		7
#define STAT_FUNC_OTHER		8
#define STAT_FUNC_AMOUNT	9

#define STAT_RES_OK			0
#define STAT_RES_ERR		1
#define STAT_RES_EXP		2
#define STAT_RES_AMOUNT 3

/// Структура для хранения статистики опроса по порту, клиенту и серверу,
/// она содержит данные как о результатах запросов, так и детализацию по функциям ModBus
typedef struct {

	//--- информация по результатам запросов ---
	unsigned int accepted; // количество принятых запросов

	unsigned int errors;

		unsigned int errors_input_communication;
		unsigned int errors_tcp_adu;
		unsigned int errors_tcp_pdu;
	
		unsigned int errors_serial_sending;
		unsigned int errors_serial_accepting;
	
		unsigned int timeouts;  // количество таймаутов
	
		unsigned int crc_errors; // количество ошибок при проверке контрольной суммы
		unsigned int errors_serial_adu;
		unsigned int errors_serial_pdu;
	
		unsigned int errors_tcp_sending;

	unsigned int sended;
																			 
	//--- детализация по функциям ModBus ---
	unsigned int func[STAT_FUNC_AMOUNT][STAT_RES_AMOUNT];

	//--- временнЫе характеристики опроса ---
	unsigned int request_time;	// время обработки последнего запроса
	unsigned int scan_rate;			// мгновенный период опроса

	///!!!--- нужен отдельный механизм сбора истории и вычисления статистических характеристик опроса
	/// который будет включаться отдельным ключом в конфигурационном скрипте и служить исключительно
	/// для выявления проблем в работе оборудования (подключение БД MySQL было бы здесь интересно)
	// кольцевой буфер результатов N последних запросов
//	unsigned int latency_history[MAX_LATENCY_HISTORY_POINTS]; 
//	unsigned int clp; // указатель на самое старое значение в буфере
	
//	unsigned int request_time_min;			///!!!
//	unsigned int request_time_max;			///!!!
//	unsigned int request_time_average;	///!!!
	
	} GW_StaticData;

void copy_stat(GW_StaticData *dst, GW_StaticData *src);
void update_stat(GW_StaticData *dst, GW_StaticData *src);
void clear_stat(GW_StaticData *dst);
void func_res_ok(int mbf, GW_StaticData *dst);
void func_res_err(int mbf, GW_StaticData *dst);

#endif  /* STATISTICS_H */
