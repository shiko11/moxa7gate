/*
MODBUS GATEWAY SOFTWARE
MOXA7GATE VERSION 1.2
SEM-ENGINEERING
					BRYANSK 2010
*/

#ifndef HMI_WEB_H
#define HMI_WEB_H

///--- shared memory operations
int init_shm();
int refresh_shm(void *arg);
int close_shm();
int check_gate_settings(input_cfg *data);

int shm_segment_id;

#endif  /* HMI_WEB_H */
