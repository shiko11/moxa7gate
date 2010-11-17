/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010             
***************************************/

///*********  ÌÎÄÓËÜ ÔÓÍÊÖÈÉ ÏÅÐÅÍÀÏÐÀÂËÅÍÈß, Î×ÅÐÅÄÅÉ ÇÀÏÐÎÑÎÂ MODBUS *********

///=== FRWD_QUEUE_H IMPLEMENTATION

#include "queue.h"
#include "messages.h"
#include "interfaces.h"

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short int *array;
	struct seminfo *__buf;
  };

///-----------------------------------------------------------------------------------------------------------------
int init_queue()
	{

	key_t sem_key=ftok("/tmp/app", 'b');
	
	if((semaphore_id = semget(sem_key, MAX_MOXA_PORTS*2+MAX_TCP_SERVERS, IPC_CREAT|IPC_EXCL|0666)) == -1) {
		sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|EVENT_SRC_SYSTEM, 29, 0, 0, 0, 0);
		return 1;
	 	}
	
	union semun sems;
	unsigned short values[1];

	values[0]=0;
	sems.array = values;
	/* Èíèöèàëèçèðóåì âñå ýëåìåíòû îäíèì çíà÷åíèåì */
	semctl(semaphore_id, 0, SETALL, sems);
	
//		printf("maximal semaphore amount %d\n", SEMMSL);
	return 0;
	}
///-----------------------------------------------------------------------------------------------------------------
int enqueue_query_ex(GW_Queue *queue, int client_id, int device_id, u8 *adu, u16 adu_len)
  {

			  if(queue->queue_len==MAX_GATEWAY_QUEUE_LENGTH) { ///!!! modbus exception response, reset queue
			 		// QUEUE OVERLOADED
			 		sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_WRN|queue->port_id, 149, client_id, 0, 0, 0);
					return 1;
					}
	
		  	pthread_mutex_lock(&queue->queue_mutex);
	
				int j, queue_current=(queue->queue_start+queue->queue_len)%MAX_GATEWAY_QUEUE_LENGTH;
	
//				printf("Queue %d\n", queue->port_id+1);
				for(j=0; j<adu_len; j++) {
					queue->queue_adu[queue_current][j]=adu[j];
//					printf("(%2.2X)", adu[j]);
					}
//				printf("\n");
				queue->queue_adu_len[queue_current]=adu_len;
				queue->queue_clients[queue_current]=client_id;
				queue->queue_slaves[queue_current]=device_id; ///!!! ATM & PROXY uses this field
	
				queue->queue_len++;
	
	///$$$	struct timeval tv1, tv2;
	///$$$	struct timezone tz;
	//(tv2.tv_sec%10)*1000+tv2.tv_usec/1000
	///$$$		gettimeofday(&tv2, &tz);
	///$$$			printf("\nP%d TCP%4.4d time%d query %d begin[%d]", port_id+1, gate502.clients[i].port, (tv2.tv_sec%10)*10+tv2.tv_usec/100000, iDATA[port_id].queue_len, queue_current);
	
	pthread_mutex_unlock(&queue->queue_mutex);
	//printf("mutex passed\n");

//	printf("query_queued %d P%d, len=%d\n", queue_current, queue->port_id+1, queue->queue_len);
	sysmsg_ex(EVENT_CAT_TRAFFIC|EVENT_TYPE_INF|queue->port_id, 220, client_id, queue->queue_len, 0, 0);

	queue->operations[0].sem_op=1;
//	printf("semaphore inc: port=%d\n", queue->port_id+1);
	semop(semaphore_id, queue->operations, 1);

  return 0;
  }
///-----------------------------------------------------------------------------------------------------------------
int get_query_from_queue(GW_Queue *queue, int *client_id, int *device_id, u8 *adu, u16 *adu_len)
  {
	int		status;

	queue->operations[0].sem_op=-1;
//	printf("semaphore dec: port=%d\n", queue->port_id+1);
	status=semop(semaphore_id, queue->operations, 1);
				 
		if(status==-1) return status;

	  if(queue->queue_len==0) { /// âíóòðåííÿÿ îøèáêà â ïðîãðàììå
	 		// QUEUE EMPTY
	 		sysmsg_ex(EVENT_CAT_MONITOR|EVENT_TYPE_ERR|queue->port_id, 148, 0, 0, 0, 0);
			return 1;
			}

  	pthread_mutex_lock(&queue->queue_mutex);

		int j, queue_current=queue->queue_start;

		for(j=0; j<queue->queue_adu_len[queue_current]; j++) {
			adu[j]=queue->queue_adu[queue_current][j];
//			printf("(%2.2X)", adu[j]);
			}
		*adu_len=queue->queue_adu_len[queue_current];
//		printf("\n");
		*client_id=queue->queue_clients[queue_current];
		*device_id=queue->queue_slaves[queue_current]; ///!!! ATM & PROXY uses this field
	

		///$$$printf(" go[%d]", queue_current);
		queue->queue_start=(queue->queue_start+1)%MAX_GATEWAY_QUEUE_LENGTH;
		queue->queue_len--;

  	pthread_mutex_unlock(&queue->queue_mutex);
	
//		printf("query_accepted P%d, len=%d\n", queue->port_id+1, queue->queue_len);
	sysmsg_ex(EVENT_CAT_TRAFFIC|EVENT_TYPE_INF|queue->port_id, 221, *client_id, queue->queue_len, 0, 0);

  return 0;
  }
