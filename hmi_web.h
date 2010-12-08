/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010                 
***************************************/

#ifndef HMI_WEB_H
#define HMI_WEB_H

///** МОДУЛЬ МОНИТОРИНГА И УПРАВЛЕНИЯ РАБОТОЙ ШЛЮЗА СРЕДСТВАМИ WEB-ИНТЕРФЕЙСА **

///=== HMI_WEB_H public constants

#define GATE_WEB_INTERFACE_TIMEOUT 2

// коды возврата ошибок
#define HMI_WEB_ENOENT  1
#define HMI_WEB_EACCES  2
#define HMI_WEB_EINVAL  3
#define HMI_WEB_ENOMEM  4
#define HMI_WEB_EEXIST  5
#define HMI_WEB_UNKNOWN 6
#define HMI_WEB_OK      7
#define HMI_WEB_CLOSED  8

///=== HMI_WEB_H public variables

  int shm_segment_id;

	time_t timestamp;  // время последнего обновления данных для web-интерфейса

///=== HMI_WEB_H public functions

///--- shared memory operations
int init_hmi_web_h();
int close_shm();

#endif  /* HMI_WEB_H */
