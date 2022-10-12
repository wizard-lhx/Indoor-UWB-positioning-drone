#include "Drv_KeyBoard.h"
#include "User_Task.h"
#include "LX_FC_EXT_Sensor.h"

u8 KeyBoard_DT_RX[4];//接收数据帧列表
void KeyBoard_DT_Receive(u8 data) //数据帧格式：0x88，第一个输入点，第二个输入点，0xAA
{                            
	static u8 rxstate;
	//判断帧头1
	if (rxstate == 0 && data == 0x88)
	{
		rxstate = 1;
		KeyBoard_DT_RX[0] = data;
	}
	//第一个位置
	else if (rxstate == 1)
	{
		rxstate = 2;
		KeyBoard_DT_RX[1] = data;
	}
	//第二个位置
	else if (rxstate == 2)
	{
		rxstate = 3;
		KeyBoard_DT_RX[2] = data;
	}	
	//判断帧尾
	else if(rxstate == 3)		//检测是否接受到结束标志
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
	//任务一才用输入
	if(Task == 1)
	{
		First_Position = KeyBoard_DT_RX[1];
		Second_Position = KeyBoard_DT_RX[2];
	}
	
	//上位机查看数据
	key_pos.st_data.keyboard_firstpos = First_Position;
	key_pos.st_data.keyboard_secondpos = Second_Position;
}

