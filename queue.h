/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010                 
***************************************/

#ifndef FRWD_QUEUE_H
#define FRWD_QUEUE_H

///*********  МОДУЛЬ ФУНКЦИЙ ПЕРЕНАПРАВЛЕНИЯ, ОЧЕРЕДЕЙ ЗАПРОСОВ MODBUS *********

#include <sys/sem.h>

#include "modbus.h"

///=== FRWD_QUEUE_H constants

#define   MAX_GATEWAY_QUEUE_LENGTH	16

///=== FRWD_QUEUE_H data types

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

///=== FRWD_QUEUE_H public variables

// Набор семафоров. Используется для синхронизации работы потоков с очередями.
int semaphore_id;

///=== FRWD_QUEUE_H public functions

int init_queue();
int enqueue_query_ex(GW_Queue *queue, int client_id, int device_id, u8 *adu, u16 adu_len);
int get_query_from_queue(GW_Queue *queue, int *client_id, int *device_id, u8 *adu, u16 *adu_len);

#endif  /* FRWD_QUEUE_H */
