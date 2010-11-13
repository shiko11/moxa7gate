/*
MODBUS GATEWAY SOFTWARE
MOXA7GATE VERSION 1.1
SEM-ENGINEERING
					BRYANSK 2010
*/

#ifndef WORKERS_H
#define WORKERS_H

#include <sys/sem.h>

#define   MAX_GATEWAY_QUEUE_LENGTH	16

#define MAX_TCP_CLIENTS_PER_PORT	8
#define MAX_CLIENT_ACTIVITY_TIMEOUT	30
#define DEFAULT_CLIENT 0

#define MB_CONNECTION_CLOSED			1
#define MB_CONNECTION_ESTABLISHED	2
#define MB_CONNECTION_ERROR				3
#define MB_SLAVE_NOT_DEFINED			0xff
#define MB_ADDRESS_NO_SHIFT				0
#define MB_SCAN_RATE_INFINITE			100000

// Набор семафоров. Используется для синхронизации работы потоков с очередями.
int semaphore_id;

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short int *array;
	struct seminfo *__buf;
  };

void *srvr_tcp_child(void *arg); /// Потоковая функция режима GATEWAY_SIMPLE
void *srvr_tcp_child2(void *arg); /// Потоковая функция режимов GATEWAY_ATM, GATEWAY_RTM
void *srvr_tcp_bridge(void *arg); /// Потоковая функция режима BRIDGE_TCP
void *gateway_proxy_thread(void *arg); /// Потоковая функция режима GATEWAY_PROXY
void *bridge_proxy_thread(void *arg); /// Потоковая функция режима BRIDGE_PROXY
void *moxa_mb_thread(void *arg); /// Потоковая функция обработки запросов к MOXA

#endif  /* WORKERS_H */
