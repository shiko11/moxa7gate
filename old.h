#define	 NOP_CNT							50		//����� ���������� ������ ������ �-�� ��� ������ ����� � ���


#define   MAX_MOXA_REG             10000 //������������ ����� ����������� 4� ��������� ���� (���� ���, � ������ �� ����������)
#define   MAX_MAP_POINT            1024 //������������ ����� �������� ����� (� �������� �� ������ ���� ������ ��� ���������, �� ����������)

//���� ���������
#define   INPUT_REGISTERS          "3x"
#define   HOLDING_REGISTERS        "4x"

#define	MB_TCP_MAX_SEND_ADU_LEN			1024		/* ������������ ����� ������-������� ����������� slave ���������� � ������  */

//��������� ������� ��� ��������� --debug-idata
u16                 _moxa_status_port;
u16                 _moxa_status_start;
u16                 _moxa_status_count;
//��������� ������� ��� ��������� --debug-odata
u16                 _moxa_4x_start;
u16                 _moxa_4x_count;

