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

///=== HMI_WEB_H public variables

  int shm_segment_id;

	time_t timestamp;  // время последнего обновления данных для web-интерфейса

///=== HMI_WEB_H public functions

///--- shared memory operations
int init_hmi_web_h();
int close_shm();

#endif  /* HMI_WEB_H */
