/******************** (C) COPYRIGHT 2017 ANO Tech ********************************
 * ����    �������ƴ�
 * ����    ��www.anotc.com
 * �Ա�    ��anotc.taobao.com
 * ����QȺ ��190169595
 * ����    ����ѭ��
**********************************************************************************/
#include "SysConfig.h"
#include "Ano_Scheduler.h"
#include "User_Task.h"
#include "Drv_Motor.h"

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
	while (1)
	{
		//��ϵͳ����󣬻���������ѭ��
	}
}
#endif
//=======================================================================================
//=======================================================================================
int main(void)
{
	//���������豸�ĳ�ʼ����������ʼ���������
	All_Init();
	//��������ʼ����ϵͳΪ�㱼�������˹�����һ��ʱ�ֵ�����
	Scheduler_Setup();
	//��������һ���͵���Ƿ���ҵ
//	Motor_DT_Transmit(0, Task);
//	Motor_DT_Transmit(0, Task);
//	Motor_DT_Transmit(0, Task);
	while (1)
	{
		//�������������������ϵͳ���ܣ������жϷ�������������������������
		Scheduler_Run();
	}
}
/******************* (C) COPYRIGHT 2014 ANO TECH *****END OF FILE************/
