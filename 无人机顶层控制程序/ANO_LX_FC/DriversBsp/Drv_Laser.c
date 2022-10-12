#include "Drv_Laser.h"
#include "Drv_Sys.h"

void Laser_OnOff(u8 sta);

void Laser_Init(void)
{
	ROM_SysCtlPeripheralEnable(LASER_SYSCTL);
	ROM_GPIOPinTypeGPIOOutput(LASER_PORT, LASER_PIN);
	//�ر�����LED
	Laser_OnOff(0);
}

//���Ƽ��⿪��
void Laser_OnOff(u8 sta)
{
	if(sta)
	{
		ROM_GPIOPinWrite(LASER_PORT, LASER_PIN, LASER_PIN);
	}
	else 
	{
		ROM_GPIOPinWrite(LASER_PORT, LASER_PIN, 0);
	}
}
