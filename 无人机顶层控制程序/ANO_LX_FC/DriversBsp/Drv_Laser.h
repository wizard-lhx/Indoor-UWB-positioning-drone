#ifndef _DRV_LASER_H_
#define _DRV_LASER_H_
#include "sysconfig.h"

#define LASER_SYSCTL		SYSCTL_PERIPH_GPIOB
#define LASER_PORT			GPIOB_BASE
#define LASER_PIN			GPIO_PIN_0

void Laser_Init(void);
void Laser_OnOff(u8 sta);

#endif
