#ifndef QUEUE_H
#define QUEUE_H

#include <sys/sem.h>
#include "modbus.h"

#define   MAX_GATEWAY_QUEUE_LENGTH	16

typedef struct { // очередь на семафорах
	int port_id;

	unsigned char			queue_adu[MAX_GATEWAY_QUEUE_LENGTH][MB_TCP_MAX_ADU_LENGTH];// TCP ADU
	unsigned short		queue_adu_len[MAX_GATEWAY_QUEUE_LENGTH];
	unsigned short		queue_clients[MAX_GATEWAY_QUEUE_LENGTH];
	unsigned short		queue_slaves[MAX_GATEWAY_QUEUE_LENGTH];
	int			queue_start, queue_len;

  pthread_mutex_t queue_mutex;
	struct sembuf operations[1];
	} GW_Queue;

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short int *array;
	struct seminfo *__buf;
  };

int	enqueue_query_ex(GW_Queue *queue, int client_id, int device_id, u8 *adu, u16 adu_len);
int	get_query_from_queue(GW_Queue *queue, int *client_id, int *device_id, u8 *adu, u16 *adu_len);

///=== QUEUE_H public variables
// Набор семафоров. Используется для синхронизации работы потоков с очередями.
int semaphore_id;

#endif  /* QUEUE_H */
