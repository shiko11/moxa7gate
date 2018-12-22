#ifndef STATISTICS_H
#define STATISTICS_H

///************* � � � � � � � �   � � � � �   � � � � � � � � � � ********************

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

/// ��������� ��� �������� ���������� ������ �� �����, ������� � �������,
/// ��� �������� ������ ��� � ����������� ��������, ��� � ����������� �� �������� ModBus
typedef struct {

	//--- ���������� �� ����������� �������� ---
	unsigned int accepted; // ���������� �������� ��������

	unsigned int errors;

		unsigned int errors_input_communication;
		unsigned int errors_tcp_adu;
		unsigned int errors_tcp_pdu;
	
		unsigned int errors_serial_sending;
		unsigned int errors_serial_accepting;
	
		unsigned int timeouts;  // ���������� ���������
	
		unsigned int crc_errors; // ���������� ������ ��� �������� ����������� �����
		unsigned int errors_serial_adu;
		unsigned int errors_serial_pdu;
	
		unsigned int errors_tcp_sending;

	unsigned int sended;
																			 
	//--- ����������� �� �������� ModBus ---
	unsigned int func[STAT_FUNC_AMOUNT][STAT_RES_AMOUNT];

	//--- ��������� �������������� ������ ---
	unsigned int request_time;	// ����� ��������� ���������� �������
	unsigned int scan_rate;			// ���������� ������ ������

	///!!!--- ����� ��������� �������� ����� ������� � ���������� �������������� ������������� ������
	/// ������� ����� ���������� ��������� ������ � ���������������� ������� � ������� �������������
	/// ��� ��������� ������� � ������ ������������ (����������� �� MySQL ���� �� ����� ���������)
	// ��������� ����� ����������� N ��������� ��������
//	unsigned int latency_history[MAX_LATENCY_HISTORY_POINTS]; 
//	unsigned int clp; // ��������� �� ����� ������ �������� � ������
	
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
