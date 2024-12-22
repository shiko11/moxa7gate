/***********   MOXA7GATE   *************
        MODBUS GATEWAY SOFTWARE         
                    VERSION 1.2         
        SEM-ENGINEERING                 
               BRYANSK 2010                 
***************************************/

#ifndef HMI_WEB_H
#define HMI_WEB_H

///** ������ ����������� � ���������� ������� ����� ���������� WEB-���������� **

///=== HMI_WEB_H public constants

// ���������������� ��������� ������� ����������

#define GATE_WEB_INTERFACE_TIMEOUT 2

// ���� �������� ������
#define HMI_WEB_ENOENT  69
#define HMI_WEB_EACCES  66
#define HMI_WEB_EINVAL  68
#define HMI_WEB_ENOMEM  70
#define HMI_WEB_EEXIST  67
#define HMI_WEB_UNKNOWN 72
#define HMI_WEB_OK      71
#define HMI_WEB_CLOSED  65

///=== HMI_WEB_H public variables

extern int shm_segment_id;
extern int shm_segment_ok;

///=== HMI_WEB_H public functions

int refresh_shm(); // ���������� ������������ ������, ����� ��� ���������� � ������ ��������
int update_shm();  // ���������� ����������� ������, ����� ��� ��������� ������������

///--- shared memory operations
int init_hmi_web_h();
int close_shm();

#endif  /* HMI_WEB_H */
