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
#define HMI_WEB_ENOENT  69
#define HMI_WEB_EACCES  66
#define HMI_WEB_EINVAL  68
#define HMI_WEB_ENOMEM  70
#define HMI_WEB_EEXIST  67
#define HMI_WEB_UNKNOWN 72
#define HMI_WEB_OK      71
#define HMI_WEB_CLOSED  65

///=== HMI_WEB_H public variables

  int shm_segment_id;

	time_t timestamp;  // время последнего обновления данных для web-интерфейса

///=== HMI_WEB_H public functions

///--- shared memory operations
int init_hmi_web_h();
int close_shm();

#endif  /* HMI_WEB_H */
