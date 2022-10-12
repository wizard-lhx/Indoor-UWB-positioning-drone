#include "Drv_Motor.h"
#include "Drv_Uart.h"
#include "LX_FC_EXT_Sensor.h"

u8 Motor_DT_TX[4];//发送数据列表
void Motor_DT_Transmit(u8 mission, u8 task) //数据帧格式：0x99,马达任务,任务一二, 0xBB
{
	Motor_DT_TX[0] = 0x99;
	Motor_DT_TX[1] = mission;
	Motor_DT_TX[2] = task;
	Motor_DT_TX[3] = 0x55;
	DrvUart1SendBuf(Motor_DT_TX, 4);
}
