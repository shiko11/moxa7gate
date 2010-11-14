/*
MODBUS GATEWAY SOFTWARE
MOXA7GATE VERSION 1.2
SEM-ENGINEERING
					BRYANSK 2010
*/

#ifndef CLIENTS_H
#define CLIENTS_H

// состо€ние клиентского соединени€
#define MB_CONNECTION_CLOSED			1
#define MB_CONNECTION_ESTABLISHED	2
#define MB_CONNECTION_ERROR				3
#define MB_SLAVE_NOT_DEFINED			0xff
#define MB_ADDRESS_NO_SHIFT				0
#define MB_SCAN_RATE_INFINITE			100000

/*--- ¬сего возможны 4 различных типа клиентов шлюза:
	1.  лиент на стороне TCP, подключенный к порту GATEWAY_SIMPLE;
	2.  лиент на стороне TCP, подключенный к шлюзу через общий TCP порт (обычно 502);
	3.  лиент на стороне RTU, подключенный к порту BRIDGE_PROXY;
	4. HMI клиент;
*/

typedef struct { // параметры клиентского устройства
	u8        mb_slave;					// адрес modbus-устройства дл€ перенаправлени€ запросов в режиме BRIDGE_SIMPLE
	unsigned int ip;						
	unsigned int port;
	int address_shift;
	u8        p_num;						// номер порта, соотнесенного с этой записью
	char device_name[DEVICE_NAME_LENGTH]; // наименование устройства

	u8        c_num;						// пор€дковый номер, 0..MAX_TCP_CLIENTS_PER_PORT-1
	time_t	connection_time;		// врем€ подключени€
	time_t	last_activity_time;		// врем€ последнего запроса

	int connection_status;	// состо€ние TCP-соединени€
	int csd;								// TCP-сокет прин€того или созданного соединени€

	int rc;									// результат функции создани€ потока (только режим GATEWAY_SIMPLE)
	pthread_t	tid_srvr; 		// идентификатор потока в системе (только режим GATEWAY_SIMPLE)

	GW_StaticData stat;
	
	} GW_Client;

#endif  /* CLIENTS_H */
