#include "Drv_KeyBoard.h"
#include "User_Task.h"
#include "LX_FC_EXT_Sensor.h"

u8 KeyBoard_DT_RX[4];//��������֡�б�
void KeyBoard_DT_Receive(u8 data) //����֡��ʽ��0x88����һ������㣬�ڶ�������㣬0xAA
{                            
	static u8 rxstate;
	//�ж�֡ͷ1
	if (rxstate == 0 && data == 0x88)
	{
		rxstate = 1;
		KeyBoard_DT_RX[0] = data;
	}
	//��һ��λ��
	else if (rxstate == 1)
	{
		rxstate = 2;
		KeyBoard_DT_RX[1] = data;
	}
	//�ڶ���λ��
	else if (rxstate == 2)
	{
		rxstate = 3;
		KeyBoard_DT_RX[2] = data;
	}	
	//�ж�֡β
	else if(rxstate == 3)		//����Ƿ���ܵ�������־
	{
		if(data == 0xAA)
		{
			rxstate = 0;
			KeyBoard_DT_RX[3] = data;
		}
		else if(data != 0xAA)
		{
			rxstate = 0;							
			for(u8 D = 0;D < 4;D++)
			{
				KeyBoard_DT_RX[D] = 0x00;
			}           
		}
	}

	else
	{
		rxstate = 0;
	}
}

void KeyBoard_DT(void)
{
	//����һ��������
	if(Task == 1)
	{
		First_Position = KeyBoard_DT_RX[1];
		Second_Position = KeyBoard_DT_RX[2];
	}
	
	//��λ���鿴����
	key_pos.st_data.keyboard_firstpos = First_Position;
	key_pos.st_data.keyboard_secondpos = Second_Position;
}

