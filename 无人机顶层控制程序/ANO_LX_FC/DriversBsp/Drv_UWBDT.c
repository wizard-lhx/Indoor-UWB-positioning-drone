#include "Drv_UWBDT.h"
#include "Drv_PID.h"
#include "LX_FC_EXT_Sensor.h"

u16 UWB_X,UWB_Y;
u8 UWB_X1,UWB_X2;//�ߵ�
u8 UWB_Y1,UWB_Y2;//�ߵ�

u8 UWB_DT_RX[7];//��������֡�б�
void UWB_DT_Receive(u8 data) //����֡��ʽ��0xda��0xda��x������߰�λ���Ͱ�λ��y������߰�λ��
{                            //�Ͱ�λ��0xbc
	static u8 rxstate;
	//�ж�֡ͷ1
	if (rxstate == 0 && data == 0xDA)
	{
		rxstate = 1;
		UWB_DT_RX[0] = data;
	}
	//�ж�֡ͷ2	
	else if (rxstate == 1 && data == 0xDA)
	{
		rxstate = 2;
		UWB_DT_RX[1] = data;
	}
	//����x��8λ
	else if (rxstate == 2)
	{
		rxstate = 3;
		UWB_DT_RX[2] = data;
	}
	//����x��8λ
	else if (rxstate == 3)
	{
		rxstate = 4;
		UWB_DT_RX[3] = data;
	}
	//����y��8λ
	else if (rxstate == 4)
	{
		rxstate = 5;
		UWB_DT_RX[4] = data;
	}
	//����y��8λ
	else if (rxstate == 5)
	{
		rxstate = 6;
		UWB_DT_RX[5] = data;
	}	
	
	//�ж�֡β
	else if(rxstate==6)		//����Ƿ���ܵ�������־
	{
		if(data == 0xBC)
		{
				rxstate = 0;
				UWB_DT_RX[6]=data;
				UWB_DT();
		}
		else if(data != 0xBC)
		{
				rxstate = 0;							
				for(int D=0;D<7;D++)
				{
						UWB_DT_RX[D]=0x00;
				}           
		}
	}

	else
	{
		rxstate = 0;
	}
}

void UWB_DT(void)
{
	UWB_X1 = UWB_DT_RX[2];
	UWB_X2 = UWB_DT_RX[3];
	UWB_X = UWB_X1*256+UWB_X2;
	UWB_Y1 = UWB_DT_RX[4];
	UWB_Y2 = UWB_DT_RX[5];
	UWB_Y = UWB_Y1*256+UWB_Y2;
	Position_X = UWB_X;
	Position_Y = UWB_Y;
	//��λ���鿴����
	fc_pos.st_data.uwb_x = UWB_X;
	fc_pos.st_data.uwb_y = UWB_Y;
}

