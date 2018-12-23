/***********   K M - 4 0 0   *************
      ���������������� ������                                
                              ������ 1.0
      ��� "����"
               ������ 2013                 
*****************************************/

#ifndef KLTM_H
#define KLTM_H

///**** ������ ��� ������ � ������������ ������������ (M340) �� �������� ���������
///**** ("�������� �������� �������", ���������� ������ MODBUS RTU/MODBUS TCP)
///**** � ����� ������� � �������� ������������ (���) - � ������

/* �������������� ������������� ����� ������ � ��������� ������ ���������,
   ��������� ����������� ��������� ���� IFACE_RTUMASTER ��� IFACE_TCPMASTER
   ��� ������������ ������ � ������ �������� ���������� PLC � CM � ���� */

#define KLTM_DATA_FIELD_SIZE 114
#define KLTM_SCOM_STRUCT_SIZE 2+KLTM_DATA_FIELD_SIZE/2

typedef struct { // ��������� ������ ������, ������������� ��� ������ ����� ���� � ��-400
	unsigned char R_PAC_NUM;  // ����� ������ ���������
	unsigned char T_PAC_NUM;  // ����� ������ �������������
	unsigned char T_PAC_LEN;  // ���������� ����� ����� ������ � ���� T_DATA
	unsigned char T_PAC_CODE; // ��� ������ (��������)
	unsigned char T_DATA[KLTM_DATA_FIELD_SIZE]; // ������
  } kltm_sCom;

///=== KLTM_H public variables

unsigned char kltm_port;   // ������������� ���������� ����� ��� ������ � ����
int           kltm_client; // ������������� ����������� ����������

// �� ��������� ������������ ��������� � ���� kltm.c
int       kltm_CM;    // ������ � ������� ����������� ���������, ��������������� ���������� CM
int       kltm_PLC;   // ������ � ������� ������,                ��������������� ���������� PLC

///=== KLTM_H public functions

void *kltm_main(void *arg); /// ��������� ������� �������� ������ ��-400

int init_kltm_h();  // ������� ��� �������� ������������ ������������ Modbus-����������

int kltm_getPLC(kltm_sCom *plc); // �������� �� ���� ������ (���������� �������� ���������� PLC)
int kltm_setCM (kltm_sCom *cm ); // �������� �  ���� �����  (����� �������� ���������� CM)

/// ������� ��� ������ � ������� (�������� ������)

int init_modem();
int set_rts(int val);
void show_modem_lines();
void print_buffer(unsigned char *buf, unsigned char len);
int get_modem_data(char *at);
int close_modem();

#endif /* KLTM_H */
